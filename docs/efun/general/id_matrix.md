---
title: general / id_matrix
---
# id_matrix

### NAME

    id_matrix() - create a 4x4 identity transformation matrix

### SYNOPSIS

    float *id_matrix()

### DESCRIPTION

    Returns a new 16-element array of floats holding the 4x4 identity
    matrix, stored in row-major order:

        ({ 1.0, 0.0, 0.0, 0.0,
           0.0, 1.0, 0.0, 0.0,
           0.0, 0.0, 1.0, 0.0,
           0.0, 0.0, 0.0, 1.0 })

    The result is the neutral starting point for the transform efuns:
    translate(), scale(), rotate_x(), rotate_y(), rotate_z(),
    lookat_rotate() and lookat_rotate2() all operate on a matrix in this
    16-float flat-array form. The translation component lives in elements
    12, 13 and 14.

### EXAMPLE

    float *m = id_matrix();
    translate(m, 10.0, 0.0, 5.0);
    // m now holds a translation-by-(10,0,5) matrix; m[12] == 10.0,
    // m[14] == 5.0

### SEE ALSO

    translate(3), scale(3), rotate_x(3)
