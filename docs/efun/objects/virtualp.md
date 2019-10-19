---
layout: default
title: objects / virtualp
---

### NAME

    virtualp()  -  determine  whether  a given variable points to a virtual
    object

### SYNOPSIS

    int virtualp( object arg );

### DESCRIPTION

    Returns true (1) if the argument is objectp() and the O_VIRTUAL flag is
    set.   The driver sets the O_VIRTUAL flag for those objects created via
    the 'compile_object' method in master.c.

### SEE ALSO

    clonep(3), userp(3), wizardp(3), objectp(3),  new(3),  clone_object(3),
    call_other(3), file_name(3)

