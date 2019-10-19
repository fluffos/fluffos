---
layout: default
title: calls / origin
---

### NAME

    origin() - determine how the current function was called

### SYNOPSIS

    int origin( void );

### DESCRIPTION

    Returns  an  integer  specifying  how  the current function was called.
    Current values are:

    1      Driver (heart_beats, etc)

    2      Local function call

    4      call_other()

    8      simul_efun object via a simul_efun call

    16     call_out()

    32     called by an efun (sort_array, etc)

    64     function_pointer

    128    functional

### SEE ALSO

    previous_object(3), /include/origin.h

