---
title: general / lookat_rotate2
---
# lookat_rotate2

### NAME

    lookat_rotate2() - build a viewing-rotation matrix from explicit eye
    and look points

### SYNOPSIS

    float *lookat_rotate2(float *matrix, float eye_x, float eye_y,
                          float eye_z, float look_x, float look_y,
                          float look_z)

### DESCRIPTION

    Replaces the 4x4 transformation `matrix` (a 16-element flat array of
    floats, as produced by id_matrix()) with a viewing-rotation matrix
    that looks from the eye point (`eye_x`, `eye_y`, `eye_z`) toward the
    look point (`look_x`, `look_y`, `look_z`).

    This is like lookat_rotate(), except both the eye point and the look
    point are given explicitly as arguments rather than being read from
    the current matrix. The world up vector is fixed at (0, 1, 0); the
    prior contents of `matrix` are ignored and fully overwritten.

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the computed matrix, and that same array is left on
    the stack as the return value. lookat_rotate2() mutates its first
    argument; the return value is the very array you passed in, not a copy.

### SEE ALSO

    lookat_rotate(3), id_matrix(3)
