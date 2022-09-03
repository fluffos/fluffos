---
layout: default
title: system / inherits
---

### NAME

    inherits() - determine if an object inherits a given file

### SYNOPSIS

    int inherits( string file, object obj );

### DESCRIPTION

    inherits() returns 0 if obj does not inherit file, 1 if it inherits the
    most recent copy of file, and 2 if it inherits an old copy of file.

    If no object is passed as the second argument, this efun will default
    to this_object().

### SEE ALSO

    deep_inherit_list(3), inherit_list(3)

