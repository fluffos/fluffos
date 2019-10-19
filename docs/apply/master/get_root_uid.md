---
layout: default
title: master / get_root_uid
---

### NAME

    get_root_uid - get the root uid

### SYNOPSIS

    string get_root_uid( void );

### DESCRIPTION

    This master apply is called by the driver each time it loads the master
    object, to verify that the master object has loaded,  and  to  get  the
    root  uid  defined by the mud.  The function should return a string, eg
    "ROOT"

### SEE ALSO

    get_bb_uid(4)

