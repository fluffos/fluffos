---
layout: default
title: mudlib / export_uid
---

### NAME

    export_uid() - set the uid of another object

### SYNOPSIS

    int export_uid( object ob );

### DESCRIPTION

    Set  the  uid of <ob> to the effective uid of this_object(). It is only
    possible when <ob> has an effective uid of 0.

### SEE ALSO

    this_object(3), seteuid(3), getuid(3), geteuid(3),  previous_object(3),
    valid_seteuid(4)

