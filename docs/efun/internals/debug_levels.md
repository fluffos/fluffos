---
layout: doc
title: internals / debug_levels
---
# debug_levels

### NAME

    debug_levels

### SYNOPSIS

    mapping debug_levels()

### DESCRIPTION

    Returns a mapping of the current debug levels, where the keys are the
    names of the debug levels and the values 0 (disabled) or the integer
    value representing the bitmask value of the debug level.

### EXAMPLE

```c
    set_debug_level( (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) );
    printf("%O\n", debug_levels());

    Result:
    ([ /* sizeof() == 14 */
      "telnet" : 0,
      "file" : 0,
      "LPC_line" : 0,
      "mapping" : 16,
      "d_flag" : 4,
      "add_action" : 0,
      "event" : 0,
      "comp_func_tab" : 0,
      "sockets" : 32,
      "dns" : 0,
      "websocket" : 0,
      "LPC" : 0,
      "connections" : 8,
      "call_out" : 1,
    ])
```
### SEE ALSO

    clear_debug_level(), set_debug_level()
