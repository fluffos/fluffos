---
title: general / norm
---
# norm

### NAME

    norm() - the magnitude (Euclidean length) of a vector

### SYNOPSIS

    float norm(int * | float * vector)

### DESCRIPTION

    Returns the norm (magnitude) of 'vector', that is, the Euclidean length:
    the square root of the sum of the squares of its elements. The result is
    always a float.

    Each element of 'vector' may independently be an int or a float; the two
    may be mixed in the same array. An element of any other type raises a
    runtime error ("norm: invalid argument 1.").

### EXAMPLE

    norm(({ 3, 4 }));       // 5.0
    norm(({ 1.0, 2.0, 2.0 })); // 3.0

### SEE ALSO

    dotprod(3), distance(3), angle(3)
