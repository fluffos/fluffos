---
title: general / log2
---
# log2

### NAME

    log2() - the base-2 logarithm of a number

### SYNOPSIS

    float log2(float | int x)

### DESCRIPTION

    Returns the base-2 logarithm of 'x'. The argument may be an int or a
    float; an int is promoted to float before the computation. The result is
    always a float.

    'x' must be strictly positive. Calling log2() with 'x' <= 0.0 raises a
    runtime error ("log2(x) with (x <= 0.0)").

### SEE ALSO

    log(3), log10(3), pow(3), exp(3)
