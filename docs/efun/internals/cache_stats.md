---
layout: default
title: internals / cache_stats
---

### NAME

    cache_stats() - report various driver and mudlib statistics

### SYNOPSIS

    void cache_stats( void );

### DESCRIPTION

    This  efun  is only available if CACHE_STATS is defined in options.h at
    driver build time.  This efun  dumps  statistics  on  the  call_other()
    cache hit rate to the caller's screen.

### SEE ALSO

    opcprof(3), mud_status(3)

