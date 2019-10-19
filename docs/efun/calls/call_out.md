---
layout: default
title: calls / call_out
---

### NAME

    call_out() - delayed function call in same object

### SYNOPSIS

    void call_out( string | function fun, int delay, mixed arg );

### DESCRIPTION

    Set  up  a  call of function <fun> in this_object(). The call will take
    place in <delay> seconds, with the argument <arg> provided.  <arg>  can
    be of any type.

    Please  note  that  you  can't  rely on write() or say() in <fun> since
    this_player() is set to 0. Use tell_object() instead.

    The define THIS_PLAYER_IN_CALL_OUT exists to remedy the above problem.

### SEE ALSO

    remove_call_out(3), call_out_info(3)

