---
layout: default
title: objects / restore_object
---

### NAME

    restore_object()  -  restore  values  of  variables from a file into an
    object

### SYNOPSIS

    int restore_object( string name, int flag );

### DESCRIPTION

    Restore values of variables for current object from file 'name'. If the
    optional second argument is 1, then all of the non-static variables are
    not zeroed out prior to restore (normally, they are).

    In the case of an error, the affected variable will be  left  untouched
    and an error given.

### SEE ALSO

    save_object(3)

