---
layout: default
title: internals / refs
---

### NAME

    refs - return the number of references to a data structure

### SYNOPSIS

    int refs( mixed data );

### DESCRIPTION

    The number of references to 'data' will be returned by refs().  This is
    useful for deciding whether or not to make a copy of a  data  structure
    before returning it.

### SEE ALSO

    children(3), inherit_list(3), deep_inherit_list(3), objects(3)

