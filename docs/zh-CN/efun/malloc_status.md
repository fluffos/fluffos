---
layout: default
title: internals / malloc_status
---

### NAME

    malloc_status() - report various statistics related to memory usage.

### SYNOPSIS

    void malloc_status( void );

### DESCRIPTION

    This  function  writes  memory usage statistics to the caller's screen.
    This function replaces the hardcoded 'malloc' command in vanilla 3.1.2.
    Note  that  the  output  produced by malloc_status() depends upon which
    memory management package is chosen  in  options.h  when  building  the
    driver.

### SEE ALSO

    mud_status(3), dumpallobj(3), memory_info(3)

