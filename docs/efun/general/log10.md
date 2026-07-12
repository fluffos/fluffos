---
title: general / log10
---
# log10

### NAME

    log10() - the base-10 logarithm of a number

### SYNOPSIS

    float log10(float | int x)

### DESCRIPTION

    Returns the base-10 logarithm of 'x'. The argument may be an int or a
    float; an int is promoted to float before the computation. The result is
    always a float.

    'x' must be strictly positive. Calling log10() with 'x' <= 0.0 raises a
    runtime error ("log10(x) with (x <= 0.0)").

### SEE ALSO

    log(3), log2(3), pow(3), exp(3)
