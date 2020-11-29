---
layout: default
title: calls / remove_call_out
---

### NAME

    remove_call_out() - remove a pending call_out

### SYNOPSIS

    int remove_call_out( string fun );
    int remove_call_out( int handle );
    int remove_call_out();

### DESCRIPTION

    In  the  first form this function removes the next pending call out for
    function <fun> in the current object.  The return  value  is the number
    of  seconds  remaining  before  the callback was to be called, or -1 if
    there was no call out pending to this function.

    In the second form, this function removes a pending call out identified
    by  the  <handle>  returned  from the  call_out()  function. The return
    value is the number of remaining seconds before the call_out was to  be
    called,  or -1  if  there  was  no  valid  call_out  identified  by the 
    specified <handle>.

    In the third form all pending call outs for the current object will be
    removed. In this case the return value is always 0.

### SEE ALSO

    call_out(3), call_out_info(3).

