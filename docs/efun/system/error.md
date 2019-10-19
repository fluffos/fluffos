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
    to  the debug log.  'err' itself is allowed to have a length of maximal
    2045 characters not  counting  an  optional  '\n'  as  last  character.
    Before processing if the error the optional '\n' will be removed, a '*'
    will be prepended unconditional an '\n' gets appended.

### SEE ALSO

    catch(3), throw(3), error_handler(4)

