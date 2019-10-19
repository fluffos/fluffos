---
layout: default
title: internals / swap
---

### NAME

    swap - swap out a file explicitly

### SYNOPSIS

    void swap( object );

### DESCRIPTION

    This  efun  should be reserved for debugging only.  It allows an object
    to be explicitly swapped out.  If enabled, it is  strongly  recommended
    that a simul_efun override (for this efun) be used to prevent abuse.

    Note:  objects which have been destructed, already swapped out, contain
    a heart beat, cloned, inherited, or interactive, cannot be swapped out.

