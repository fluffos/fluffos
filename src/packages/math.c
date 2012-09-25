/*
	math.c: this file contains the math efunctions called from
	inside eval_instruction() in interpret.c.
    -- coded by Truilkan 93/02/21
    
    Added norm, dotprod, distance, angle, log2.  
    Also added round() which works on floats.  If you do work with floats,
      you'll discover it's actually more useful than you'd think.
    Added int args to the efuns as apppropriate.
      - Hamlet 20090516
*/

#include <math.h>
#include <limits.h>
#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#include "../efun_protos.h"
#endif

#define SQUARE(n) ((n)*(n))

#ifdef F_COS
void
f_cos (void)
{
    sp->u.real = cos(sp->u.real);
}
#endif

#ifdef F_SIN
void
f_sin (void)
{
    sp->u.real = sin(sp->u.real);
}
#endif
#ifdef F_TAN
void
f_tan (void)
{
    /*
     * maybe should try to check that tan won't blow up (x != (Pi/2 + N*Pi))
     */
    sp->u.real = tan(sp->u.real);
}
#endif

#ifdef F_ASIN
void
f_asin (void)
{
    if (sp->u.real < -1.0) {
        error("math: asin(x) with (x < -1.0)\n");
        return;
    } else if (sp->u.real > 1.0) {
        error("math: asin(x) with (x > 1.0)\n");
        return;
    }
    sp->u.real = asin(sp->u.real);
}
#endif

#ifdef F_ACOS
void
f_acos (void)
{
    if (sp->u.real < -1.0) {
        error("math: acos(x) with (x < -1.0)\n");
        return;
    } else if (sp->u.real > 1.0) {
        error("math: acos(x) with (x > 1.0)\n");
        return;
    }
    sp->u.real = acos(sp->u.real);
}
#endif

#ifdef F_ATAN
void
f_atan (void)
{
    sp->u.real = atan(sp->u.real);
}
#endif

#ifdef F_SQRT
void
f_sqrt (void)
{   double val;

    if(sp->type == T_NUMBER)
      val = (double) sp->u.number;
    else
      val = sp->u.real;
      
    if (val < 0.0) {
        error("math: sqrt(x) with (x < 0.0)\n");
        return;
    }
    sp->u.real = (double) sqrt(val);
    sp->type = T_REAL;
}
#endif

#ifdef F_LOG
void
f_log (void)
{
    if (sp->u.real <= 0.0) {
        error("math: log(x) with (x <= 0.0)\n");
        return;
    }
    sp->u.real = log(sp->u.real);
}
#endif

#ifdef F_LOG10
void
f_log10 (void)
{
    double val;

    if(sp->type == T_NUMBER)
      val = (double) sp->u.number;
    else
      val = sp->u.real;

    if (val <= 0.0) {
        error("math: log10(x) with (x <= 0.0)\n");
        return;
    }
    sp->u.real = log10(val);
    sp->type = T_REAL;
}
#endif

#ifdef F_LOG2
void
f_log2 (void)
{
    double val;

    if(sp->type == T_NUMBER)
      val = (double) sp->u.number;
    else
      val = sp->u.real;

    if (val <= 0.0) {
        error("math: log2(x) with (x <= 0.0)\n");
        return;
    }
    sp->u.real = (double) log2((double)val);
    sp->type = T_REAL;
}
#endif

#ifdef F_POW
void
f_pow (void)
{
    double val, val2;
    
    if((sp-1)->type == T_NUMBER)
      val = (double) (sp-1)->u.number;
    else
      val = (sp-1)->u.real;
      
    if(sp->type == T_NUMBER)
      val2 = (double) sp->u.number;
    else
      val2 = sp->u.real;

    
    (sp - 1)->u.real = pow(val, val2);
    sp--;
    sp->type = T_REAL;
}
#endif

#ifdef F_EXP
void
f_exp (void)
{
    sp->u.real = exp(sp->u.real);
}
#endif

#ifdef F_FLOOR
void
f_floor (void)
{
    sp->u.real = floor(sp->u.real);
}
#endif

#ifdef F_CEIL
void
f_ceil (void)
{
    sp->u.real = ceil(sp->u.real);
}
#endif

#ifdef F_ROUND
void f_round (void)
{
    sp->u.real = (double) round(sp->u.real);
}
#endif

#ifdef F_NORM
/* The norm (magnitude) of a vector.
   Yes, you could use dotprod() below to implement norm(), but in the interest
   of speed, norm() has less cases.
*/
static double norm(array_t *a) {
  int len = sp->u.arr->size;
  double total = 0.0;

  while(len-- > 0)
    if(a->item[len].type == T_NUMBER)
      total += SQUARE((double) a->item[len].u.number);
    else if(a->item[len].type == T_REAL)
      total += SQUARE(a->item[len].u.real);
    else {
      return -INT_MAX + 1;
    }

  return (double) sqrt(total);
}

void f_norm(void) {
  double val = norm(sp->u.arr);

  if(val == (-INT_MAX + 1)) {
    pop_stack();
    error("norm: invalid argument 1.\n");
    return;
  }

  pop_stack();
  push_real(val);
}
#endif

#if defined(F_DOTPROD) | defined(F_DISTANCE) | defined(F_ANGLE)
static double vector_op(array_t *a, array_t *b, 
			  double (*func)(const double, const double)) {
  int len = a->size;
  double total = 0.0;

  if(b->size != len) {
    return -INT_MAX;
  }

  while(len-- > 0) {
    if(b->item[len].type == T_NUMBER) {
      if(a->item[len].type == T_NUMBER)
        total += func((double) a->item[len].u.number,
                      (double) b->item[len].u.number);
      else if(a->item[len].type == T_REAL)
        total += func(a->item[len].u.real,
                      (double) b->item[len].u.number);
      else {
        return -INT_MAX + 1;
      }
    }
    else if(b->item[len].type == T_REAL) {
      if(a->item[len].type == T_NUMBER)
        total += func((double) a->item[len].u.number, 
                      b->item[len].u.real);
                 
      else if(a->item[len].type == T_REAL)
        total += func(a->item[len].u.real, b->item[len].u.real);
      else {
        return -INT_MAX + 1;
      }
    }
    else {
      return -INT_MAX + 2;
    }
  }
  
  return total;
}
#endif

#ifdef F_DOTPROD
static double dotprod_mult(const double a, const double b) {
  return a * b;
}

/* dot product of two vectors */
static double dotprod(array_t *a, array_t *b) {
  return vector_op(a, b, dotprod_mult);
}

void f_dotprod(void) {
  double total = vector_op((sp-1)->u.arr, sp->u.arr, dotprod_mult);
  
  if(total == -INT_MAX) {
    pop_2_elems();
    error("dotprod: cannot take the dotprod of vectors of different sizes.\n");
    return;
  }

  if((total == (-INT_MAX + 1)) || (total == (-INT_MAX + 2))) {
    pop_2_elems();
    error("dotprod: invalid arg %d.\n", (total + INT_MAX));
    return;
  }
  
  pop_2_elems();
  push_real(total);
}
#endif

#ifdef F_DISTANCE
static double distance_mult(const double a, const double b) {
  return SQUARE(b - a);
}

/* The (Euclidian) distance between two points */
void f_distance(void) {
  double total = vector_op((sp-1)->u.arr, sp->u.arr, distance_mult);
  
  if(total == -INT_MAX) {
    pop_2_elems();
    error("distance: cannot take the distance of vectors of different sizes.\n");
    return;
  }

  if((total == (-INT_MAX + 1)) || (total == (-INT_MAX + 2))) {
    pop_2_elems();
    error("distance: invalid arg %d.\n", (total + INT_MAX));
    return;
  }
  
  pop_2_elems();
  push_real((double)sqrt(total));
}
#endif

#ifdef F_ANGLE
void f_angle(void) {
  double dot, norma, normb;
  
  dot = dotprod((sp-1)->u.arr, sp->u.arr);
  
  if(dot <= (-INT_MAX + 2)) {
    pop_2_elems();
    if(dot == -INT_MAX)
      error("angle: cannot calculate the angle between vectors of different sizes.\n");
    else
      error("angle: invalid arg %d.\n", (dot + INT_MAX));
    return;
  }
  
  norma = norm((sp-1)->u.arr);
  
  if(norma <= (-INT_MAX + 1)) {
    pop_2_elems();
    error("angle: invalid argument 1.\n");
    return;
  }

  normb = norm(sp->u.arr);
  
  if(normb <= (-INT_MAX + 1)) {
    pop_2_elems();
    error("angle: invalid argument 2.\n");
    return;
  }

  pop_2_elems();
  push_real((double)acos( dot / (norma * normb) ));
}
#endif
