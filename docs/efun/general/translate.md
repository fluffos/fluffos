---
title: general / translate
---
# translate

### NAME

    translate() - apply a translation to a transformation matrix

### SYNOPSIS

    float *translate(float *matrix, float x, float y, float z)

### DESCRIPTION

    Post-multiplies the 4x4 transformation `matrix` (a 16-element flat
    array of floats, as produced by id_matrix()) by a translation of
    (`x`, `y`, `z`), computing `matrix` = `matrix` * translation.

    The passed `matrix` array is modified IN PLACE: its 16 elements are
    overwritten with the result, and that same array is left on the stack
    as the return value. In other words, translate() mutates its first
    argument; the return value is the very array you passed in, not a copy.

### EXAMPLE

    float *m = id_matrix();
    translate(m, 10.0, 0.0, 5.0);
    // m is now the translation matrix for (10, 0, 5); m[12] == 10.0,
    // m[13] == 0.0, m[14] == 5.0

### SEE ALSO

    id_matrix(3), scale(3), rotate_x(3)
