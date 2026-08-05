---
title: internals / debugger_attached
---
# debugger_attached

### NAME

    debugger_attached() - test whether an LPC debugger client is attached

### SYNOPSIS

    int debugger_attached();

### DESCRIPTION

    Returns 1 if a WebSocket LPC debugger client is currently attached to
    the driver, 0 otherwise. Cheap to call, so it can guard debugger-only
    diagnostics (e.g. before calling `debug_break()`, or before building an
    expensive debug value) without cost when no debugger is attached.

    See the [WebSocket LPC Debugger](../../concepts/general/debugger.md)
    guide for how to enable the debugger (`debugger port` in the config
    file).

### EXAMPLES

    ```c
    void process_order(mapping order) {
        if (debugger_attached() && order["total"] < 0) {
            debug_break();
        }
        // ...
    }
    ```

### SEE ALSO

    debug_break(3)
