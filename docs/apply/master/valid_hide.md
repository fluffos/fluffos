---
layout: default
title: master / valid_hide
---

### NAME

    valid_hide - allows or disallows objects to hide and see hidden objects

### SYNOPSIS

    int valid_hide( object ob );

### DESCRIPTION

    Add  valid_hide  to  master.c  in  order to allow objects to hide them‐
    selves, or see other objects that  have  hidden  themselves.   When  an
    object tries to use the set_hide() efun to hide itself, valid_hide will
    be called with the object that is wanting to hide as the  sole  parame‐
    ter.   It  should return 1 to allow it, or 0 to not allow it.  The same
    call takes place when it needs to be determined  if  a  certain  object
    should be able to see hidden objects.

### SEE ALSO

    set_hide(3)

