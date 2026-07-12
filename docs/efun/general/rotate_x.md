---
title: general / rotate_x
---
# rotate_x

### NAME

    rotate_x() - rotate a transformation matrix about the X axis

### SYNOPSIS

    float *rotate_x(float *matrix, float degrees)

### DESCRIPTION

    Post-multiplies the 4x4 transformation `matrix` (a 16-element flat
    array of floats, as produced by id_matrix()) by a rotation of
    `degrees` about the X axis, computing `matrix` = `matrix` * rotation.

    The angle is given in DEGREES, not radians (it is converted to radians
    internally).

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the result, and that same array is left on the stack
    as the return value. rotate_x() mutates its first argument; the return
    value is the very array you passed in, not a copy.

### SEE ALSO

    rotate_y(3), rotate_z(3), id_matrix(3)
