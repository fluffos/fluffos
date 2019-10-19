---
layout: default
title: master / valid_seteuid
---

### NAME

    valid_seteuid - secures the use of seteuid(3)

### SYNOPSIS

    int valid_seteuid( object obj, string euid );

### DESCRIPTION

    The  driver  calls  valid_seteuid(ob,  euid)  in the master object from
    inside the seteuid(euid) efunction.  If valid_seteuid() returns 0, then
    the  seteuid()  call will fail.  If valid_seteuid() returns 1, then the
    seteuid(3) will succeed.

### SEE ALSO

    seteuid(3), geteuid(3), getuid(3), export_uid(3)

