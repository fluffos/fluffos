---
layout: default
title: objects / find_object
---

### NAME

    find_object() - find an object by file name

### SYNOPSIS

    object find_object( string str, void | int flag );

### DESCRIPTION

    Find the object with the file name 'str',  which can reference a cloned
    object by using the file name format returned by file_name().

    If 'flag' is 1, find_object() will behave like load_object().

### RETURN VALUES

    find_object() returns the loaded object if it can be located (or loaded
    when 'flag' is 1), otherwise the value 0 will be returned.

### SEE ALSO

    file_name(3), load_object(3), stat(3)
