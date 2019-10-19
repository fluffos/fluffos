---
layout: default
title: mappings / unique_mapping
---

### NAME

    unique_mapping() - create a mapping from an array based on a function

### SYNOPSIS

    mapping unique_mapping( mixed *arr, string fun, object ob,
                              mixed extra, ... );
    mapping unique_mapping( mixed *arr, function f, mixed extra, ... );

### DESCRIPTION

    Returns a mapping built in the following manner:

    'ob->fun()' or 'f' is evaluated for each member of the array.  The mem‐
    bers for  which  the  function  returns  the  same  value  are  grouped
    together, and associated with the return value as the key.

    The  key/value  ordering  in the returned mapping is consistent but not
    guaranteed to not change in the future.

### SEE ALSO

    filter_array(3), sort_array(3), map(3)

