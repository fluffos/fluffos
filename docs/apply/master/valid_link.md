---
layout: default
title: master / valid_link
---

### NAME

    valid_link - controls the use of link(3)

### SYNOPSIS

    int valid_link( string from, string to );

### DESCRIPTION

    The  driver calls valid_link(from, to) in the master object from inside
    the link(from, to) efunction.  If  valid_link()  returns  0,  then  the
    link()  will  fail.   If  valid_link() returns 1, then the link(3) will
    succeed if rename() would succeed if called with the same arguments.

### SEE ALSO

    link(3)

