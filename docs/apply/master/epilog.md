---
layout: default
title: master / epilog
---

### NAME

    epilog  -  returns  an array of the filenames of the objects to be pre‐
    loaded.

### SYNOPSIS

    string *epilog( int load_empty );
    void preload( string filename );

### DESCRIPTION

    The driver calls epilog() in master after the master  object  has  been
    loaded.   Mudlibs typically use epilog to initialize data structures in
    master (such as security tables etc).  epilog() should returns an array
    of  filenames which correspond to objects that the mudlib wants to have
    preloaded; that is, loaded before the first player logs in.   For  each
    filename  returned  in  the array, the driver will called preload(file‐
    name) in master.

    The variable 'load_empty' is non-zero if the -e  option  was  specified
    when  starting up the driver.  It can be used as a signal to the mudlib
    to not load castles, etc.

### SEE ALSO

    preload(4)

