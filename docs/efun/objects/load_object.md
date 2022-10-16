---
layout: default
title: objects / load_object
---

### NAME

    load_object() - find or load an object by file name

### SYNOPSIS

    object load_object( string str );

### DESCRIPTION

    Find  the  object with the file name 'str'.  If the file exists and the
    object hasn't been loaded yet, it is loaded and returned (if possible).

### RETURN VALUES

    load_object() returns the loaded object if it can be loaded (or already
    loaded), otherwise the value 0 will be returned.

### SEE ALSO

    file_name(3), find_object(3), stat(3)

