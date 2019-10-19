---
layout: default
title: mudlib / geteuid
---

### NAME

    geteuid() - return the effective user id of an object or function

### SYNOPSIS

    string geteuid( object|function );

### DESCRIPTION

    If  given  an  object  argument,  geteuid returns the effective user id
    (euid) of the object.  If given an  argument  of  type  'function',  it
    returns  the  euid of the object that created that 'function' variable.
    If the object, at the time of the function variable's construction, had
    no euid, the object's uid is stored instead.

### SEE ALSO

    seteuid(3), getuid(3), functionp(3), export_uid(3), previous_object(3),
    this_object(3), valid_seteuid(4)

