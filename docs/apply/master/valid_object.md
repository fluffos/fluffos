---
layout: default
title: master / valid_object
---

### NAME

    valid_object - allows control over which objects can be loaded

### SYNOPSIS

    int valid_object( object obj );

### DESCRIPTION

    After  loading  an object, the driver will call valid_object() with the
    newly created object as its argument, in the  master  object.   If  the
    function  exists, and returns 0, then the object will be destructed and
    the efun that caused it to load will error out.  If it does not  exist,
    or returns 1, then loading will proceed as normal.  In conjunction with
    'nomask' and the inherits() efun, this could allow  safely  restricting
    destruct() via a simul_efun.

### SEE ALSO

    valid_override(4)

