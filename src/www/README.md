# src/www — the FluffOS web terminals

Two browser terminals share one stack here. Both render with
[xterm.js](https://xtermjs.org) and speak telnet through the shared
`telnet.js`; they differ only in transport:

| Page | Transport | Serves whom |
|---|---|---|
| `index.html` | WebSocket → the **native** driver's `websocket` ports | players connecting to a live mud (the driver itself serves this page over the ws port's HTTP side — `websocket http dir` in the config, e.g. `testsuite/etc/config.test` points it at this directory) |
| `wasm/index.html` | the synchronous **wasm byte bridge** (`fluffos_input` / `Module.fluffos.onOutput`) | a mudlib running entirely in the browser tab (`docs/build-wasm.md`; packaged by `tools/wasm/pack-mudlib.sh`) |

```
xterm.js (vendor/)             terminal emulation: SGR 16/256/truecolor,
  │                            alternate screen, cursor, wide chars,
  │  onData / write            scrollback, mouse reporting, bracketed
  │                            paste, keyboard encoding
telnet.js (TelnetClient)       option negotiation: ECHO, SGA, NAWS, TTYPE
  │                            (+ per-page hooks: GMCP/MSP on index.html)
  │  sendBytes / receive
transport                      index.html: WebSocket (binaryType
                               arraybuffer); wasm/index.html: queued
                               calls over the synchronous bridge
```

## What each layer owns

**xterm.js** owns everything terminal. Do not hand-roll escape parsing,
key encoding, or screen state here — an earlier bespoke VT emulator in
`wasm/index.html` was replaced wholesale, and the old `parseANSI()` in
`index.html` (which stripped all cursor addressing, making full-screen
LPC TUIs impossible) is gone. The mudlib TUI library
(`testsuite/std/tui`) emits the VT100/xterm subset and both pages run it
end to end: `tuidemo` / `tuidemo select` / `tuidemo dashboard`.

**telnet.js** owns option negotiation, and it is how the driver drives
the page's input modes (`src/net/telnet.cc`):

* `WILL ECHO` / `WONT ECHO` — server takes over / returns echo. In line
  mode the pages flip the input bar to a password field.
* `WILL SGA` / `WONT SGA` — the driver's **char-mode signal**
  (`set_charmode()`, sent when the mudlib arms `get_char()`). The pages
  auto-switch: input bar hides, `term.onData` streams xterm's encoded
  keystrokes (arrows, F-keys, modifiers, mouse reports, bracketed
  pastes) straight to the driver; `WONT SGA` restores the line bar.
* `DO NAWS` — window size wanted (`request_term_size()`). The pages
  answer with the *real* geometry from the fit addon and re-report on
  every resize (`term.onResize`), which re-fires the mudlib's
  `window_size` apply — live TUI relayout.
* `DO TTYPE` → subnegotiation answer `xterm-256color`.
* Anything else is refused unless the page opts in via hooks:
  `onRemoteOption(opt)` (accept a server `WILL`, e.g. GMCP/MSP on
  `index.html`), `onLocalOption(opt)` (accept a server `DO`),
  `onSubneg(opt, bytes)` (payloads). The parser state machine survives
  telnet sequences and UTF-8 characters split across transport chunks
  (streaming `TextDecoder`).

**The pages** own UI (status bar, line-input bar with history, the
connection panel on `index.html`, the error modal + jsbridge handlers on
`wasm/index.html`) and the transport glue.

## Line mode vs char mode

Line mode keeps the classic MUD UX: a local-echo input bar (masked when
the server echoes), Enter sends `line + \r\n`. The xterm viewport above
it is display-only. Char mode (TUI active) hands the whole terminal to
the mud. The switch is fully automatic via SGA — no user toggle.

## The wasm bridge is synchronous both ways (wasm/index.html)

`fluffos_input()` can emit output *before it returns*, and
`Module.fluffos.onOutput` fires while the page's telnet parser is
mid-stream. Sending from inside `receive()` therefore re-enters the
parser and corrupts negotiation state (stack overflow at boot, in the
worst case). The page decouples both directions: outbound bytes queue
and flush from a 0-timeout, inbound chunks drain through a non-reentrant
loop. Keep it that way.

## vendor/ (xterm.js)

`vendor/` holds `@xterm/xterm` and `@xterm/addon-fit` dist files,
byte-exact from the official npm tarballs (versions in the license
headers / `xterm.js` banner; currently xterm 6.0.0, addon-fit 0.11.0),
plus their licenses. To upgrade:

```
npm pack @xterm/xterm @xterm/addon-fit
tar xzf xterm-xterm-*.tgz;    cp package/lib/xterm.js package/css/xterm.css vendor/
tar xzf xterm-addon-fit-*.tgz; cp package/lib/addon-fit.js vendor/
sha256sum the copies against the tarball originals; update the LICENSE copies
```

Both pages load them with plain `<script>` tags (UMD globals `Terminal`
and `FitAddon.FitAddon`) — no bundler, no CDN; the pages must work
self-contained from any static host and offline.

## Packaging

* `tools/wasm/pack-mudlib.sh` copies `wasm/index.html`, `telnet.js` and
  `vendor/` into the dist bundle (and the release zip ships the same
  set) — if you add a file a page loads, add it to the packer AND the
  `build-wasm` packaging step in `.github/workflows/release.yml`.
* `index.html` + `telnet.js` + `vendor/` are served as-is by the native
  driver's ws http mount; no build step.

## Testing

CI runs `tools/ws-smoke.js` (see the `Websocket smoke test` step in
`.github/workflows/ci.yml`): a dependency-free node websocket client
that boots `driver etc/config.test`, exercises the `telnet` and `ascii`
subprotocols, verifies option negotiation (SGA char-mode switch), and
pushes multi-window output bursts and live TUI frames through the
connection (no test exercised a real ws client before it).

Critically, it also forces **genuine kernel-level backpressure** (pauses
the test client's socket, then pushes a burst big enough to fill the
kernel send buffer, then resumes) and confirms the connection recovers.
This is the regression test for an output wedge in
`src/net/ws_telnet.cc` / `ws_ascii.cc`: `lws_send_pipe_choked()` is
true not only when lws holds a truncated send (in which case lws
re-arms the writeable callback itself) but also when a zero-timeout
`poll(POLLOUT)` says the socket is simply full — and in that second
case every write so far succeeded, lws has nothing pending, and nobody
re-arms anything unless the drain loop requests the next writeable
callback itself whenever it exits with data still queued. A *quick*,
unpaused multi-window
burst does **not** exercise this — on a fast reader the whole payload
fits in the kernel's send buffer and no write ever blocks, so it passes
identically whether the fix is present or not. Only forced backpressure
tells them apart; see the comment above the drain loop in
`ws_telnet.cc`. The smoke test forces backpressure on the plain and TLS
ports (the TLS partial-write path retries differently) and also
destructs a player mid-choke — teardown must release the session
resources even when the driver side is already gone (an early return
there once leaked the evbuffer, and leaked a since-removed session
timer as a use-after-free the ASan CI job caught).

For the full browser experience (rendering, keyboard, resize) drive the
pages with a real browser: boot `driver etc/config.test`, open
`http://localhost:4001/`, connect to `127.0.0.1:4001` with the `telnet`
subprotocol, and run the `tuidemo` showcases. The wasm page is tested
the same way against `tools/wasm/build.sh`'s dist output.
