---
layout: default
title: internals / dumpallobj
---

### NAME

    dumpallobj()  - report various statistics on all objects that have been
    loaded

### SYNOPSIS

    void dumpallobj( string | void );

### DESCRIPTION

    This function dumps a list of statistics on all objects that have  been
    loaded.   If  no  argument  is  specified, then the information will be
    dumped to a file named /OBJ_DUMP.  If an argument  is  specified,  then
    that name is used as the filename for the dump.

### SEE ALSO

    mud_status(3), debug_info(3)

