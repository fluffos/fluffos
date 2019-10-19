---
layout: default
title: master / get_bb_uid
---

### NAME

    get_bb_uid - get the backbone uid

### SYNOPSIS

    string get_bb_uid( void );

### DESCRIPTION

    This  master  apply  is  called  by the driver on startup, after it has
    loaded the master object, to get the backbone uid defined by  the  mud.
    The function should return a string, eg "BACKBONE"

### SEE ALSO

    get_root_uid(4)

