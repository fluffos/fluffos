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

    1   Trace all function calls to lfuns.

    2   Trace all calls to "call_other".

    4   Trace all function returns.

    8   Print arguments at function calls and return values.

    16  Print all executed stack machine instructions (produces  a  lot  of
        output!).

    32  Enable trace in heart beat functions.

    64  Trace calls to apply.

    128 Show object name in tracing.

### SEE ALSO

    traceprefix(3)

