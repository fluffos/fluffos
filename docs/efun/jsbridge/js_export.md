---
title: jsbridge / js_export
---
# js_export

### NAME
    js_export - register an LPC function the hosting page can call

### SYNOPSIS
    int js_export(string name);                              // unregister
    int js_export(string name, string | function callback);  // register

### DESCRIPTION
    Registers `callback` under `name` so the page's JavaScript can call
    into LPC:

        // JS (Promise of the string result)
        const sum = await Module.fluffos.callLPC("add", "2", "3");

    The callback runs on the next driver tick (proper driver context,
    like call_out) as:

        mixed callback(string *args, int id);

    `args` are the JavaScript arguments as strings; `id` is the call id.
    The return value settles the page's Promise: a string is passed
    through as-is, other values are %O-formatted (integers and floats
    print plainly; return a JSON-encoded string for structured data). A
    runtime error in the callback, or a destructed owner object, rejects
    the Promise.

    A string callback names a function called on the registering object.
    Re-registering a name replaces the previous entry; calling with only
    `name` unregisters it. Returns 1 when a callback was registered, 0
    when the name was (or is now) unregistered.

    Like call_out(1), the callback runs without a command context:
    this_player() is 0 inside it.

    Only available on the WebAssembly build (guard LPC code with
    `#ifdef __PACKAGE_JSBRIDGE__`).

### EXAMPLES
    ```c
    private mixed lpc_add(string *args, int id) {
        int total = 0;
        foreach (string a in args) total += to_int(a);
        return total;
    }

    void create() { js_export("add", (: lpc_add :)); }
    ```

    Page side (helper included in the bundled web shell):
    ```js
    const sum = await fluffos.callLPC("add", "2", "3");   // "5"
    ```

### SEE ALSO
    js_call(3), js_eval(3), call_out(3)

### NOTE
    Exports are driver-global (one namespace per driver instance) and
    hold a reference on the registering object until unregistered. The
    page can only call names the mudlib chose to export -- the export
    table is the mudlib's capability surface toward the page, the mirror
    of the page's handler table.
