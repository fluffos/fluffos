---
title: Build (WebAssembly)
---

# Running FluffOS in the browser (WebAssembly)

FluffOS cross-compiles to WebAssembly with [Emscripten](https://emscripten.org):
the whole compiler + VM + efun stack runs inside a webpage, boots a real
mudlib from an in-memory filesystem, and **speaks real telnet with the
page** — the page is the telnet client, JavaScript is the wire. The LPC
testsuite passes in the browser and under node.

This page is the end-to-end workflow: build the toolchain deps once,
build the driver, package **your** mudlib, serve it, embed it. For the
architecture (event-loop inversion, the `Transport` interface, what is
and isn't supported) see
[`src/wasm/README.md`](https://github.com/fluffos/fluffos/blob/master/src/wasm/README.md).

> **Don't want to build?** Every
> [FluffOS release](https://github.com/fluffos/fluffos/releases) ships
> `fluffos-<version>-wasm.zip` — the prebuilt driver, the web terminal,
> and `pack-mudlib.sh`. Unzip it and skip straight to step 3 (the packer
> still needs emscripten's `file_packager`, i.e. emsdk on PATH).

## 0. Prerequisites

* **Emscripten** (`emcc`, `emcmake`, `emmake` on PATH) — install via
  [emsdk](https://emscripten.org/docs/getting_started/downloads.html), or
  your distro package (`apt install emscripten`). **Avoid emsdk 6.0.2**: it
  defaulted `GROWABLE_ARRAYBUFFERS=1`, and in browsers with wasm
  resizable-ArrayBuffer support the driver fails on boot with
  `getRandomValues ... must not be resizable` (and jsbridge string passing
  breaks the same way). Use 6.0.3+ — the version CI pins is in
  `.github/actions/build-wasm/action.yml`.
* A **native toolchain** plus the usual FluffOS build deps (OpenSSL/ICU
  headers): the codegen tools run on the build machine, and ICU's cross
  build needs a native ICU build first.
* `cmake` ≥ 3.22, `curl`, `python3`.

## 1. Build the WASM dependencies (once)

The driver needs a static WASM build of **ICU** (Unicode: grapheme
iteration, charset conversion) — the only cross-built dependency:

```bash
tools/wasm/build-deps.sh          # installs into /opt/wasm-deps
# or: PREFIX=$HOME/wasm-deps tools/wasm/build-deps.sh
```

This is fully scripted, including the ICU cross-compile quirks (the
`mh-unknown` platform file, and generating the data archive as C source
with the host `genccode` because `pkgdata` cannot emit wasm objects).
It only runs once; re-runs are no-ops.

The ICU data archive is **trimmed to break-iterator data** so the
driver stays small (`fluffos.wasm` is ~3.6MB raw, ~0.8MB brotli) —
table charsets (GBK, Big5, …) are not available on this target unless
you rebuild the deps with `ICU_KEEP` (see the header of
`build-deps.sh`). UTF-8/UTF-16/Latin-1/ASCII always work.

## 2. Build the driver

Two CMake presets do the two stages — codegen tools natively, then the
cross build:

```bash
cmake --preset native-tools && cmake --build --preset native-tools
emcmake cmake --preset wasm  && cmake --build --preset wasm
```

Non-default deps prefix: `emcmake cmake --preset wasm -DFLUFFOS_WASM_DEPS=$HOME/wasm-deps`.

Output: `build-wasm/src/fluffos.js` + `fluffos.wasm`.

`tools/wasm/build.sh` runs both presets **and** packages the bundled
testsuite as a demo in one command.

## 3. Package your mudlib

`tools/wasm/pack-mudlib.sh` turns any mudlib + the driver into a
static web bundle:

```bash
tools/wasm/pack-mudlib.sh \
    --mudlib /path/to/mylib \
    --config etc/config \
    --out dist/
```

* `--mudlib` is packed with Emscripten's `file_packager` into
  `mudlib.data` + `mudlib.js`, mounted read-write in the page's memory
  filesystem at `--mount` (default `/<basename>`).
* `--config` is the runtime config **relative to the mudlib root**,
  exactly like running `driver etc/config` natively from that directory.
  Ports/TLS entries in the config are ignored (there are no listening
  sockets; the page connects directly).
* The bundle contains `index.html` (a self-contained web terminal with a
  minimal telnet client), `fluffos.js`/`fluffos.wasm`,
  `mudlib.js`/`mudlib.data`, and `fluffos-boot.js` (mount + config for
  the page).

## 4. Serve it

Any static HTTP server works (wasm cannot load from `file://`):

```bash
python3 -m http.server -d dist 8080
# open http://localhost:8080/
```

For production, serve `fluffos.wasm` and `mudlib.data` with gzip/brotli
(fluffos.wasm is ~0.8MB brotli) and long cache lifetimes.

## 5. Run the LPC testsuite in the WASM driver

```bash
node tools/wasm/run-testsuite.js         # boots testsuite/, runs -ftest
```

Exit code 0 plus `Checks succeeded.` is the pass signal — the same gate
CI uses (see the `wasm` job in `.github/workflows/ci.yml`). Tests for
packages that don't exist on this target (sockets, external, db, ffi,
pcre, crypto, async, compress) skip themselves via `#ifdef __PACKAGE_*__`
guards.

## 6. Embedding API (custom frontends)

`index.html` is intentionally small; any frontend (e.g. xterm.js) can
drive the same module:

```js
const M = await createFluffOS({ print, printErr, locateFile });
M.FS.chdir('/mylib');                          // mudlib mount point
M.fluffos = {
  onOutput:     (id, bytes) => { /* server->client telnet bytes */ },
  onDisconnect: (id)        => { /* connection closed */ },
};
M.ccall('fluffos_boot', 'number', ['string'], ['etc/config']);
setInterval(() => M.ccall('fluffos_tick', 'number', ['number'],
                          [performance.now()]), 50);
const id = M.ccall('fluffos_connect', 'number', [], []);   // telnet "dial"
M.ccall('fluffos_input', null, ['number','array','number'],
        [id, bytes, bytes.length]);                        // client->server
// also exported: fluffos_flag (master::flag, e.g. 'test'),
// fluffos_disconnect, fluffos_shutdown
```

The driver does real telnet negotiation on each connection (ECHO for
password masking, NAWS, GMCP…), so the page needs at least a
minimal telnet layer — copy the ~60-line client from
`src/www/wasm/index.html`.

## 7. Calling JavaScript from LPC (jsbridge)

The WASM driver ships the `jsbridge` package: LPC code can reach the
page's JavaScript — `fetch()`, canvas/WebGL, storage, anything the page
exposes — and get called back asynchronously.

```c
// synchronous eval; result as a string
write(js_eval("navigator.userAgent"));

// async: call a handler the page registered; cb(result, success, id)
void got_body(string body, int success, int id) { write(body); }
js_call("fetch_text", ({ "https://example.com/data.json" }),
        (: got_body :));
```

The page registers handlers on the module (they may return values or
Promises; structured data crosses the bridge as JSON strings):

```js
M.fluffos.handlers = {
  fetch_text:  async (url) => (await fetch(url)).text(),
  canvas_draw: (op, ...args) => { /* draw on a <canvas> */ },
};
```

The reverse direction — the page calling into LPC — is `js_export`:

```c
js_export("add", (: lpc_add :));   // mixed lpc_add(string *args, int id)
```

```js
const sum = await M.fluffos.callLPC("add", "2", "3");  // Promise -> "5"
```

**Demo**: the bundled web terminal wires `fetch_text` and `canvas_draw`
handlers; try `jsdemo eval 1+2`, `jsdemo fetch /index.html`,
`jsdemo canvas` (LPC drawing on the page's canvas), and `jsdemo export`
followed by `await fluffos.callLPC("add", "2", "3")` in the devtools
console — implemented in `testsuite/command/jsdemo.lpc` +
`src/www/wasm/index.html`. See the
[WASM driver cookbook](driver/wasm.md) for more recipes.

## Notes & limits

* No eval limit yet on this target: a `while(1);` in LPC blocks the tab.
* `resolve()` raises "DNS resolver is not available"; connections report
  `127.0.0.1`.
* Mudlib writes live in page memory for the session; persistent storage
  (IDBFS) is on the roadmap in `src/wasm/README.md`.
* No zlib on this target: compressed `save_object` degrades to a plain
  save, gzip'd `write_file` raises an error, and `.gz` files aren't
  transparently decompressed.
* Only algorithmic charsets ship (UTF-8/UTF-16/UTF-32, Latin-1, ASCII);
  `string_encode()` to a table charset raises an error. LPC can adapt
  with `#ifdef __WASM__` (predefined on this target).
* Background tabs throttle timers; the driver catches up (capped) when
  the tab wakes.
