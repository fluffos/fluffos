---
layout: default
title: system / replace_program
---

### NAME

    replace_program() - replaces the program in this_object()

### SYNOPSIS

    void replace_program( string str );

### DESCRIPTION

    replace_program() replaces the program in this_object() with that of an
    object it inherits.  The string argument is the filename of the  object
    whose  program  is  to  be used.  Once the replacement takes place, the
    current object effectively becomes a clone of that  other  object,  but
    with  its  current  filename and global variable values. The program is
    not actually replaced until the current execution is completed.

### SEE ALSO

    clone_object(3), new(3)

