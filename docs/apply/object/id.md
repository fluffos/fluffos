---
layout: default
title: object / id
---

### NAME

    id - function called by present() in order to identify an object

### SYNOPSIS

    int id( string an_id );

### DESCRIPTION

    The  present(3)  efunction calls id() to determine if a given object is
    named by a given string.  id() should return 1 if the object wishes  to
    be known by the name in the string anId; it should return 0 otherwise.

### SEE ALSO

    present(3)

