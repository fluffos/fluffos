# src/www agent guide

Read `README.md` here first — it is the architecture doc (the two pages,
the xterm.js/telnet.js/transport layering, line-vs-char mode, vendor
policy, packaging, testing). This file is the agent-facing checklist of
rules and once-bitten lessons.

## Hard rules

1. **Never hand-roll terminal emulation.** xterm.js owns escape parsing,
   key/mouse/paste encoding, screen state, wide chars. A bespoke VT
   emulator existed here once and was replaced; don't reintroduce one,
   and don't "quickly strip ANSI" for display — the old `parseANSI()`
   deleted cursor addressing and silently broke every full-screen TUI.
2. **One telnet implementation.** Both pages share `telnet.js`. Fix bugs
   there, not with per-page workarounds; page-specific options (GMCP,
   MSP) go through its hooks (`onRemoteOption`/`onLocalOption`/
   `onSubneg`), never as forks of the parser.
3. **Self-contained pages.** Plain `<script>` tags, UMD globals, no
   bundler, no CDN. Everything a page loads lives next to it when
   deployed: keep `tools/wasm/pack-mudlib.sh` and the release-zip step
   in `.github/workflows/release.yml` in sync with any new file.
4. **Vendor byte-exact from the official npm tarball** (`npm pack`,
   sha256-compare, keep licenses). Never reconstruct vendor code by any
   lossy channel (root AGENTS.md §14 — observed to fabricate plausible
   code that passes tests).
5. **wasm bridge reentrancy** (`wasm/index.html`): the bridge is
   synchronous both ways — never call `fluffos_input` from inside
   `TelnetClient.receive()` processing, and never process inbound bytes
   reentrantly. The outbox (0-timeout flush) / inbox (non-reentrant
   drain) queues exist because negotiation replies sent mid-parse
   re-entered the parser and blew the stack at boot.
6. **Browser compat**: no regex lookbehind (`(?<!...)` broke Safari
   <16.4), set `ws.binaryType = 'arraybuffer'` (Blob handlers reorder
   async), and expect telnet sequences AND multi-byte UTF-8 to split
   across ws frames (streaming decode + stateful parsing already handle
   this — keep it true).

## Driver-side facts that bite web-client work

* **Char mode is `WILL SGA`**, sent by `set_charmode()` when the mudlib
  arms `get_char()` (`src/net/telnet.cc`); `WONT SGA` = back to line
  mode. ECHO masks passwords. NAWS answers must use real terminal
  geometry (fit addon), re-sent on resize → drives the `window_size`
  apply → TUIs relayout live.
* **The lws output wedge** (`src/net/ws_telnet.cc` / `ws_ascii.cc`): the
  handlers drain multiple write windows per callback in a loop gated on
  `lws_send_pipe_choked()`. That gate is true in TWO cases, and they
  differ in who re-arms the next writeable callback: (a) lws holds a
  truncated send from a genuine partial write — lws re-arms itself; (b)
  a zero-timeout `poll(POLLOUT)` says the socket is full RIGHT NOW —
  every write so far fully succeeded, lws has nothing pending, and
  **nobody re-arms anything**. The drain loop therefore requests the
  next writeable callback itself whenever it exits with data still
  queued — otherwise output freezes permanently (megabytes stuck in
  `pss->buffer`). `lws_callback_on_writable()` is the upstream-documented
  way to ask (lws README.coding.md: "if you want to send something, do
  not just send it but request a callback when the socket is writeable"),
  and calling it from inside the writeable handler is safe — traced end
  to end through `_lws_change_pollfd()` → the evlib glue's `event_add()`;
  an earlier theory that in-handler requests are "lossy with the libevent
  event lib" did not survive tracing and is dead. The same doc's "do not
  rely on only your own WRITEABLE requests appearing" is why the handler
  drains from `pss->buffer` and treats a nothing-to-do callback as a
  no-op. If ws output ever "stops after a burst", start at that
  data-remaining re-arm. The bug only reproduces under real backpressure (a slow or
  paused reader) — a fast local reader never fills the kernel send
  buffer, so a burst test that doesn't force backpressure passes on
  broken code. It's also invisible to every non-ws test.
* **Session teardown must free the pss resources UNCONDITIONALLY in
  `LWS_CALLBACK_CLOSED`.** On driver-initiated closes (mudlib destructs
  the interactive), `close_user_websocket()` nulls `pss->user` before
  CLOSED fires — an early `return` on `!pss->user` there leaked the
  evbuffer on every such close, and when an interim revision parked a
  libevent timer in the session it became a deterministic ASan
  use-after-free (the pending timer fired on the freed wsi/pss). Any new
  per-session resource must be released on BOTH sides of that
  `pss->user` check.
* **Multiple stale ws connections skew debugging**: each test run that
  doesn't quit cleanly leaves the driver processing net-dead teardown;
  restart `driver etc/config.test` between debugging sessions and watch
  for `]`-prefixed escape spam in its log (writes to dead connections).

## Testing expectations

* Any change to these pages, `telnet.js`, or `src/net/ws_*.cc` must keep
  `node tools/ws-smoke.js` green (CI runs it: boots the real driver,
  telnet + ascii subprotocols, SGA switch, multi-window bursts,
  paused-socket backpressure bursts on plain AND TLS ports, a
  destruct-while-choked teardown check, TUI frames, clean quit). It
  exists because GTest + the LPC suite exercise **zero** websocket
  client traffic. The backpressure checks are what actually regression-
  guard the lws output wedge above — verified to fail (all three) on the
  pre-fix driver; don't remove the socket pause/resume from them to
  "speed it up", they are no-ops without it (a fast reader never blocks
  a write). The teardown check mostly bites on the ASan CI job, where a
  teardown memory bug aborts the driver and the follow-up connection
  check catches it.
* For rendering/keyboard/resize changes, drive a real browser through
  the `tuidemo` showcases (see README.md "Testing") — the smoke test
  can't see what xterm.js draws.
