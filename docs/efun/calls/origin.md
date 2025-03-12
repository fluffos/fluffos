---
layout: doc
title: calls / origin
---
# origin

### NAME

    origin() - determine how the current function was called

### SYNOPSIS

    string origin(void);

### DESCRIPTION

    Returns string specifying how the current function was called.
    Current values are:

    "driver"            Driver (heart_beats, etc)
    "local"             Local function call
    "call_other"        call_other()
    "simul"             simul_efun object via a simul_efun call
    "internal"          call_out(), etc
    "efun"              called by an efun (sort_array, etc)
    "function pointer"  function_pointer
    "functional"        functional

### SEE ALSO

    previous_object(3), /include/origin.h
