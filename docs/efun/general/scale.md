---
title: general / scale
---
# scale

### NAME

    scale() - apply a scaling to a transformation matrix

### SYNOPSIS

    float *scale(float *matrix, float x, float y, float z)

### DESCRIPTION

    Post-multiplies the 4x4 transformation `matrix` (a 16-element flat
    array of floats, as produced by id_matrix()) by a scaling of
    (`x`, `y`, `z`), computing `matrix` = `matrix` * scaling.

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the result, and that same array is left on the stack
    as the return value. scale() mutates its first argument; the return
    value is the very array you passed in, not a copy.

### SEE ALSO

    id_matrix(3), translate(3), rotate_x(3)
