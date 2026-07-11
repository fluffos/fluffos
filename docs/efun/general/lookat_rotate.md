---
title: general / lookat_rotate
---
# lookat_rotate

### NAME

    lookat_rotate() - build a viewing-rotation matrix aimed at a point

### SYNOPSIS

    float *lookat_rotate(float *matrix, float look_x, float look_y,
                         float look_z)

### DESCRIPTION

    Replaces the 4x4 transformation `matrix` (a 16-element flat array of
    floats, as produced by id_matrix()) with a viewing-rotation matrix
    that orients toward the look point (`look_x`, `look_y`, `look_z`).

    Unlike translate()/scale()/rotate_*(), this efun does NOT
    post-multiply; it derives a fresh matrix from the current one:

    - The eye position is taken from the current matrix's translation
      column (elements 12, 13 and 14).
    - The current matrix's first column (elements 0, 4 and 8) is used as
      the up reference.
    - The forward, right and up axes are recomputed and normalized to
      look from the eye toward the look point.

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the computed matrix, and that same array is left on
    the stack as the return value. lookat_rotate() mutates its first
    argument; the return value is the very array you passed in, not a copy.

    Because the eye and up reference come from the matrix itself, populate
    the matrix with the eye position (for example via id_matrix() then
    translate()) before calling this efun. Use lookat_rotate2() to supply
    both the eye point and the look point explicitly instead.

### SEE ALSO

    lookat_rotate2(3), id_matrix(3)
