---
layout: default
title: general / map
---

### NAME

    map()  - modify an mapping, array, or string via application of a func‐
    tion

### SYNOPSIS

    mapping map( mapping | mixed* | string x, string fun, object ob,
                 mixed extra, ... );
    mapping map( mapping | mixed* | string x, function f, mixed extra, ... );

### DESCRIPTION

    If the first argument is a mapping, map() behaves exactly like map_map‐
    ping().  If it is an array, map() behaves exactly like map_array().  If
    it's argument is a string, map() passes each character (as an  int)  to
    the  function,  and replaces the character with the return value if the
    return value is a non-zero integer.

### SEE ALSO

    filter_array(3), sort_array(3), map_array(3), map_mapping(3)

