---
layout: default
title: mappings / keys
---

### NAME

    keys()  -  return an array of the keys from the (key, value) pairs in a
    mapping

### SYNOPSYS

    mixed *keys( mapping m );

### DESCRIPTION

    keys() returns an array of keys (indices) corresponding to the keys  in
    the (key, value) pairs stored in the mapping m.

    For example, if:

        mapping m;
        m = (["hp" : 35, "sp" : 42, "mass" : 100]);

    then

        keys(m) == ({"hp", "sp", "mass"})

    Note:  the  keys  will not be returned in any apparent order.  However,
    they will be returned in the same order  as  the  corresponding  values
    (returned by the values() efun).

### SEE ALSO

    values(3), each(3)

