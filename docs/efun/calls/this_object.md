---
layout: default
title: calls / this_object
---

### NAME

    this_object() - return the object pointer of the calling object

### SYNOPSIS

    object this_object( void );

### DESCRIPTION

    Return the object pointer of this object.  This function is useful when
    an object wants to call functions that are not in the same source  file
    but are in the same object (via inheritance).

### SEE ALSO

    this_player(3), previous_object(3), origin(3)

