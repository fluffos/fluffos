---
title: jsbridge / js_call
---
# js_call

### NAME
    js_call - call a JavaScript handler registered by the hosting page

### SYNOPSIS
    int js_call(string handler, string *args);
    int js_call(string handler, string *args, string | function callback);

### DESCRIPTION
    Invokes `handler` from the page's handler table
    (`Module.fluffos.handlers` in JavaScript) with `args` spread as
    individual string arguments. The handler may return a value or a
    Promise; when it settles, the callback runs on a later driver tick:

        void callback(string result, int success, int id);

    `success` is 1 with the handler's return value stringified (objects
    JSON-encoded) as `result`, or 0 with an error description (missing
    handler, thrown exception, rejected Promise). Returns a nonnegative
    call id, also passed to the callback.

    Like call_out(1), the callback runs without a command context:
    this_player() is 0 inside it — capture the player in the closure and
    use tell_object(3).

    Only available on the WebAssembly build (guard LPC code with
    `#ifdef __PACKAGE_JSBRIDGE__`).

### EXAMPLES
    ```c
    void got_body(object me, string body, int success, int id) {
        if (me) tell_object(me, success ? body : "fetch failed: " + body);
    }

    int cmd(string url) {
        js_call("fetch_text", ({ url }), (: got_body, this_player() :));
        return 1;
    }
    ```

    Page side:
    ```js
    Module.fluffos.handlers = {
      fetch_text: async (url) => (await fetch(url)).text(),
    };
    ```

### SEE ALSO
    js_eval(3), call_out(3), resolve(3)

### NOTE
    Arguments and results cross the bridge as strings; JSON-encode
    structured data on either side. The page controls which handlers
    exist — a bundle's JavaScript capability surface is exactly its
    handler table.
