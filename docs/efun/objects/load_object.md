---
layout: default
title: objects / load_object
---

### NAME

    load_object() - find or load an object by file name

### SYNOPSIS

    object load_object( string str );

### DESCRIPTION

    Find  the  object with the file name <str>.  If the file exists and the
    object hasn't been  loaded  yet,  it  is  loaded.   Otherwise  zero  is
    returned.

### SEE ALSO

    file_name(3), stat(3), find_object(3)

