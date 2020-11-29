---
layout: default
title: calls / call_out
---

### NAME

    call_out - delayed function call in same object

### SYNOPSIS

    int call_out( string | function fun, int | float delay, mixed arg ... );

### DESCRIPTION

    Schedule a future call of function <fun> in this_object(). The call will
    take place in <delay> seconds, with each of the arguments <arg> provided.
    <arg> can be of any type.

    If the gametick in the runtime config is less than 1000, you may specify
    a <delay> as a float in milliseconds (gametick / 1000) representing a 
    value that is divisible by the gametick. A <delay> which is not equally
    divisible by the gametick value will be rounded up to the next game tick.
    A <delay> of 0 is special, see below.

    For example, if your gametick is set to 250ms in the runtime config,
    you may perform callouts with a granularity of a quarter of a second.
    call_out( "function", 0.75 ) will execute the <fun> in 3 game ticks
    (750 milliseconds), thereby affording more precision over timing
    when calling out.

    A <delay> of 0 (or 0.0), will perform <fun> on the same gametick in which
    it was called after all previous call_outs have been executed. The number
    of call_outs(0)s which may be executed on the same gametick may be configured
    via the "call_out(0) nest level" property in the runtime config.

    Please note that you can't rely on write() or say() in <fun> since
    this_player() is set to 0. Use tell_object() instead.

    The runtime config value "this_player in call_out" exists to remedy the above
    problem.

    The return value is an integer representing the handle of the call_out
    which may be used as an argument to remove_call_out().

### SEE ALSO

    remove_call_out(3), call_out_info(3)

