---
layout: default
title: master / object_name
---

### NAME

    object_name - called by the driver to find out an object's name

### SYNOPSIS

    string object_name( object );

### DESCRIPTION

    This  master  apply  is called by the sprintf() efun, when printing the
    "value" of an object.  This function should return a string correspond‐
    ing to the name of the object (eg a user's name).

### SEE ALSO

    file_name(3)

