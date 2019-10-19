---
layout: default
title: internals / mud_status
---

### NAME

    mud_status() - report various driver and mudlib statistics

### SYNOPSIS

    void mud_status( int extra );

### DESCRIPTION

    This  function  writes  driver  and  mudlib  statistics to the caller's
    screen.  If extra is non-zero,  then  additional  information  will  be
    written.   This  function  replaces  the hardcoded 'status' and 'status
    tables' commands in vanilla 3.1.2.

### SEE ALSO

    debug_info(3), dumpallobj(3), memory_info(3), uptime(3)

