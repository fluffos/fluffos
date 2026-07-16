---
title: internals / refs
---
# refs

### NAME

    refs - return the number of references to a data structure

### SYNOPSIS

    int refs( mixed data );

### DESCRIPTION

    The number of references to 'data' will be returned by refs().  This is
    useful for deciding whether or not to make a copy of a  data  structure
    before returning it.

### NOTES

    A structure held by a single variable reports 1; a higher count you
    cannot account for often means the structure references itself -- see
    has_cycle(3) and the "Reference Loops" concepts page.

### SEE ALSO

    children(3), inherit_list(3), deep_inherit_list(3), objects(3),
    has_cycle(3), find_cycles(3), break_cycles(3), find_orphaned_cycles(3)

