/*
 *  matrix.c -- matrix efuns.
 *              2-93 : Dwayne Fontenot : original coding.
 */

#include <math.h>

#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#include "../efun_protos.h"
#endif

#include "matrix.h"

static Matrix identity =
{1., 0., 0., 0.,
 0., 1., 0., 0.,
 0., 0., 1., 0.,
 0., 0., 0., 1.};

static void print_matrix (Matrix, char *);
static void print_array (Vector *, char *);
static Vector *normalize_array (Vector *);
static Vector *cross_product (Vector *, Vector *, Vector *);
static Vector *points_to_array (Vector *, Vector *, Vector *);

void f_id_matrix (void)
{
    array_t *matrix;
    int i;

    matrix = allocate_empty_array(16);
    for (i = 0; i < 16; i++) {
        matrix->item[i].type = T_REAL;
        matrix->item[i].u.real = identity[i];
    }
    push_refed_array(matrix);
}

void f_translate (void)
{
    array_t *matrix;
    double x, y, z;
    Matrix current_matrix;
    Matrix trans_matrix;
    Matrix final_matrix;
    int i;

    if ((sp - 1)->type != T_REAL) {
	bad_arg(3, F_TRANSLATE);
    }
    if (sp->type != T_REAL) {
	bad_arg(4, F_TRANSLATE);
    }
    /*
     * get arguments from stack.
     */
    matrix = (sp - 3)->u.arr;
    x = (sp - 2)->u.real;
    y = (sp - 1)->u.real;
    z = sp->u.real;
    sp -= 3;

    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create translation matrix.
     */
    translate_matrix(x, y, z, trans_matrix);
    /*
     * compute transformed matrix.
     */
    mult_matrix(current_matrix, trans_matrix, final_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = final_matrix[i];
    }
}

void f_scale (void)
{
    array_t *matrix;
    double x, y, z;
    Matrix current_matrix;
    Matrix scaling_matrix;
    Matrix final_matrix;
    int i;

    if ((sp - 1)->type != T_REAL) {
	bad_arg(3, F_SCALE);
    }
    if (sp->type != T_REAL) {
	bad_arg(4, F_SCALE);
    }
    /*
     * get arguments from stack.
     */
    matrix = (sp - 3)->u.arr;
    x = (sp - 2)->u.real;
    y = (sp - 1)->u.real;
    z = sp->u.real;
    sp -= 3;
    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create scaling matrix.
     */
    scale_matrix(x, y, z, scaling_matrix);
    /*
     * compute transformed matrix.
     */
    mult_matrix(current_matrix, scaling_matrix, final_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = final_matrix[i];
    }
}

void f_rotate_x (void)
{
    array_t *matrix;
    double angle;
    Matrix current_matrix;
    Matrix rot_matrix;
    Matrix final_matrix;
    int i;

    /*
     * get arguments from stack.
     */
    matrix = (sp - 1)->u.arr;
    angle = (sp--)->u.real;
    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create x rotation matrix.
     */
    rotate_x_matrix(angle, rot_matrix);
    /*
     * compute transformed matrix.
     */
    mult_matrix(current_matrix, rot_matrix, final_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = final_matrix[i];
    }
}

void f_rotate_y (void)
{
    array_t *matrix;
    double angle;
    Matrix current_matrix;
    Matrix rot_matrix;
    Matrix final_matrix;
    int i;

    /*
     * get arguments from stack.
     */
    matrix = (sp - 1)->u.arr;
    angle = (sp--)->u.real;
    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create y rotation matrix.
     */
    rotate_y_matrix(angle, rot_matrix);
    /*
     * compute transformed matrix.
     */
    mult_matrix(current_matrix, rot_matrix, final_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = final_matrix[i];
    }
}

void f_rotate_z (void)
{
    array_t *matrix;
    double angle;
    Matrix current_matrix;
    Matrix rot_matrix;
    Matrix final_matrix;
    int i;

    /*
     * get arguments from stack.
     */
    matrix = (sp - 1)->u.arr;
    angle = (sp--)->u.real;
    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create z rotation matrix.
     */
    rotate_z_matrix(angle, rot_matrix);
    /*
     * compute transformed matrix.
     */
    mult_matrix(current_matrix, rot_matrix, final_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = final_matrix[i];
    }
}

void f_lookat_rotate (void)
{
    array_t *matrix;
    double x, y, z;
    Matrix current_matrix;
    Matrix lookat_matrix;
    int i;

    if ((sp - 1)->type != T_REAL) {
	bad_arg(3, F_LOOKAT_ROTATE);
    }
    if (sp->type != T_REAL) {
	bad_arg(4, F_LOOKAT_ROTATE);
    }
    /*
     * get arguments from stack.
     */
    matrix = (sp - 3)->u.arr;
    x = (sp - 2)->u.real;
    y = (sp - 1)->u.real;
    z = sp->u.real;
    sp -= 3;
    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create new viewing transformation matrix.
     */
    lookat_rotate(current_matrix, x, y, z, lookat_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = lookat_matrix[i];
    }
}

#ifdef F_LOOKAT_ROTATE2
void f_lookat_rotate2 (void)
{
    array_t *matrix;
    double ex, ey, ez, lx, ly, lz;
    Matrix current_matrix;
    Matrix lookat_matrix;
    int i, j;

    for (j = 4; j >= 0; j--) {
	if ((sp - j)->type != T_REAL) {
	    bad_arg(7 - j, F_LOOKAT_ROTATE2);
	}
    }
    /*
     * get arguments from stack.
     */
    matrix = (sp - 6)->u.arr;
    ex = (sp - 5)->u.real;
    ey = (sp - 4)->u.real;
    ez = (sp - 3)->u.real;
    lx = (sp - 2)->u.real;
    ly = (sp - 1)->u.real;
    lz = sp->u.real;
    sp -= 5;
    free_array((sp--)->u.arr);

    /*
     * convert vec matrix to float matrix.
     */
    for (i = 0; i < 16; i++) {
	current_matrix[i] = matrix->item[i].u.real;
    }
    /*
     * create new viewing transformation matrix.
     */
    lookat_rotate2(ex, ey, ez, lx, ly, lz, lookat_matrix);
    /*
     * convert float matrix to vec matrix.
     */
    for (i = 0; i < 16; i++) {
	matrix->item[i].u.real = lookat_matrix[i];
    }
}
#endif

#ifdef DEBUG
static void print_matrix (Matrix m, char * label)
{
    int i;
    int j;

    fprintf(stderr, "%s:\n", label);
    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    fprintf(stderr, "%f\t", m[i * 4 + j]);
	}
	fprintf(stderr, "\n");
    }
}

static void print_array (Vector * v, char * label)
{
    fprintf(stderr, "%s:\t%f\t%f\t%f\n", label, v->x, v->y, v->z);
}
#endif

static Vector *normalize_array (Vector * v)
{
    double xx, yy, zz, mm, m;

    xx = v->x * v->x;
    yy = v->y * v->y;
    zz = v->z * v->z;
    mm = xx + yy + zz;
    m = sqrt(mm);
    if (m) {
	v->x /= m;
	v->y /= m;
	v->z /= m;
    }
    return (v);
}

static Vector *cross_product (Vector * v, Vector * va, Vector * vb)
{
    v->x = (va->y * vb->z) - (va->z * vb->y);
    v->y = (va->z * vb->x) - (va->x * vb->z);
    v->z = (va->x * vb->y) - (va->y * vb->x);
    return (v);
}

static Vector *points_to_array (Vector * v, Vector * pa, Vector * pb)
{
    v->x = pa->x - pb->x;
    v->y = pa->y - pb->y;
    v->z = pa->z - pb->z;
    return (v);
}

void lookat_rotate (Matrix T, double x, double y, double z, Matrix M)
{
    static Vector N, V, U;
    static Vector ep, lp;

    lp.x = x;
    lp.y = y;
    lp.z = z;
    ep.x = T[12];
    ep.y = T[13];
    ep.z = T[14];
    points_to_array(&N, &lp, &ep);
    normalize_array(&N);

    U.x = T[0];
    U.y = T[4];
    U.z = T[8];
    cross_product(&V, &N, &U);
    normalize_array(&V);

    cross_product(&U, &V, &N);
    normalize_array(&U);

    M[0] = U.x;
    M[1] = V.x;
    M[2] = N.x;
    M[3] = 0.;
    M[4] = U.y;
    M[5] = V.y;
    M[6] = N.y;
    M[7] = 0.;
    M[8] = U.z;
    M[9] = V.z;
    M[10] = N.z;
    M[11] = 0.;
#if 0
    M[12] = ep.x;
    M[13] = ep.y;
    M[14] = ep.z;
    M[15] = 1.;
#endif
#if 0
    M[12] = -U.x * ep.x - U.y * ep.y - U.z * ep.z;
    M[13] = -V.x * ep.x - V.y * ep.y - V.z * ep.z;
    M[14] = -N.x * ep.x - N.y * ep.y - N.z * ep.z;
#endif
    M[12] = ((U.x * ep.x) + (U.y * ep.y) + (U.z * ep.z));
    M[13] = ((V.x * ep.x) + (V.y * ep.y) + (V.z * ep.z));
    M[14] = ((N.x * ep.x) + (N.y * ep.y) + (N.z * ep.z));
    M[15] = 1.;

#ifdef DEBUG
    print_array(&lp, "look point");
    print_array(&ep, "eye point");
    print_array(&N, "normal array");
    print_array(&V, "V = N x U");
    print_array(&U, "U = V x N");
    print_matrix(M, "final matrix");
#endif				/* DEBUG */
}

void lookat_rotate2 (double ex, double ey, double ez, double lx, double ly, double lz, Matrix M)
{
    static Vector N, V, U;
    static Vector ep, lp;

    ep.x = ex;
    ep.y = ey;
    ep.z = ez;
    lp.x = lx;
    lp.y = ly;
    lp.z = lz;
    points_to_array(&N, &lp, &ep);
    normalize_array(&N);

    U.x = 0.;
    U.y = 1.;
    U.z = 0.;
    cross_product(&V, &N, &U);
    normalize_array(&V);

    cross_product(&U, &V, &N);
    normalize_array(&U);

    M[0] = U.x;
    M[1] = V.x;
    M[2] = N.x;
    M[3] = 0.;
    M[4] = U.y;
    M[5] = V.y;
    M[6] = N.y;
    M[7] = 0.;
    M[8] = U.z;
    M[9] = V.z;
    M[10] = N.z;
    M[11] = 0.;
#if 0
    M[12] = ep.x;
    M[13] = ep.y;
    M[14] = ep.z;
    M[15] = 1.;
#endif
#if 0
    M[12] = -U.x * ep.x - U.y * ep.y - U.z * ep.z;
    M[13] = -V.x * ep.x - V.y * ep.y - V.z * ep.z;
    M[14] = -N.x * ep.x - N.y * ep.y - N.z * ep.z;
#endif
    M[12] = ((U.x * ep.x) + (U.y * ep.y) + (U.z * ep.z));
    M[13] = ((V.x * ep.x) + (V.y * ep.y) + (V.z * ep.z));
    M[14] = ((N.x * ep.x) + (N.y * ep.y) + (N.z * ep.z));
    M[15] = 1.;

#ifdef DEBUG
    print_array(&lp, "look point");
    print_array(&ep, "eye point");
    print_array(&N, "normal array");
    print_array(&V, "V = N x U");
    print_array(&U, "U = V x N");
    print_matrix(M, "final matrix");
#endif				/* DEBUG */
}

void translate_matrix (double x, double y, double z, Matrix m)
{
    m[0] = 1.;
    m[1] = 0.;
    m[2] = 0.;
    m[3] = 0.;
    m[4] = 0.;
    m[5] = 1.;
    m[6] = 0.;
    m[7] = 0.;
    m[8] = 0.;
    m[9] = 0.;
    m[10] = 1.;
    m[11] = 0.;
    m[12] = x;
    m[13] = y;
    m[14] = z;
    m[15] = 1.;
}

void scale_matrix (double x, double y, double z, Matrix m)
{
    m[0] = x;
    m[1] = 0.;
    m[2] = 0.;
    m[3] = 0.;
    m[4] = 0.;
    m[5] = y;
    m[6] = 0.;
    m[7] = 0.;
    m[8] = 0.;
    m[9] = 0.;
    m[10] = z;
    m[11] = 0.;
    m[12] = 0.;
    m[13] = 0.;
    m[14] = 0.;
    m[15] = 1.;
}

void rotate_x_matrix (double a, Matrix m)
{
    double a_rad;
    double c, s;

    a_rad = (double) (a * RADIANS_PER_DEGREE);
    c = cos(a_rad);
    s = sin(a_rad);
    m[0] = 1.;
    m[1] = 0.;
    m[2] = 0.;
    m[3] = 0.;
    m[4] = 0.;
    m[5] = c;
    m[6] = s;
    m[7] = 0.;
    m[8] = 0.;
    m[9] = -s;
    m[10] = c;
    m[11] = 0.;
    m[12] = 0.;
    m[13] = 0.;
    m[14] = 0.;
    m[15] = 1.;
}

void rotate_y_matrix (double a, Matrix m)
{
    double a_rad;
    double c, s;

    a_rad = (double) (a * RADIANS_PER_DEGREE);
    c = cos(a_rad);
    s = sin(a_rad);
    m[0] = c;
    m[1] = 0.;
    m[2] = -s;
    m[3] = 0.;
    m[4] = 0.;
    m[5] = 1.;
    m[6] = 0.;
    m[7] = 0.;
    m[8] = s;
    m[9] = 0.;
    m[10] = c;
    m[11] = 0.;
    m[12] = 0.;
    m[13] = 0.;
    m[14] = 0.;
    m[15] = 1.;
}

void rotate_z_matrix (double a, Matrix m)
{
    double a_rad;
    double c, s;

    a_rad = (double) (a * RADIANS_PER_DEGREE);
    c = cos(a_rad);
    s = sin(a_rad);
    m[0] = c;
    m[1] = s;
    m[2] = 0.;
    m[3] = 0.;
    m[4] = -s;
    m[5] = c;
    m[6] = 0.;
    m[7] = 0.;
    m[8] = 0.;
    m[9] = 0.;
    m[10] = 1.;
    m[11] = 0.;
    m[12] = 0.;
    m[13] = 0.;
    m[14] = 0.;
    m[15] = 1.;
}

void mult_matrix (Matrix ma, Matrix mb, Matrix m)
{
    m[0] = ma[0] * mb[0] + ma[1] * mb[4] + ma[2] * mb[8] + ma[3] * mb[12];

    m[1] = ma[0] * mb[1] + ma[1] * mb[5] + ma[2] * mb[9] + ma[3] * mb[13];

    m[2] = ma[0] * mb[2] + ma[1] * mb[6] + ma[2] * mb[10] + ma[3] * mb[14];

    m[3] = ma[0] * mb[3] + ma[1] * mb[7] + ma[2] * mb[11] + ma[3] * mb[15];

    m[4] = ma[4] * mb[0] + ma[5] * mb[4] + ma[6] * mb[8] + ma[7] * mb[12];

    m[5] = ma[4] * mb[1] + ma[5] * mb[5] + ma[6] * mb[9] + ma[7] * mb[13];

    m[6] = ma[4] * mb[2] + ma[5] * mb[6] + ma[6] * mb[10] + ma[7] * mb[14];

    m[7] = ma[4] * mb[3] + ma[5] * mb[7] + ma[6] * mb[11] + ma[7] * mb[15];

    m[8] = ma[8] * mb[0] + ma[9] * mb[4] + ma[10] * mb[8] + ma[11] * mb[12];

    m[9] = ma[8] * mb[1] + ma[9] * mb[5] + ma[10] * mb[9] + ma[11] * mb[13];

    m[10] = ma[8] * mb[2] + ma[9] * mb[6] + ma[10] * mb[10] + ma[11] * mb[14];

    m[11] = ma[8] * mb[3] + ma[9] * mb[7] + ma[10] * mb[11] + ma[11] * mb[15];

    m[12] = ma[12] * mb[0] + ma[13] * mb[4] + ma[14] * mb[8] + ma[15] * mb[12];

    m[13] = ma[12] * mb[1] + ma[13] * mb[5] + ma[14] * mb[9] + ma[15] * mb[13];

    m[14] = ma[12] * mb[2] + ma[13] * mb[6] + ma[14] * mb[10] + ma[15] * mb[14];

    m[15] = ma[12] * mb[3] + ma[13] * mb[7] + ma[14] * mb[11] + ma[15] * mb[15];
}
