---
layout: default
title: general / undefinedp
---

### NAME

    undefinedp() - determine whether or not a given variable is undefined.

### SYNOPSIS

    int undefinedp( mixed arg );

### DESCRIPTION

    Return 1 if 'arg' is undefined.  'arg' will be undefined in the follow‐
    ing cases:

    1.  it is a variable set equal to the return value of a call_other to a
        non-existent method (e.g. arg = call_other(obj, "???")).

    2.  it  is  a variable set equal to the return value of an access of an
        element  in  a   mapping   that   doesn't   exist   (e.g.   arg   =
        map[not_there]).

### SEE ALSO

    mapp(3),  stringp(3),  pointerp(3),  objectp(3),  intp(3),  bufferp(3),
    floatp(3), functionp(3), nullp(3), errorp(3)

