---
title: system / reclaim_objects
---
# reclaim_objects

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

    The driver also runs this walk on its own every 30 to 60 seconds. By
    default that automatic walk covers every object in one go, which holds
    the game for as long as the walk takes. The 'reclaim slice budget'
    runtime option (see the driver config) spreads the automatic walk over
    several turns of the event loop instead. A call to reclaim_objects()
    always walks every object at once.

### SEE ALSO

    destruct(3)

