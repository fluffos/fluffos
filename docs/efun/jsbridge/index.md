---
title: jsbridge
---

# jsbridge

LPC ↔ JavaScript bridge efuns. **Only available on the WebAssembly build
of the driver** (`__PACKAGE_JSBRIDGE__`): they call into the JavaScript
of the page (or node process) hosting the driver.

See the [WASM driver cookbook](../../driver/wasm.md) for recipes
(fetch, canvas, audio, persistent saves) and
[Build (WebAssembly)](../../build-wasm.md) for the build workflow.

* [js_eval](js_eval)
* [js_export](js_export)
* [js_call](js_call)
