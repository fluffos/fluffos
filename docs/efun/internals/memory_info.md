---
layout: default
title: internals / memory_info
---

### NAME

    memory_info - obtain info on object/overall memory usage

### SYNOPSIS

    varargs int memory_info( object ob );

### DESCRIPTION

    If  optional argument 'ob' is given, memory_info() returns the approxi‐
    mate amount of memory that 'ob' is using.  If  no  argument  is  given,
    memory_info()  returns the approximate amount of memory that the entire
    mud is using.  Note that the amount of memory the mud is using does not
    necessarily  correspond  to  the amount of memory actually allocated by
    the mud from the system.

### SEE ALSO

    debug_info(3), malloc_status(3), mud_status(3)

