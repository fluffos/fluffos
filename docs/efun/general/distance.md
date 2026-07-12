---
title: general / distance
---
# distance

### NAME

    distance() - the Euclidean distance between two points

### SYNOPSIS

    float distance(int * | float * point_a, int * | float * point_b)

### DESCRIPTION

    Returns the Euclidean distance between the points 'point_a' and 'point_b':
    the square root of the sum of the squares of the differences of their
    corresponding coordinates. The result is always a float.

    Both points must have the same dimension (array size); otherwise a runtime
    error is raised ("distance: cannot take the distance of vectors of
    different sizes."). Each coordinate may independently be an int or a float;
    a coordinate of any other type raises a runtime error.

### EXAMPLE

    distance(({ 0, 0 }), ({ 3, 4 }));       // 5.0
    distance(({ 1.0, 1.0 }), ({ 4.0, 5.0 })); // 5.0

### SEE ALSO

    norm(3), dotprod(3), angle(3)
