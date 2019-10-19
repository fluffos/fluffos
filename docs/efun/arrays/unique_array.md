---
layout: default
title: arrays / unique_array
---

### NAME

    unique_array() - partitions an array of objects into groups

### SYNOPSIS

    mixed  unique_array( object *obarr, string separator, void | mixed skip
    ); mixed unique_array( mixed *arr, function f, void | mixed skip );

### DESCRIPTION

    Groups objects together for which the 'separator' function returns  the
    same  value.  'obarr'  should  be  an array of objects, other types are
    ignored.  The 'separator' function is called only once in  each  object
    in  'obarr'.   THe  optional  'skip'  parameter enables a pre-filter on
    'obarr', skipping elements which match 'skip'.  The second form works a
    bit differently.  each element of the array is passed to f and the ele‐
    ments are partitioned based on the return value of f.   In  particular,
    the array does not need to be composed of objects.

### RETURN VALUE

    The return value is an array of arrays of objects on the form:

         ({
             ({Same1:1, Same1:2, Same1:3, .... Same1:N }),
             ({Same2:1, Same2:2, Same2:3, .... Same2:N }),
             ({Same3:1, Same3:2, Same3:3, .... Same3:N }),
                           ....
             ({SameM:1, SameM:2, SameM:3, .... SameM:N }),
         })

