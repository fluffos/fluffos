---
layout: doc
title: arrays / filter_array
---
# filter_array

### NAME

    filter_array() - return a selective sub-array

### SYNOPSIS
```c
mixed *filter_array( mixed *arr, string fun, object|string ob, mixed extra, ... );
mixed *filter_array( mixed *arr, function f, mixed extra, ...);
```
### DESCRIPTION

    filter_array() returns an array holding the items of <arr> which passed
    sucessfully through the function <fun> found in object <ob>.  The func‐
    tion  <fun>  is  called  for each element in <arr> with that element as
    parameter.  The second parameter <extra> and following  parameters  are
    sent  in  each  call  if given.  An object is considered to have passed
    sucessfully through <fun> (and hence is included in the  return  array)
    if  <fun>  returns  1.  If f is passed it is used instead of ob->fun().
    If <arr> is not an array, then 0 will be returned.

### SEE ALSO

    filter(3), map(3), sort_array(3), unique_array(3)

