---
layout: default
title: internals / trace
---

### NAME

    trace() - sets trace flags and returns the old ones.

### SYNOPSIS

    int trace( int traceflags );

### DESCRIPTION

    Sets  the trace flags and returns the old trace flags.  When tracing is
    on a lot of information is printed during execution.

    The trace bits are:

    Trace all function calls to lfuns.

    Trace all calls to "call_other".

    Trace all function returns.

    Print arguments at function calls and return values.

    Print all executed stack machine instructions (produces a lot  of  out‐
    put!).

    Enable trace in heart beat functions.

    Trace calls to apply.

    Show object name in tracing.

### SEE ALSO

    traceprefix(3)
