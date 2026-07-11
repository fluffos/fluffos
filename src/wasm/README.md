# FluffOS on WebAssembly (Emscripten)

This directory contains the WebAssembly port of the driver: the whole
compiler + VM + efun stack runs inside a browser page (or node), boots a
real mudlib from an in-memory filesystem, and **speaks real telnet with
the webpage** — the page is the telnet client, the JS host is the wire.

Status: **working**. The driver boots the bundled `testsuite/` mudlib,
accepts virtual connections through `master::connect()` / `logon()`,
processes commands (including `input_to`/`get_char`, prompts, snoop), runs
heartbeats and call_outs, and the LPC testsuite passes clean (tests of
packages that don't exist on this target skip themselves via
`__PACKAGE_*__` guards). CI runs the suite under node on every PR (the
`wasm` job in `.github/workflows/ci.yml`).

This file documents the architecture. **The user-facing end-to-end
workflow (deps → presets → packer → serve → embed) is
`docs/build-wasm.md`.** Quick start:

```
tools/wasm/build-deps.sh     # once: cross-build ICU + zlib
tools/wasm/build.sh          # presets native-tools + wasm, pack testsuite
python3 -m http.server -d build-wasm/dist 8080
# open http://localhost:8080/
# package your own mudlib: tools/wasm/pack-mudlib.sh --help
```

---

## 1. Architecture

Three native concepts have no browser equivalent; each got a seam:

### 1.1 The event loop is inverted (`wasm/backend_wasm.cc`)

The native driver blocks forever in libevent's `event_base_loop()`
(`backend.cc`). A browser tab cannot block: the page owns the event loop.

The scheduling core (the gametick queue, `add_gametick_event()`, the
maintenance events) lives in the shared `backend.cc`; only the loop that
advances time is per-target. `wasm/backend_wasm.cc` implements it with a
plain wall-time priority queue and **no libevent at all**: the page calls
the exported `fluffos_tick(now_ms)` on a timer
(`setInterval`/`requestAnimationFrame`), which drains due wall-time
events (including buffered user commands, scheduled by the transports)
and advances as many gameticks as have elapsed (capped catch-up for
suspended tabs). Heartbeats, call_outs, resets, reclaims — all driver
scheduling already went through this API, so nothing above it changed.

### 1.2 The socket is a `Transport` (`net/transport.h`)

Each `interactive_t` owns a `Transport` — the abstract byte pipe
(`write` / `flush` / `schedule_command` / `close`). Everything above it
is transport-agnostic and **compiled unchanged on every target**:
`comm.cc` (users, command queue, input_to, prompts, snoop) and
`net/telnet.cc` (full telnet negotiation via libtelnet, which is pure C).
There are three implementations, selected per target at link time:

- `SocketTransport` and `WebsocketTransport`
  (`net/transport_libevent.cc`, native only): bufferevent/TLS and
  libwebsockets, plus the listening ports, accept handler and socket
  read path;
- `WasmConsoleTransport` (`wasm/comm_wasm.cc`): outbound wire bytes go
  to `Module.fluffos.onOutput(id, bytes)` in JS; inbound bytes come from
  the exported `fluffos_input(id, bytes)` → `comm_telnet_received()` —
  the same path a socket read takes. Connection setup
  (`fluffos_connect()`) mirrors the native accept flow: `user_add()`,
  telnet init + initial negotiations, `master::connect(port)`,
  `logon()`. There are no listening sockets (`init_user_conn` is a
  no-op).

Because the driver still emits/consumes real telnet, the page needs a
(tiny) telnet client — `src/www/wasm/index.html` includes one (~60
lines): refuse every option except ECHO (used to mask password input),
skip subnegotiations, decode the rest as UTF-8. Any full JS telnet client
(or an xterm.js frontend) plugs in the same two callbacks.

The same link-time pattern covers the other per-target singletons:

| Interface | Native | WASM |
|---|---|---|
| event loop (`backend.h`) | `backend_libevent.cc` | `wasm/backend_wasm.cc` |
| connection transports | `net/transport_libevent.cc` | `wasm/comm_wasm.cc` |
| TLS (`net/tls.h`) | `net/tls.cc` | `net/tls_stub.cc` (fails cleanly) |
| DNS resolver (`packages/core/dns.h`) | `packages/core/dns_libevent.cc` | `packages/core/dns_stub.cc` |
| crash handler (`base/internal/crash_handler.h`) | `base/internal/crash_handler.cc` (backward-cpp) | `wasm/crash_handler_wasm.cc` |

The shared logic files contain no `#ifdef __EMSCRIPTEN__`; the only
platform conditionals live in `net/net_compat.h` (type declarations) and
one thread-capability guard in `base/internal/tracing.cc`.

### 1.3 The filesystem is a VFS (Emscripten MEMFS)

The driver's file I/O (compilation, `save_object`, `read_file`, ed, logs)
is plain POSIX + ghc::filesystem, which Emscripten maps onto its in-memory
FS transparently — **zero driver changes were needed**.

The mudlib is bundled with Emscripten's `file_packager`
(`tools/wasm/build.sh` stage 3): it packs `testsuite/` into a single
`mudlib.data` image + `mudlib.js` loader that mounts it at `/testsuite`
before the runtime starts. Any mudlib can be packed the same way
(`MUDLIB=/path/to/lib tools/wasm/build.sh`). Writes go to MEMFS and last
for the page session; a persistent overlay (IDBFS/OPFS syncing `/data`,
save files etc.) is the natural next step — see §5.

## 2. What was removed, what was kept

| Subsystem | WASM build | Why |
|---|---|---|
| libevent | **removed** | replaced by host-driven tick queues |
| libwebsockets, `net/websocket.cc`, `net/ws_*.cc` | **removed** | the page *is* the client; no listening sockets |
| OpenSSL, `net/tls.cc` | **removed** | no TLS endpoint to terminate; 2 struct fields typedef'd via `net/net_compat.h` |
| libtelnet, `net/telnet.cc`, `net/msp.cc`, mssp | **kept** | pure C / portable; the page speaks telnet |
| ICU (uc + data) | **kept** (cross-built) | core string handling: grapheme iteration, charset conversion, sprintf width |
| zlib | **kept** (cross-built) | libtelnet MCCP + compress package |
| `thirdparty/crypt` (musl crypt) | **kept** | pure C |
| backward-cpp | **removed** | no native unwinder in wasm |
| jemalloc | **removed** | dlmalloc from emscripten |
| POSIX eval-limit timers (`posix_timers.cc`) | auto-disabled | `__linux__` only; see §5 for the planned replacement |

Package matrix (`src/CMakeLists.txt` forces these under `EMSCRIPTEN`):

| Package | State | Reason |
|---|---|---|
| core, ops, math, matrix, trim, uids, sha1, parser, contrib, develop, mudlib_stats, compress | **on** | portable |
| dwlib | default off (same as native) | portable; enable with `-DPACKAGE_DWLIB=ON` |
| **jsbridge** | **on (WASM only)** | `js_eval()` / `js_call()` / `js_export()`: LPC ↔ page JavaScript in both directions (fetch, canvas/WebGL, page UIs driving the game, …) — see `docs/build-wasm.md` §7 and `docs/driver/wasm.md` |
| sockets | off | BSD sockets (LPC socket efuns) |
| external | off | `posix_spawn` child processes |
| async | off | worker threads (see §5) |
| db | off | MySQL/SQLite/PG client libs |
| crypto | off | OpenSSL EVP (see §5: sha1 stays) |
| ffi | off | libffi + dlopen |
| pcre | off | libpcre not cross-built yet (regexp package efuns; core regexp stays) |

DNS (`packages/core/dns.cc`): the resolver half is stubbed to fail the
same way as a native host without a nameserver (`resolve()` raises an LPC
error); the address-cache half (`query_ip_name`/`query_ip_number`) is
compiled unchanged.

## 3. Build system

Two CMake presets, because the codegen tools (`make_func`,
`build_applies`, `make_options_defs`) execute at build time and must be
native:

```
# stage 1 -- native host tools (preset: native-tools)
cmake --preset native-tools && cmake --build --preset native-tools

# stage 2 -- cross build (preset: wasm, through the emscripten wrapper)
emcmake cmake --preset wasm && cmake --build --preset wasm
```

`tools/wasm/build.sh` runs both presets and packages the bundled
testsuite; `tools/wasm/pack-mudlib.sh` packages any mudlib (see
`docs/build-wasm.md`).

### 3.1 Cross-built dependencies (ICU, zlib)

The build expects static wasm libraries under one prefix
(`-DFLUFFOS_WASM_DEPS`, default `/opt/wasm-deps`): `libicuuc.a`,
`libicudata.a`, `libz.a` + headers. **`tools/wasm/build-deps.sh` builds
all of it**; for the record, the two ICU cross-compile quirks it handles:

- copy `config/mh-linux` to `config/mh-unknown` (ICU doesn't know the
  emscripten triple);
- `pkgdata` cannot produce a wasm object for the ~30MB data archive —
  generate it as C instead with the *host* build's `genccode`
  (`genccode -e icudt74 icudt74l.dat && emcc -c ... && emar rcs
  libicudata.a ...`).

### 3.2 Link flags (see `driver-web` in `src/CMakeLists.txt`)

- exceptions (compile+link): LPC error handling is C++ exceptions;
  emscripten disables catching by default. On emsdk >= 3.1.57 the build
  uses `-fwasm-exceptions` (native wasm EH — much faster unwinding, and
  supported by every current browser and node >= 18); older toolchains
  fall back to the JS-based `-sDISABLE_EXCEPTION_CATCHING=0`.
- `-sMODULARIZE=1 -sEXPORT_NAME=createFluffOS --no-entry`: the page
  instantiates the module and drives exported entry points; there is no
  `main()`.
- `-sINITIAL_MEMORY=128MB -sALLOW_MEMORY_GROWTH=1 -sTOTAL_STACK=16MB`:
  ICU's static data (~30MB) lives in the data segment.
- `-g0 --profiling-funcs`: full DWARF is dropped at link (binaryen's
  wasm-opt asserts trying to update it at -O3, notably with wasm EH) but
  the function-name section is kept, so browser/node stack traces stay
  readable.
- no `-pthread`, no stack protector, no `-march=native`, no LTO.

## 4. JS embedding API

```js
const M = await createFluffOS({ print, printErr, locateFile });
M.FS.chdir('/testsuite');                       // mudlib mount point
M.fluffos = {
  onOutput:    (id, bytes) => {...},            // server->client wire bytes
  onDisconnect:(id)        => {...},
};
M.ccall('fluffos_boot', 'number', ['string'], ['etc/config.test']);
setInterval(() => M.ccall('fluffos_tick', 'number', ['number'],
                          [performance.now()]), 50);
const id = M.ccall('fluffos_connect', 'number', [], []);
M.ccall('fluffos_input', null, ['number','array','number'], [id, bytes, n]);
// also exported: fluffos_flag (master::flag, e.g. 'test' runs the LPC
// testsuite), fluffos_disconnect, fluffos_shutdown
```

## 5. Refactoring roadmap (remaining phases)

Delivered so far: the `Transport` interface + per-target implementations
(link-time dispatch, no `#ifdef` seams in shared code), the inverted
backend, the JS telnet bridge, presets + `tools/wasm/build-deps.sh` +
`tools/wasm/pack-mudlib.sh`, the web shell, docs
(`docs/build-wasm.md`), and a CI job that gates on the LPC testsuite
running inside the wasm driver. The follow-up phases, in recommended
order:

1. **Eval limit.** `set_eval()` currently warns "platform doesn't support
   eval limit": a runaway LPC loop hangs the tab. Replace the
   SIGVTALRM/posix-timer scheme with a deadline check
   (`emscripten_get_now() > deadline → outoftime = 1`) polled in the
   interpreter's existing backward-branch/apply hooks. This also benefits
   macOS/Windows, which have no eval limit today either.
2. **Persistent storage.** Mount an IDBFS (or OPFS) overlay over the
   mudlib's write paths (`/data`, save files, logs) and
   `FS.syncfs()` on a timer + on `visibilitychange`, so player data
   survives page reloads.
3. **More packages.** PCRE cross-builds with emconfigure (or switch the
   pcre package to PCRE2, which has better wasm support). `crypto` can
   come back once OpenSSL's libcrypto is cross-built (or be rebased onto
   smaller portable digests). `async` can return as synchronous fallbacks
   (the efuns' contracts allow completing "later" on the next tick).
   `db` with SQLite is feasible (sqlite3 compiles to wasm famously well)
   and would give mudlibs a real database in the browser.
4. **LPC sockets over WebSocket/WebRTC.** If a mudlib needs outbound
   `socket_efuns`, they can be tunneled through `WebSocket` objects on
   the JS side with the same bridge pattern as the console (bytes in /
   bytes out per socket id). Inter-mud protocols would then work.
5. **Size/latency budget.** `fluffos.wasm` is ~47MB debug / much smaller
   with `-O2 -g0` + `--strip-debug`; the dominant cost is ICU data.
   Apply an `ICU_DATA_FILTER_FILE` (keep root locale, brkitr, and the
   charsets the mudlib actually uses) to cut it to a few MB, and serve
   both files with gzip/brotli (wasm+data compress ~4:1).
   (`-fwasm-exceptions` is already used on emsdk >= 3.1.57 — see §3.2.)
6. **Native loopback transport.** The `Transport` interface makes an
   in-process console user possible on the native driver too (a
   `PipeTransport`), which would let driver tests exercise the full
   login/command pipeline without sockets.

## 6. Known limitations (current state)

- No eval limit: `while(1);` in LPC blocks the tab (phase 1 above).
- `resolve()` raises "DNS resolver is not available"; `query_ip_number()`
  reports 127.0.0.1 for web connections.
- Disabled-package efuns (`socket_*`, `external_start`, `db_*`, ffi,
  async I/O, PCRE efuns) don't exist; their testsuite files skip
  themselves via `__PACKAGE_*__` guards and the suite passes clean.
- MEMFS writes are per-session until phase 2 lands.
- The tab suspends timers in background: gameticks catch up (capped at
  100 ticks) when the tab wakes rather than running while hidden.
