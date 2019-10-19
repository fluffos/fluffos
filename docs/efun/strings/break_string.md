---
layout: default
title: strings / break_string
---

### NAME

    break_string() - break a string at regular intervals

### SYNOPSIS

    string break_string( string str | int foobar, int len,
                         void | int indent | string indent );

### DESCRIPTION

    This  function  breaks  a  continous  string  without newlines into a a
    string with newlines inserted at every 'len':th character.

    If 'indent' is defined and is given as an integer, 'indent'  number  of
    spaces  are  inserted after every newline. If 'indent' is a string that
    string is inserted before every newline.

    If the first argument is an integer, the function simply returns 0.

### BUGS

    break_string() is *not* context sensitive.  It does not take account of
    periods '.' and new-lines '\n'.  This means that the result is not nec‐
    essarily what you would expect.  My experience is that you  should  use
    one  space  after  a period and do not embed new-lines in the text, but
    append them to the result of break_string().  If this format  does  not
    meet  your  tastes  you  may  find  break_string() to be inadequate.  A
    future revision of break_string()  may  allow  different  line-breaking
    semantics.  (Cygnus)

### SEE ALSO

    implode(3), explode(3), sprintf(3)

