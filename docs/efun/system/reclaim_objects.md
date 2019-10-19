---
layout: default
title: system / reclaim_objects
---

### NAME

    reclaim_objects - reclaim any lingering objets

### SYNOPSIS

    int reclaim_objects( void );

### DESCRIPTION

    Cycles  through  all  objects  that are loaded, and frees any lingering
    objects that it can.  This could result in a sizable amount  of  memory
    being  freed  up, depending on how the mud is coded.  Objects are typi‐
    cally left lingering when a global variable in  more  than  one  object
    contains  a  pointer  to it, and the object gets destructed.  This efun
    returns the number of destructed objects encountered in variables.

### SEE ALSO

    destruct(3)

