---
title: WASM driver cookbook
---

# The WASM driver: packer + JsBridge cookbook

The WebAssembly build turns FluffOS into something you can hand out as a
URL: the full driver (compiler, VM, efuns, telnet) runs inside the page,
your mudlib rides along in a packed filesystem image, and the `jsbridge`
package lets LPC call into the browser — `fetch()`, canvas/WebGL, audio,
storage — and get called back.

This page is the *what can I build with it* guide. For building the
driver itself (deps, presets, CI) see [Build (WebAssembly)](../build-wasm.md);
for the internals see
[`src/wasm/README.md`](https://github.com/fluffos/fluffos/blob/master/src/wasm/README.md).

## The three pieces

| Piece | What it gives you |
|---|---|
| the wasm driver (`cmake --preset wasm`) | `fluffos.js` + `fluffos.wasm`: a driver instance per browser tab, driven by exported functions |
| the packer (`tools/wasm/pack-mudlib.sh`) | your mudlib + the driver + a web terminal as one static bundle — no server-side code at all |
| the `jsbridge` package | `js_eval()` / `js_call()`: LPC ↔ page JavaScript, with async callbacks |

## Recipe: publish a playable mudlib as a static site

```bash
tools/wasm/pack-mudlib.sh --mudlib /path/to/mylib --config etc/config --out dist/
```

`dist/` is entirely static — serve it from GitHub Pages, Netlify, an S3
bucket, or `python3 -m http.server`. Every visitor gets their **own**
driver instance booting a pristine copy of the mudlib (nothing is shared
between tabs). That makes it ideal for:

* a **playable demo** of your MUD linked from its website — no login
  server exposure, no abuse surface, throw-away worlds;
* **mudlib development tutorials** — readers edit LPC in the packed lib
  (`ed` works in the terminal!), `update` the object, and see the result
  without installing anything;
* **regression sandboxes** — pin a bundle per release and bisect
  behavior in a browser tab.

A minimal GitHub Pages deploy job (after the `wasm` CI job's build
steps):

```yaml
- run: tools/wasm/pack-mudlib.sh --mudlib mylib --config etc/config --out site/
- uses: actions/upload-pages-artifact@v3
  with: { path: site }
- uses: actions/deploy-pages@v4
```

## Recipe: a custom frontend (xterm.js or your own UI)

The bundled `index.html` is deliberately small. Any UI can drive the
driver through five exported functions and two callbacks:

```js
const M = await createFluffOS({ locateFile: (f) => f });
M.FS.chdir('/mylib');
M.fluffos = {
  onOutput: (id, bytes) => term.write(bytes),   // server->client telnet bytes
  onDisconnect: (id) => term.write('*** gone'),
};
M.ccall('fluffos_boot', 'number', ['string'], ['etc/config']);
setInterval(() => M.ccall('fluffos_tick', 'number', ['number'],
                          [performance.now()]), 50);
const id = M.ccall('fluffos_connect', 'number', [], []);
term.onData((s) => {                             // client->server bytes
  const b = Array.from(new TextEncoder().encode(s));
  M.ccall('fluffos_input', null, ['number','array','number'], [id, b, b.length]);
});
```

xterm.js pairs especially well: it already understands ANSI colors, and
its `AttachAddon`-style byte interface maps 1:1 onto
`onOutput`/`fluffos_input`. The driver speaks real telnet (ECHO for
password masking, NAWS, GMCP…), so either strip IAC sequences with the
~60-line client in `src/www/wasm/index.html` or handle the options you
care about (GMCP gives you a structured out-of-band channel to your UI
for free — health bars, maps, inventory panels).

`fluffos_connect()` can be called more than once: several "players" in
one tab (e.g. a split-screen tutorial where the reader plays one
character and a script drives another).

## JsBridge: the contract

```c
// synchronous: evaluate JS, get the result (or "JS error: ...") as a string
string ua = js_eval("navigator.userAgent");

// asynchronous: call a handler the page registered; the callback runs on
// a later driver tick, like call_out(0) or the resolve() efun
int id = js_call("handler_name", ({ "arg1", "arg2" }), (: my_callback :));
void my_callback(string result, int success, int id) { ... }
```

```js
// page side: plain functions or async functions / Promises
M.fluffos.handlers = {
  handler_name: async (arg1, arg2) => { ... return value; },
};
```

And the reverse — the page calling **into** LPC:

```c
// LPC: export a function under a name
private mixed lpc_add(string *args, int id) { return to_int(args[0]) + to_int(args[1]); }
js_export("add", (: lpc_add :));
```

```js
// JS: a Promise of the (string) return value
const sum = await M.fluffos.callLPC("add", "2", "3");   // "5"
```

Rules of the road:

* **Strings across the bridge.** Args are an array of strings; the
  return value is stringified (objects become JSON). For structured
  data, JSON-encode/decode on both sides.
* **Callbacks run without a command context.** Like `call_out`,
  `this_player()` is 0 inside the callback — capture the player in the
  closure: `object me = this_player(); js_call(..., (: on_done, me :))`
  and use `tell_object(me, ...)`. (See `testsuite/command/jsdemo.lpc`.)
* **Failure is a flag, not a crash.** A missing handler, a thrown JS
  exception, or a rejected Promise all deliver `success == 0` with the
  error text as the result. `js_eval` returns `"JS error: ..."` text.
* **The page owns the handlers.** LPC can only call what the embedding
  page chose to expose — a bundle's capability surface is exactly its
  handler table (plus `js_eval`, which you can neuter with a CSP that
  forbids `eval`).
* **The mudlib owns the exports.** Symmetrically, the page can only call
  LPC names registered with `js_export()`; each export runs on the next
  driver tick and holds a reference on its object until unregistered.

## Recipe: a UI that drives the game (js_export)

`js_export` turns page chrome into game input without going through the
terminal. Buttons, inventory panels, or a clickable minimap call LPC
directly and render what it returns:

```c
// LPC (in the player object): expose game state and actions to the page
private mixed ui_inventory(string *args, int id) {
    return json_encode(map(all_inventory(this_object()), (: $1->short() :)));
}
private mixed ui_go(string *args, int id) {
    return do_go(args[0]) ? "ok" : "blocked";
}
void setup_ui() {
    js_export("inventory", (: ui_inventory :));
    js_export("go", (: ui_go :));
}
```

```js
// JS: a sidebar that refreshes after every move
document.querySelectorAll('button.dir').forEach((b) =>
  b.onclick = async () => {
    await fluffos.callLPC('go', b.dataset.dir);
    render(JSON.parse(await fluffos.callLPC('inventory')));
  });
```

The same channel works for tooling: a "run tests" button
(`js_export("run_tests", ...)`), a live object inspector, or a debug
panel querying `query_load_av()` — all without touching the telnet
stream.

## Recipe: pull the web into the game (fetch)

```js
M.fluffos.handlers.fetch_json = async (url) =>
    JSON.stringify(await (await fetch(url)).json());
```

```c
void got_weather(string body, int ok, int id) {
    if (!ok) return;
    mapping data = json_decode(body);   // your mudlib's JSON package
    tell_object(me, "It is " + data["temperature"] + " degrees outside.\n");
}
js_call("fetch_json", ({ "https://api.example.com/weather" }),
        (: got_weather, this_player() :));
```

Live weather in the tavern, a news board fed by an RSS proxy, real
GitHub issues as quest mobs — anything with a JSON API. CORS applies (it
is the page doing the fetching), so use APIs that allow it or a
same-origin proxy.

## Recipe: graphics from LPC (canvas / WebGL)

The bundled terminal registers a `canvas_draw` handler (try
`jsdemo canvas`); the pattern generalizes to any rendering the page can
do — a `<canvas>` 2D minimap, a WebGL scene, or a DOM panel:

```c
// LPC decides WHAT to draw; JS decides HOW.
void draw_room_map(object room) {
    js_call("canvas_draw", ({ "clear" }));
    foreach (object ob in all_inventory(room))
        js_call("canvas_draw", ({ "circle", "" + ob->query_x(),
                                  "" + ob->query_y(), "8" }));
}
```

For anything nontrivial, send one JSON scene description per frame
instead of one call per primitive, and let the JS handler render it —
`js_call("render_scene", ({ json_encode(scene) }))`. Heartbeats make a
fine frame clock for turn-based visuals; for smooth animation let JS
interpolate between LPC-sent keyframes.

## Recipe: sound and notifications

```js
M.fluffos.handlers.play_tone = (freq, ms) => {
  const ctx = new AudioContext(), o = ctx.createOscillator();
  o.frequency.value = +freq; o.connect(ctx.destination);
  o.start(); o.stop(ctx.currentTime + (+ms) / 1000);
};
M.fluffos.handlers.notify = (title, body) => {
  if (Notification.permission === 'granted') new Notification(title, { body });
};
```

```c
js_call("play_tone", ({ "440", "150" }));            // combat hit
js_call("notify", ({ "You have died.", "Respawning at the temple." }));
```

(The classic MSP sound protocol also still works — the driver negotiates
it over telnet — but the bridge gives you the whole Web Audio API.)

## Recipe: saves that survive a reload (localStorage)

MEMFS writes vanish when the tab closes. Until the IDBFS overlay lands
(roadmap), the bridge makes persistence a ten-liner:

```js
M.fluffos.handlers.store_put = (key, value) =>
    localStorage.setItem('mud:' + key, value);
M.fluffos.handlers.store_get = (key) =>
    localStorage.getItem('mud:' + key) || "";
```

```c
// save (player object): efun-serialized, browser-stored
js_call("store_put", ({ query_name(), save_variable(query_stats()) }));

// restore on login
void on_restore(string data, int ok, int id) {
    if (ok && strlen(data)) set_stats(restore_variable(data));
}
js_call("store_get", ({ query_name() }), (: on_restore, this_object() :));
```

`save_variable()`/`restore_variable()` round-trip any LPC value, so a
player's whole state fits through the bridge as one string.

## Recipe: drive the driver from node (no browser)

The same module runs headless — that is how CI executes the LPC
testsuite inside the wasm driver (`node tools/wasm/run-testsuite.js`).
The pattern is useful beyond testing: batch-compile a mudlib to find
errors, script gameplay for regression tests, or embed an LPC
interpreter in a node service. `js_call` handlers work there too — in
node, `fetch_text` could read files or call services instead.

## Gotchas

* **No eval limit yet on this target**: `while(1);` in LPC blocks the
  tab (deadline-based limiter is on the roadmap).
* **Handlers are per-page trust**: anything you register is callable by
  any LPC code in the bundle. Don't expose `eval`-grade handlers in a
  bundle that runs untrusted mudlib code.
* **js_call inside the testsuite runner** completes on a *later tick*;
  synchronous test files can schedule calls but not await them.
* **Background tabs throttle timers** — the driver catch-up is capped
  (~100 gameticks), so long-suspended tabs resume near real time rather
  than replaying hours.
* Packages needing sockets/processes/native libs (sockets, external,
  db, ffi, pcre, crypto, async) are off; their efuns don't exist there.
