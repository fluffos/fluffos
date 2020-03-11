---
layout: default
title: objects / find_object
---

### NAME

    find_object() - find an object by file name

### SYNOPSIS

    object find_object( string str );
    object find_object( string str, int flag );

### DESCRIPTION

    Find the object with the file name <str>. If the object is a cloned
    object, then it can be found using the file name which would by
    returned if file_name() was called with it as the argument.

    If 'flag' is 1, this function has the same effect as 'load_object'.

### SEE ALSO

    file_name(3), stat(3), load_object(3)
