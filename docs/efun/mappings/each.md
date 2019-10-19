---
layout: default
title: mappings / each
---

### NAME

    each() - iterate over the (key, value) pairs in a mapping

### SYNOPSIS

    mixed *each( mapping m, int reset );

### DESCRIPTION

    The  (key,  value)  pairs  in  a mapping may be iterated over using the
    each() efun.  each() returns the (key, value) pairs in the  same  order
    as  keys()  and values() do.  each() returns a null vector when the end
    of the mapping is reached.  The next access to each() (after  receiving
    the null vector) will start over at the beginning of the map.  The cur‐
    rent position in the map always resets to the beginning when a  differ‐
    ent  object  accesses  the  map  via  each().  Given the singlethreaded
    nature of MudOS, this doesn't present a problem (and in fact makes sure
    that  an  object  can't  leave  the  current  position in an in-between
    state).  If the optional second argument is given as 1,  then  each()'s
    position in mapping 'm' is reset to the beginning.

    For example:

        mixed *pair;

        while ((pair = each(x)) != ({})) {
          write("key   = " + pair[0] + "\n");
          write("value = " + pair[1] + "\n");
        }

### SEE ALSO

    keys(3), values(3)

