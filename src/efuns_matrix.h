/*
 *  matrix.h -- definitions and prototypes for matrix.c
 *              2-93 : Dwayne Fontenot : original coding.
 */

#ifndef _MATRIX_H_
#define _MATRIX_H_

#define RADIANS_PER_DEGREE 0.01745329252

typedef float Matrix[16];

typedef struct {
    float x;
    float y;
    float z;
}      Vector;

void translate_matrix PROT((double x, double y, double z, Matrix m));

void scale_matrix PROT((double x, double y, double z, Matrix m));

void rotate_x_matrix PROT((double a, Matrix m));

void rotate_y_matrix PROT((double a, Matrix m));

void rotate_z_matrix PROT((double a, Matrix m));

void lookat_rotate PROT((Matrix T, double x, double y, double z, Matrix m));

void lookat_rotate2
     PROT((double a, double b, double c, double d, double e, double f, Matrix m));

void mult_matrix PROT((Matrix m1, Matrix m2, Matrix m));

#endif				/* _MATRIX_H_ */
