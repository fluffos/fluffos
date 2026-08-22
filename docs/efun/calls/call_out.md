---
title: calls / call_out
---
# call_out

### NAME

    call_out - delayed function call in same object

### SYNOPSIS

    int call_out( string | function fun, int | float delay, mixed arg ... );
    promise call_out( int | float delay );

### DESCRIPTION

    Schedule a future call of function <fun> in this_object(). The call will
    take place in <delay> seconds, with each of the arguments <arg> provided.
    <arg> can be of any type.

    The second form -- a delay with NO callback -- schedules a timer and
    returns a promise instead of a handle: fulfilled with 0 when the delay
    elapses, rejected if the call_out is removed (remove_call_out(3) with no
    argument sweeps it with the rest) or this object is destructed first.
    Inside an async function, `await call_out(delay)` is the non-blocking
    pause idiom. Extra arguments are an error in this form (there is nothing
    to call with them), and no handle is returned, so a timer you may need
    to cancel individually should use the classic form. In call_out_info(4)
    such a timer's function slot reads "<timer>".

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
    of call_outs(0)s which may be executed on the same gametick may be
    configured via the "call_out(0) nest level" property in the runtime config.

    Please note that you can't rely on write() or say() in <fun> since
    this_player() is set to 0. Use tell_object() instead.

    The runtime config value "this_player in call_out" exists to remedy the
    above problem.

    The return value is an integer representing the handle of the call_out
    which may be used as an argument to remove_call_out() or
    find_call_out(). The handle remains valid until the call_out fires or
    is removed, regardless of how many newer call_outs are scheduled after
    it. Handles are always positive; 0 is never a valid handle.

### SEE ALSO

    remove_call_out, call_out_info, call_out_walltime

