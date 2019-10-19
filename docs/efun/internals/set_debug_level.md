---
layout: default
title: internals / set_debug_level
---

### NAME

    set_debug_level()  -  sets the debug level used by the driver's debug()
    macro

### SYNOPSIS

    void set_debug_level( int level );

### DESCRIPTION

    This efun is only available when the  driver  is  compiled  with  -DDE‐
    BUG_MACRO.  The purpose of this efun is to allow the amount and type of
    debugging information produced to be controlled  from  within  the  mud
    (while the driver is running).

    For  more information, read the file debug.h which is included with the
    driver source.

### SEE ALSO

    set_malloc_mask(3)

