---
layout: default
title: system / error
---

### NAME

    error - generate a run-time error

### SYNOPSIS

    void error( string err );

### DESCRIPTION

    A  run-time error 'err' will be generated when error() is called.  Exe‐
    cution of the current thread will halt, and the trace will be  recorded
    to  the  debug log.  counting an optional '0 as last character.  Before
    processing if the error the optional '0 will be removed, a '*' will  be
    prepended unconditional an '0 gets appended.

### SEE ALSO

    catch(3), throw(3), error_handler(4)
