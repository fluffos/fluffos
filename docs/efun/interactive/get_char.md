---
title: interactive / get_char
---
# get_char

### NAME

    get_char  -  causes  next  character of input to be sent to a specified
    function

### SYNOPSIS

    varargs int get_char( string | function fun, int flag, ... );

### DESCRIPTION

    Enable next character of user input to be sent to the function 'fun' as
    an argument. The input character will not be parsed by the driver.

    Note  that get_char is non-blocking which means that the object calling
    get_char does not pause waiting for input.  Instead the object  contin‐
    ues  to  execute  any statements following the get_char.  The specified
    function 'fun' will not be called until the user input  has  been  col‐
    lected.

    If  "get_char()"  is  called more than once in the same execution, only
    the first call has any effect.

    If optional argument 'flag' is non-zero, the char given by  the  player
    will not be echoed, and is not seen if snooped (this is useful for col‐
    lecting passwords).

    The function 'fun' will be called with the  user  input  as  its  first
    argument (a string). Any additional arguments supplied to get_char will
    be passed on to 'fun' as arguments following the user input.

### DELIVERY

    Exactly one keystroke is delivered per callback:

    - A printable character arrives as itself.  A multi-byte UTF-8  charac‐
      ter  is  delivered  whole,  as one valid one-character string (it is
      never split into fragment bytes).
    - Control bytes arrive verbatim, including Backspace (0x08) and Delete
      (0x7f) -- char mode performs no line editing.
    - Escape sequences (arrow keys, function keys)  arrive  byte-by-byte:
      ESC, '[', 'A' are three separate callbacks.  Reassembling  them  is
      the mudlib's job; see /std/tui/keys.lpc in the testsuite mudlib for
      a full decoder, and the TUI library built on top of it.

    Char mode is one-shot: after each callback the driver reverts to  line
    mode  unless  the callback re-arms get_char() before returning.  Pass‐
    ing the no-echo flag on every re-arm keeps client echo off across  the
    whole exchange.

### SEE ALSO

    call_other(3), call_out(3), input_to(3)
