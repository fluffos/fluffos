---
title: general / angle
---
# angle

### NAME

    angle() - the angle between two vectors, in radians

### SYNOPSIS

    float angle(int * | float * vector_a, int * | float * vector_b)

### DESCRIPTION

    Returns the angle between 'vector_a' and 'vector_b', expressed in radians.
    It is computed as acos(dotprod(a, b) / (norm(a) * norm(b))). The result is
    always a float.

    Both vectors must have the same size; otherwise a runtime error is raised
    ("angle: cannot calculate the angle between vectors of different sizes.").
    Each element may independently be an int or a float; an element of any
    other type raises a runtime error.

### EXAMPLE

    angle(({ 1, 0 }), ({ 0, 1 })); // 1.5707963...  (pi/2, a right angle)
    angle(({ 1, 0 }), ({ 1, 0 })); // 0.0

### SEE ALSO

    norm(3), dotprod(3), distance(3)
