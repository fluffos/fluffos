---
layout: doc
title: internals / set_debug_level
---
# set_debug_level

### NAME

    set_debug_level()  -  sets the debug level used by the driver's debug()
    macro

### SYNOPSIS

    void set_debug_level( int | string level );

### DESCRIPTION

    The purpose of this efun is to allow the amount and type of
    debugging information produced to be controlled from within the mud
    (while the driver is running).

    The information is printed to stdout as well as to the file specified in
    the runtime configuration file as the "debug log file" setting.

    The level is a bitmask integer or a string. If using an integer, multiple 
    levels can be set by using the bitwise OR operator (|).

    The following levels are available:
        "call_out"        1 << 0
        "d_flag"          1 << 2
        "connections"     1 << 3
        "mapping"         1 << 4
        "sockets"         1 << 5
        "comp_func_tab"   1 << 6
        "LPC"             1 << 7
        "LPC_line"        1 << 8
        "event"           1 << 9
        "dns"             1 << 10
        "file"            1 << 11
        "add_action"      1 << 12
        "telnet"          1 << 13
        "websocket"       1 << 14

    When level is an integer, the debug level will be set to that value,
    erasing any previous settings.

    If level is a string, the debug level for that option will be toggled on,
    while all other options will be preserved.

### EXAMPLES
```c
set_debug_level( (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) );
set_debug_level("call_out");
```

### SEE ALSO

    clear_debug_level(), debug_levels()
