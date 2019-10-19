---
layout: default
title: internals / opcprof
---

### NAME

    opcprof() - reports statistics on calling frequencies of various efuns

### SYNOPSIS

    void opcprof( string | void );

### DESCRIPTION

    This  function dumps a list of statistics on each efunction and eopera‐
    tor.  If no argument is specified, then the information will be  dumped
    to  files  named  /OPCPROF.efun  and /OPCPROF.eoper.  If an argument is
    specified, then that name is used as the filename for the dump.

### SEE ALSO

    function_profile(3)

