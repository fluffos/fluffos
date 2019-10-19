---
layout: default
title: system / function_profile
---

### NAME

    function_profile() - get function profiling information for an object

### SYNOPSIS

    mixed *function_profile( object ob );

### DESCRIPTION

    Returns  function  profiling  information for 'ob', or this_object() if
    'ob' is not specified.  This is only available if the driver  was  com‐
    piled with PROFILE_FUNCTIONS defined.

### RETURN VALUE

    An  array  of  mappings is returned, one for each function in 'ob', the
    format of the mapping is:
           ([ "name"     : name_of_the_function,
              "calls"    : number_of_calls,

              /* cpu time expressed in microseconds */
              "self"     : cpu_time_spent_in self,
              "children" : cpu_time_spent_in_children
           ])
    The usefulness of this is tied to the resolution of the CPU clock--even
    though  the  units  are microseconds, the CPU timer resolution is often
    much less

### SEE ALSO

    rusage(3), time_expression(3), opcprof(3)

