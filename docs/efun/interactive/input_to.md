---
layout: default
title: interactive / input_to
---

### NAME

    input_to()  - causes next line of input to be sent to a specified func‐
    tion

### SYNOPSIS

    varargs void input_to( string | function fun, int flag, ... );

### DESCRIPTION

    Enable next line of user input to be sent to the local  function  'fun'
    as an argument. The input line will not be parsed by the driver.

    Note  that input_to is non-blocking which means that the object calling
    input_to does not pause waiting for input.  Instead the object  contin‐
    ues  to  execute  any statements following the input_to.  The specified
    function 'fun' will not be called until the user input  has  been  col‐
    lected.

    If  "input_to()"  is  called more than once in the same execution, only
    the first call has any effect.

    If optional argument 'flag' is non-zero, the line given by  the  player
    will not be echoed, and is not seen if snooped (this is useful for col‐
    lecting passwords).

    The function 'fun' will be called with the  user  input  as  its  first
    argument (a string). Any additional arguments supplied to input_to will
    be passed on to 'fun' as arguments following the user input.

### SEE ALSO

    call_other(3), call_out(3), get_char(3)

