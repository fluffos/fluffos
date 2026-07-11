---
title: jsbridge / js_eval
---
# js_eval

### NAME
    js_eval - evaluate JavaScript in the hosting page, synchronously

### SYNOPSIS
    string js_eval(string code);

### DESCRIPTION
    Evaluates `code` as JavaScript in the context of the page (or node
    process) hosting the WASM driver and returns the result as a string:
    primitives are stringified, objects are JSON-encoded, undefined/null
    become "". A thrown JavaScript exception is returned as the string
    "JS error: ..." — it never unwinds into the driver.

    Only available on the WebAssembly build (guard LPC code with
    `#ifdef __PACKAGE_JSBRIDGE__`).

### EXAMPLES
    ```c
    write(js_eval("navigator.userAgent"));
    int width = to_int(js_eval("window.innerWidth"));
    ```

### SEE ALSO
    js_call(3)

### NOTE
    A page can disable eval entirely with a Content-Security-Policy; use
    js_call(3) with page-registered handlers for anything structural.
