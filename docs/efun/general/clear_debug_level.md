---
layout: doc
title: general / clear_debug_level
---
# clear_debug_level

### NAME

    clear_debug_level

### SYNOPSIS

    void clear_debug_level(string)

### DESCRIPTION

    This efun will toggle off the debug level for the specified option. The
    options are the same as for set_debug_level() when using a string.

        call_out
        d_flag
        connections
        mapping
        sockets
        comp_func_tab
        LPC
        LPC_line
        event
        dns
        file
        add_action
        telnet
        websocket

### EXAMPLE

```c
clear_debug_level("call_out");
```

### SEE ALSO

    set_debug_level(), debug_levels()
