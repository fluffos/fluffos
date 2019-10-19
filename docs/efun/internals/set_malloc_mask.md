---
layout: default
title: internals / set_malloc_mask
---

### NAME

    set_malloc_mask()  -  sets the mask controlling display of malloc debug
    info

### SYNOPSIS

    void set_malloc_mask( int mask );

### DESCRIPTION

    This efun is only available when DEBUGMALLOC and DEBUGMALLOC_EXTENSIONS
    are  both defined in options.h at driver build time.  The mask controls
    what memory-related debugging information is displayed  as  the  driver
    allocates  and  deallocates memory.  Read md.c in the driver source for
    more information.

### SEE ALSO

    debugmalloc(3)

