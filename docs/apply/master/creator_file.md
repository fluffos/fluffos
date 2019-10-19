---
layout: default
title: master / creator_file
---

### NAME

    creator_file - specifies the uid to give to a newly created object

### SYNOPSIS

    string creator_file(string filename);

### DESCRIPTION

    The  creator_file() function is called in the master object each time a
    new object is created.  The 'filename' of the object is passed  as  the
    sole  parameter,  and  the string that creator_file() returns is set as
    the new object's uid.  If the AUTO_SETEUID option is  enabled  at  com‐
    pile-time of the driver, it is also set as the new object's euid.

    One exception: if the AUTO_TRUST_BACKBONE option is enabled at compile-
    time of the driver, and creator_file() returns  the  backbone  uid  (as
    specified  by  get_bb_uid()  in the master object), the object is given
    the uid and euid of the object that loaded it.

### SEE ALSO

    seteuid(3), new(3), clone_object(3), call_other(3)

