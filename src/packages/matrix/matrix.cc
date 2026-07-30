/*
 *  matrix.c -- matrix efuns.
 *              2-93 : Dwayne Fontenot : original coding.
 */

#include "base/package_api.h"

#include <cmath>

#include "matrix.h"

static Matrix identity = {1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.};

static void print_matrix(Matrix, const char*);
static void print_array(Vector*, const char*);
static Vector* normalize_array(Vector*);
static Vector* cross_product(Vector*, Vector*, Vector*);
static Vector* points_to_array(Vector*, Vector*, Vector*);

// Validate that `arg` holds a 16-element array of floats and copy it into the
// C float matrix `out`. Errors -- without modifying the caller's array -- on a
// wrong size or a non-float element. Returns the array so the transformed
// result can be written back with store_matrix().
static array_t* matrix_arg_to_floats(const svalue_t* arg, Matrix out) {
  array_t* matrix = arg->u.arr;
  if (matrix->size < 16) {
    error("matrix transform requires a 16-element array.\n");
  }
  for (int i = 0; i < 16; i++) {
    if (matrix->item[i].type != T_REAL) {
      error("matrix transform requires a 16-element float array.\n");
    }
    out[i] = matrix->item[i].u.real;
  }
  return matrix;
}

// Write the C float matrix `src` back into the LPC float array `matrix`.
static void store_matrix(array_t* matrix, const Matrix src) {
  for (int i = 0; i < 16; i++) {
    matrix->item[i].u.real = src[i];
  }
}

void f_id_matrix() {
  array_t* matrix;
  int i;

  matrix = allocate_empty_array(16);
  for (i = 0; i < 16; i++) {
    matrix->item[i].type = T_REAL;
    matrix->item[i].u.real = identity[i];
  }
  push_refed_array(matrix);
}

void f_translate() {
  array_t* matrix;
  LPC_FLOAT x, y, z;
  Matrix current_matrix;
  Matrix trans_matrix;
  Matrix final_matrix;

  if ((sp - 1)->type != T_REAL) {
    bad_arg(3, F_TRANSLATE);
  }
  if (sp->type != T_REAL) {
    bad_arg(4, F_TRANSLATE);
  }
  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 3, current_matrix);
  x = (sp - 2)->u.real;
  y = (sp - 1)->u.real;
  z = sp->u.real;
  sp -= 3;

  /*
   * create translation matrix, then compute and store the transformed matrix.
   */
  translate_matrix(x, y, z, trans_matrix);
  mult_matrix(current_matrix, trans_matrix, final_matrix);
  store_matrix(matrix, final_matrix);
}

void f_scale() {
  array_t* matrix;
  LPC_FLOAT x, y, z;
  Matrix current_matrix;
  Matrix scaling_matrix;
  Matrix final_matrix;

  if ((sp - 1)->type != T_REAL) {
    bad_arg(3, F_SCALE);
  }
  if (sp->type != T_REAL) {
    bad_arg(4, F_SCALE);
  }
  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 3, current_matrix);
  x = (sp - 2)->u.real;
  y = (sp - 1)->u.real;
  z = sp->u.real;
  sp -= 3;
  /*
   * create scaling matrix, then compute and store the transformed matrix.
   */
  scale_matrix(x, y, z, scaling_matrix);
  mult_matrix(current_matrix, scaling_matrix, final_matrix);
  store_matrix(matrix, final_matrix);
}

void f_rotate_x() {
  array_t* matrix;
  LPC_FLOAT angle;
  Matrix current_matrix;
  Matrix rot_matrix;
  Matrix final_matrix;

  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 1, current_matrix);
  angle = (sp--)->u.real;
  /*
   * create x rotation matrix, then compute and store the transformed matrix.
   */
  rotate_x_matrix(angle, rot_matrix);
  mult_matrix(current_matrix, rot_matrix, final_matrix);
  store_matrix(matrix, final_matrix);
}

void f_rotate_y() {
  array_t* matrix;
  LPC_FLOAT angle;
  Matrix current_matrix;
  Matrix rot_matrix;
  Matrix final_matrix;

  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 1, current_matrix);
  angle = (sp--)->u.real;
  /*
   * create y rotation matrix, then compute and store the transformed matrix.
   */
  rotate_y_matrix(angle, rot_matrix);
  mult_matrix(current_matrix, rot_matrix, final_matrix);
  store_matrix(matrix, final_matrix);
}

void f_rotate_z() {
  array_t* matrix;
  LPC_FLOAT angle;
  Matrix current_matrix;
  Matrix rot_matrix;
  Matrix final_matrix;

  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 1, current_matrix);
  angle = (sp--)->u.real;
  /*
   * create z rotation matrix, then compute and store the transformed matrix.
   */
  rotate_z_matrix(angle, rot_matrix);
  mult_matrix(current_matrix, rot_matrix, final_matrix);
  store_matrix(matrix, final_matrix);
}

void f_lookat_rotate() {
  array_t* matrix;
  LPC_FLOAT x, y, z;
  Matrix current_matrix;
  Matrix lookat_matrix;

  if ((sp - 1)->type != T_REAL) {
    bad_arg(3, F_LOOKAT_ROTATE);
  }
  if (sp->type != T_REAL) {
    bad_arg(4, F_LOOKAT_ROTATE);
  }
  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 3, current_matrix);
  x = (sp - 2)->u.real;
  y = (sp - 1)->u.real;
  z = sp->u.real;
  sp -= 3;
  /*
   * create new viewing transformation matrix and store it.
   */
  lookat_rotate(current_matrix, x, y, z, lookat_matrix);
  store_matrix(matrix, lookat_matrix);
}

#ifdef F_LOOKAT_ROTATE2
void f_lookat_rotate2(void) {
  array_t* matrix;
  LPC_FLOAT ex, ey, ez, lx, ly, lz;
  Matrix current_matrix;
  Matrix lookat_matrix;
  int j;

  for (j = 4; j >= 0; j--) {
    if ((sp - j)->type != T_REAL) {
      bad_arg(7 - j, F_LOOKAT_ROTATE2);
    }
  }
  /*
   * get arguments from stack.
   */
  matrix = matrix_arg_to_floats(sp - 6, current_matrix);
  ex = (sp - 5)->u.real;
  ey = (sp - 4)->u.real;
  ez = (sp - 3)->u.real;
  lx = (sp - 2)->u.real;
  ly = (sp - 1)->u.real;
  lz = sp->u.real;

  pop_n_elems(6);

  /*
   * create new viewing transformation matrix and store it.
   */
  lookat_rotate2(ex, ey, ez, lx, ly, lz, lookat_matrix);
  store_matrix(matrix, lookat_matrix);
}
#endif

#ifdef DEBUG
static void print_matrix(Matrix m, const char* label) {
  int i;
  int j;

  debug_message("%s:\n", label);
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      debug_message("%f\t", m[i * 4 + j]);
    }
    debug_message("\n");
  }
}

static void print_array(Vector* v, const char* label) {
  debug_message("%s:\t%f\t%f\t%f\n", label, v->x, v->y, v->z);
}
#endif

static Vector* normalize_array(Vector* v) {
  LPC_FLOAT xx, yy, zz, mm, m;

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

static Vector* cross_product(Vector* v, Vector* va, Vector* vb) {
  v->x = (va->y * vb->z) - (va->z * vb->y);
  v->y = (va->z * vb->x) - (va->x * vb->z);
  v->z = (va->x * vb->y) - (va->y * vb->x);
  return (v);
}

static Vector* points_to_array(Vector* v, Vector* pa, Vector* pb) {
  v->x = pa->x - pb->x;
  v->y = pa->y - pb->y;
  v->z = pa->z - pb->z;
  return (v);
}

// Shared body of lookat_rotate()/lookat_rotate2(): build an orientation matrix
// from an eye point `ep`, a look point `lp`, and an initial up hint `u`. The two
// public entry points differ only in how `ep`, `lp`, and the up hint are derived.
static void build_lookat_matrix(Vector ep, Vector lp, Vector u, Matrix M) {
  Vector n, v;

  points_to_array(&n, &lp, &ep);
  normalize_array(&n);

  cross_product(&v, &n, &u);
  normalize_array(&v);

  cross_product(&u, &v, &n);
  normalize_array(&u);

  M[0] = u.x;
  M[1] = v.x;
  M[2] = n.x;
  M[3] = 0.;
  M[4] = u.y;
  M[5] = v.y;
  M[6] = n.y;
  M[7] = 0.;
  M[8] = u.z;
  M[9] = v.z;
  M[10] = n.z;
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
  M[12] = ((u.x * ep.x) + (u.y * ep.y) + (u.z * ep.z));
  M[13] = ((v.x * ep.x) + (v.y * ep.y) + (v.z * ep.z));
  M[14] = ((n.x * ep.x) + (n.y * ep.y) + (n.z * ep.z));
  M[15] = 1.;

#ifdef DEBUG
  print_array(&lp, "look point");
  print_array(&ep, "eye point");
  print_array(&n, "normal array");
  print_array(&v, "V = N x U");
  print_array(&u, "U = V x N");
  print_matrix(M, "final matrix");
#endif /* DEBUG */
}

void lookat_rotate(const Matrix T, LPC_FLOAT x, LPC_FLOAT y, LPC_FLOAT z, Matrix M) {
  Vector lp = {x, y, z};
  Vector ep = {T[12], T[13], T[14]};
  Vector u = {T[0], T[4], T[8]};

  build_lookat_matrix(ep, lp, u, M);
}

void lookat_rotate2(LPC_FLOAT ex, LPC_FLOAT ey, LPC_FLOAT ez, LPC_FLOAT lx, LPC_FLOAT ly,
                    LPC_FLOAT lz, Matrix M) {
  Vector ep = {ex, ey, ez};
  Vector lp = {lx, ly, lz};
  Vector u = {0., 1., 0.};

  build_lookat_matrix(ep, lp, u, M);
}

void translate_matrix(LPC_FLOAT x, LPC_FLOAT y, LPC_FLOAT z, Matrix m) {
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

void scale_matrix(LPC_FLOAT x, LPC_FLOAT y, LPC_FLOAT z, Matrix m) {
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

void rotate_x_matrix(LPC_FLOAT a, Matrix m) {
  LPC_FLOAT a_rad;
  LPC_FLOAT c, s;

  a_rad = (LPC_FLOAT)(a * RADIANS_PER_DEGREE);
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

void rotate_y_matrix(LPC_FLOAT a, Matrix m) {
  LPC_FLOAT a_rad;
  LPC_FLOAT c, s;

  a_rad = (LPC_FLOAT)(a * RADIANS_PER_DEGREE);
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

void rotate_z_matrix(LPC_FLOAT a, Matrix m) {
  LPC_FLOAT a_rad;
  LPC_FLOAT c, s;

  a_rad = (LPC_FLOAT)(a * RADIANS_PER_DEGREE);
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

void mult_matrix(const Matrix ma, const Matrix mb, Matrix m) {
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
