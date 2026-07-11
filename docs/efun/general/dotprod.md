---
title: general / dotprod
---
# dotprod

### NAME

    dotprod() - the dot product of two vectors

### SYNOPSIS

    float dotprod(int * | float * vector_a, int * | float * vector_b)

### DESCRIPTION

    Returns the dot product of 'vector_a' and 'vector_b': the sum of the
    products of their corresponding elements. The result is always a float.

    Both vectors must have the same size; otherwise a runtime error is raised
    ("dotprod: cannot take the dotprod of vectors of different sizes."). Each
    element of either vector may independently be an int or a float; an element
    of any other type raises a runtime error.

### EXAMPLE

    dotprod(({ 1, 2, 3 }), ({ 4, 5, 6 })); // 32.0  (1*4 + 2*5 + 3*6)

### SEE ALSO

    norm(3), distance(3), angle(3)
