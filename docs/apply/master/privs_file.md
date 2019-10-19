---
layout: default
title: master / privs_file
---

### NAME

    privs_file  -  specifies  the  privs  string to give to a newly created
    object

### SYNOPSIS

    string privs_file( string filename );

### DESCRIPTION

    The privs_file() function is called in the master  object  when  a  new
    file  is  created.  The 'filename' of the object is passed as the argu‐
    ment, and the string that privs_file()  returns  is  used  as  the  new
    object's privs string.

    The  privs_file() functionality is only available if the driver is com‐
    piled with the PRIVS option defined.

### SEE ALSO

    query_privs(3), set_privs(3)

