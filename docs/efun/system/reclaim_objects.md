---
title: system / reclaim_objects
---
# reclaim_objects

### NAME

    reclaim_objects - reclaim any lingering objects and unused array capacity

### SYNOPSIS

    int reclaim_objects( void );

### DESCRIPTION

    Cycles  through  all  objects  that are loaded, and frees any lingering
    objects that it can.  This could result in a sizable amount  of  memory
    being  freed  up, depending on how the mud is coded.  Objects are typi‐
    cally left lingering when a global variable in  more  than  one  object
    contains  a  pointer  to it, and the object gets destructed.  This efun
    returns the number of destructed objects encountered in variables.

    The same pass also hands back unused array capacity.  pop_array() and
    shift_array() shorten an array without releasing its element storage, so
    that removing an element is cheap and a repeated push/pop cycle does not
    churn the allocator; the storage an array no longer needs is returned
    here.  An array that grew large and was then drained therefore keeps its
    footprint until this runs.

    The driver runs this pass by itself every 30 to 60 seconds, so calling it
    is only useful when the memory is wanted back sooner than that.

    Only arrays reachable from an object's variables are considered -- that
    includes arrays nested inside other arrays, mappings and classes held in
    a variable, but not one that exists solely as a local variable.

### RETURN VALUE

    The number of destructed objects found in variables.  Array capacity
    returned by the pass is not counted; use memory_info() to observe it.

### SEE ALSO

    destruct(3), memory_info(3), pop_array(3), shift_array(3)

