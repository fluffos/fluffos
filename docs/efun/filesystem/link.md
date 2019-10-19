---
layout: default
title: filesystem / link
---

### NAME

    link() - link a file to another

### SYNOPSIS

    void link( string original, string reference );

### DESCRIPTION

    Creates  a  link  <reference> to the file <original>.  This efun causes
    valid_link(original, reference) to be called in the master object.   If
    valid_link() returns 0, the link() call fails.  If valid_link() returns
    1 then the link() suceeds iff rename() would succeed if called with the
    same arguments.

    Note: This efun creates a hard link, not a symbolic one.

### SEE ALSO

    rm(3), rmdir(3), rename(3), mkdir(3), cp(3)

