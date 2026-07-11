---
title: general / rotate_z
---
# rotate_z

### NAME

    rotate_z() - rotate a transformation matrix about the Z axis

### SYNOPSIS

    float *rotate_z(float *matrix, float degrees)

### DESCRIPTION

    Post-multiplies the 4x4 transformation `matrix` (a 16-element flat
    array of floats, as produced by id_matrix()) by a rotation of
    `degrees` about the Z axis, computing `matrix` = `matrix` * rotation.

    The angle is given in DEGREES, not radians (it is converted to radians
    internally).

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the result, and that same array is left on the stack
    as the return value. rotate_z() mutates its first argument; the return
    value is the very array you passed in, not a copy.

### SEE ALSO

    rotate_x(3), rotate_y(3), id_matrix(3)
