---
title: internals / destructed_objects
---
# destructed_objects

### NAME

    destructed_objects() - list destructed objects still referenced

### SYNOPSIS

    mixed *destructed_objects();

### DESCRIPTION

    Returns an array of the objects that have been destructed but are
    still referenced somewhere -- the driver's "dangling" list. Each
    element is itself an array of two items: the object's name (a string)
    and its current reference count (an int).

    An object appears here when it has been destructed while another
    svalue still holds a pointer to it; the entry is a leak-hunting aid
    for tracking down which destructed objects are being kept alive and
    by how many references.

### SEE ALSO

    refs(3)
