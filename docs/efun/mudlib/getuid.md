---
layout: default
title: mudlib / getuid
---

### NAME

    getuid() - return the user id (uid) of an object

### SYNOPSIS

    string getuid( object ob );

### DESCRIPTION

    Returns  the  user id of an object.  The uid of an object is determined
    at object creation by the creator_file() function.

### SEE ALSO

    seteuid(3),   geteuid(3),   export_uid(3),    this_object(3),    previ‐
    ous_object(3), creator_file(4), valid_seteuid(4)

