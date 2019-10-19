---
layout: default
title: mappings / values
---

### NAME

    values() - return an array of the values from the (key, value) pairs
               in a mapping

### SYNOPSIS

    mixed *values( mapping m );

### DESCRIPTION

    values() returns an array of values corresponding to the value elements
    in the (key, value) pairs stored in the mapping m.

    For example, if:

        mapping m;

        m = (["hp" : 35, "sp" : 42, "mass" : 100]);

    then

        values(m) == ({35, 42, 100})

    Note: the values will be returned in the same order as the  correspond‐
    ing keys.

### SEE ALSO

    keys(3), each(3)

