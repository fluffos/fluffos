/*
 *  matrix.h -- definitions and prototypes for matrix.c
 *              2-93 : Dwayne Fontenot : original coding.
 */

#ifndef _MATRIX_H_
#define _MATRIX_H_

#define RADIANS_PER_DEGREE 0.01745329252

typedef double Matrix[16];

typedef struct {
    double x;
    double y;
    double z;
}      Vector;

void translate_matrix (double x, double y, double z, Matrix m);

void scale_matrix (double x, double y, double z, Matrix m);

void rotate_x_matrix (double a, Matrix m);

void rotate_y_matrix (double a, Matrix m);

void rotate_z_matrix (double a, Matrix m);

void lookat_rotate (Matrix T, double x, double y, double z, Matrix m);

void lookat_rotate2
     (double a, double b, double c, double d, double e, double f, Matrix m);

void mult_matrix (Matrix m1, Matrix m2, Matrix m);

#endif				/* _MATRIX_H_ */
