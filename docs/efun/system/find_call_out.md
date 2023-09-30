---
layout: default
title: system / find_call_out
---

### NAME

    find_call_out() - find a call out scheduled to be called

### SYNOPSIS

    int find_call_out( string func ) ;
    int find_call_out( int handle ) ;

### DESCRIPTION

    Find a call out scheduled to be called either by function name or by
    handle.

    If the argument is a string, then the first call out due to be executed
    for function 'func' is found, and the time left in seconds is returned.
    If no call out is found, then return -1.

    If the argument is an integer, then the call out with handle 'handle'
    is found, and the time left in seconds is returned. If no call out is
    found, then return -1.

    This will find call outs scheduled by call_out() or call_out_walltime().

    If the time remaining is fractional, then the result will be an integer,
    the value of which will be floored.

    Example:
    int handle = call_out("func", 10);
    printf("Remaining time: %O\n", find_call_out(handle));
    // Remaining time: 10

    int handle = call_out("func", 10.75);
    printf("Remaining time: %O\n", find_call_out(handle));
    // Remaining time: 10

    This efun will only find call outs initiated by this_object().

### SEE ALSO

    call_out(3), call_out_walltime(3), remove_call_out(3), set_heart_beat(3)
