---
title: internals / debug_break
---
# debug_break

### NAME

    debug_break() - programmatic breakpoint for the LPC debugger

### SYNOPSIS

    void debug_break();

### DESCRIPTION

    Pauses the VM at the next executed instruction when a WebSocket LPC
    debugger client is attached, equivalent to hitting a manually-set
    breakpoint at this line. The attached debugger receives a `stopped`
    event and can inspect the call stack, local variables, and object
    state exactly as it would at any other breakpoint.

    When no debugger is attached, `debug_break()` is an exact no-op — safe
    to leave in shipped mudlib code, the same way JavaScript's `debugger;`
    statement is safe in shipped browser code.

    See the [WebSocket LPC Debugger](../../concepts/general/debugger.md)
    guide for how to enable the debugger (`debugger port` in the config
    file).

### EXAMPLES

    ```c
    void process_order(mapping order) {
        if (debugger_attached() && order["total"] < 0) {
            debug_break();  // inspect `order` interactively
        }
        // ...
    }
    ```

### SEE ALSO

    debugger_attached(3)
