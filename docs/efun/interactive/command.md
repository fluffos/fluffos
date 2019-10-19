---
layout: default
title: interactive / command
---

### NAME

    command() - execute a command as if given by the object

### SYNOPSIS

    int command( string str, object ob );

### DESCRIPTION

    Execute  'str'  for  object  'ob', or this_object() if 'ob' is omitted.
    Note that the usability of the second argument  is  determined  by  the
    local administrator and will usually not be available, in which case an
    error will result.  In case of failure,  0  is  returned,  otherwise  a
    numeric  value  is  returned, which is the LPC "evaluation cost" of the
    command.  Bigger numbers mean higher cost, but the whole scale is  sub‐
    jective and unreliable.

### SEE ALSO

    add_action(3), enable_commands(3)

