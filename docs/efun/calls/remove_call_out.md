---
layout: default
title: calls / remove_call_out
---

### NAME

    remove_call_out() - remove a pending call_out

### SYNOPSIS

    int remove_call_out( string fun ); int remove_call_out();

### DESCRIPTION

    In  the  first form this function removes the next pending call out for
    function 'fun' in the current object.  The return  value  is  the  time
    remaining  before  the callback is to be called, or -1 if there were no
    call out pending to this function.

    In the second form all pending call outs for the current object will be
    removed.  In this case the return value is always 0.

### SEE ALSO

    call_out(3), call_out_info(3).

