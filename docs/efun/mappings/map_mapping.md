---
layout: default
title: mappings / map_mapping
---

### NAME

    map_mapping()  -  modify  an  mapping  of elements via application of a
    function

### SYNOPSIS

    mapping map_mapping( mapping map, string fun, object ob,
                         mixed extra, ... );
    mapping map_mapping( mapping map, function f, mixed extra, ... );

### DESCRIPTION

    Returns an mapping with the same keys as  map  whose  items  have  been
    mapped  throught  'ob->fun()'  or 'f'.  The function is called for each
    element in 'map' and the return value is  put  back  in  that  spot  in
    'map'.   'extra' and following are passed as parameters to the function
    after the item.

### SEE ALSO

    filter_array(3), sort_array(3), map(3)

