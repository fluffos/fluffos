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

#include "base/package_api.h"

#include <cmath>
#include <climits>

#define SQUARE(n) ((n) * (n))

#ifdef F_COS
LPC_EFUN_UNARY_REAL(cos, cos)
#endif

#ifdef F_SIN
LPC_EFUN_UNARY_REAL(sin, sin)
#endif
#ifdef F_TAN
void f_tan(LPCFrame& f) {
  /*
   * maybe should try to check that tan won't blow up (x != (Pi/2 + N*Pi))
   */
  f.top().u.real = tan(f.top().u.real);
  f.done();
}
#endif

#ifdef F_ASIN
void f_asin(LPCFrame& f) {
  if (f.top().u.real < -1.0) {
    error("math: asin(x) with (x < -1.0)\n");
  }
  if (f.top().u.real > 1.0) {
    error("math: asin(x) with (x > 1.0)\n");
  }
  f.top().u.real = asin(f.top().u.real);
  f.done();
}
#endif

#ifdef F_ACOS
void f_acos(LPCFrame& f) {
  if (f.top().u.real < -1.0) {
    error("math: acos(x) with (x < -1.0)\n");
  }
  if (f.top().u.real > 1.0) {
    error("math: acos(x) with (x > 1.0)\n");
  }
  f.top().u.real = acos(f.top().u.real);
  f.done();
}
#endif

#ifdef F_ATAN
LPC_EFUN_UNARY_REAL(atan, atan)
#endif

#ifdef F_SQRT
void f_sqrt(LPCFrame& f) {
  LPC_FLOAT val = f.as_real(0);

  if (val < 0.0) {
    error("math: sqrt(x) with (x < 0.0)\n");
  }
  f.top().type = T_REAL;
  f.top().u.real = sqrt(val);
  f.done();
}
#endif

#ifdef F_LOG
void f_log(LPCFrame& f) {
  if (f.top().u.real <= 0.0) {
    error("math: log(x) with (x <= 0.0)\n");
  }
  f.top().u.real = log(f.top().u.real);
  f.done();
}
#endif

#ifdef F_LOG10
void f_log10(LPCFrame& f) {
  LPC_FLOAT val = f.as_real(0);

  if (val <= 0.0) {
    error("math: log10(x) with (x <= 0.0)\n");
  }
  f.top().type = T_REAL;
  f.top().u.real = log10(val);
  f.done();
}
#endif

#ifdef F_LOG2
void f_log2(LPCFrame& f) {
  LPC_FLOAT val = f.as_real(0);

  if (val <= 0.0) {
    error("math: log2(x) with (x <= 0.0)\n");
  }
  f.top().type = T_REAL;
  f.top().u.real = log2(val);
  f.done();
}
#endif

#ifdef F_POW
void f_pow(LPCFrame& f) {
  LPC_FLOAT val = f.as_real(0);
  LPC_FLOAT val2 = f.as_real(1);
  f.ret(pow(val, val2));
}
#endif

#ifdef F_EXP
LPC_EFUN_UNARY_REAL(exp, exp)
#endif

#ifdef F_FLOOR
LPC_EFUN_UNARY_REAL(floor, floor)
#endif

#ifdef F_CEIL
LPC_EFUN_UNARY_REAL(ceil, ceil)
#endif

#ifdef F_ROUND
LPC_EFUN_UNARY_REAL(round, round)
#endif

#ifdef F_NORM
/* The norm (magnitude) of a vector.
   Yes, you could use dotprod() below to implement norm(), but in the interest
   of speed, norm() has less cases.
*/
// norm()/vector_op() used to signal errors by returning special sentinel
// float values (-INT_MAX, -INT_MAX+1, -INT_MAX+2) instead of a real
// result. A legitimate result can coincide with one of those exact
// values (e.g. a dotprod() of large-magnitude vectors landing on
// -INT_MAX+1), which would misreport a valid computation as an error.
// Report success/failure out-of-band instead.
enum { VEC_OK = 0, VEC_ERR_SIZE_MISMATCH = -1, VEC_ERR_ARG1 = 1, VEC_ERR_ARG2 = 2 };

static LPC_FLOAT norm(array_t* a, bool* ok) {
  LPC_INT len = a->size;
  LPC_FLOAT total = 0.0;
  *ok = true;

  while (len-- > 0) {
    if (a->item[len].type == T_NUMBER) {
      total += SQUARE((LPC_FLOAT)a->item[len].u.number);
    } else if (a->item[len].type == T_REAL) {
      total += SQUARE(a->item[len].u.real);
    } else {
      *ok = false;
      return 0;
    }
  }

  return sqrt(total);
}

void f_norm(LPCFrame& f) {
  bool ok;
  LPC_FLOAT const val = norm(f.array(0), &ok);

  if (!ok) {
    f.ret_void();
    error("norm: invalid argument 1.\n");
  }

  f.ret(val);
}
#endif

#if defined(F_DOTPROD) | defined(F_DISTANCE) | defined(F_ANGLE)
static LPC_FLOAT vector_op(array_t* a, array_t* b,
                           LPC_FLOAT (*func)(const LPC_FLOAT, const LPC_FLOAT), int* status) {
  LPC_INT len = a->size;
  LPC_FLOAT total = 0.0;
  *status = VEC_OK;

  if (b->size != len) {
    *status = VEC_ERR_SIZE_MISMATCH;
    return 0;
  }

  while (len-- > 0) {
    if (b->item[len].type == T_NUMBER) {
      if (a->item[len].type == T_NUMBER) {
        total += func(static_cast<LPC_FLOAT>(a->item[len].u.number),
                      static_cast<LPC_FLOAT>(b->item[len].u.number));
      } else if (a->item[len].type == T_REAL) {
        total += func(a->item[len].u.real, static_cast<LPC_FLOAT>(b->item[len].u.number));
      } else {
        *status = VEC_ERR_ARG1;
        return 0;
      }
    } else if (b->item[len].type == T_REAL) {
      if (a->item[len].type == T_NUMBER) {
        total += func(static_cast<LPC_FLOAT>(a->item[len].u.number), b->item[len].u.real);

      } else if (a->item[len].type == T_REAL) {
        total += func(a->item[len].u.real, b->item[len].u.real);
      } else {
        *status = VEC_ERR_ARG1;
        return 0;
      }
    } else {
      *status = VEC_ERR_ARG2;
      return 0;
    }
  }

  return total;
}
#endif

#ifdef F_DOTPROD
static LPC_FLOAT dotprod_mult(const LPC_FLOAT a, const LPC_FLOAT b) { return a * b; }

/* dot product of two vectors */
static LPC_FLOAT dotprod(array_t* a, array_t* b, int* status) {
  return vector_op(a, b, dotprod_mult, status);
}

void f_dotprod(LPCFrame& f) {
  int status;
  LPC_FLOAT const total = vector_op(f.array(0), f.array(1), dotprod_mult, &status);

  if (status == VEC_ERR_SIZE_MISMATCH) {
    f.ret_void();
    error("dotprod: cannot take the dotprod of vectors of different sizes.\n");
  }

  if (status == VEC_ERR_ARG1 || status == VEC_ERR_ARG2) {
    f.ret_void();
    error("dotprod: invalid arg %d.\n", status);
  }

  f.ret(total);
}
#endif

#ifdef F_DISTANCE
static LPC_FLOAT distance_mult(const LPC_FLOAT a, const LPC_FLOAT b) { return SQUARE(b - a); }

/* The (Euclidian) distance between two points */
void f_distance(LPCFrame& f) {
  int status;
  LPC_FLOAT const total = vector_op(f.array(0), f.array(1), distance_mult, &status);

  if (status == VEC_ERR_SIZE_MISMATCH) {
    f.ret_void();
    error("distance: cannot take the distance of vectors of different sizes.\n");
  }

  if (status == VEC_ERR_ARG1 || status == VEC_ERR_ARG2) {
    f.ret_void();
    error("distance: invalid arg %d.\n", status);
  }

  f.ret(sqrt(total));
}
#endif

#ifdef F_ANGLE
void f_angle(LPCFrame& f) {
  LPC_FLOAT dot, norma, normb;
  int status;
  bool ok;

  dot = dotprod(f.array(0), f.array(1), &status);

  if (status != VEC_OK) {
    f.ret_void();
    if (status == VEC_ERR_SIZE_MISMATCH) {
      error(
          "angle: cannot calculate the angle between vectors of different "
          "sizes.\n");
    } else {
      error("angle: invalid arg %d.\n", status);
    }
  }

  norma = norm(f.array(0), &ok);

  if (!ok) {
    f.ret_void();
    error("angle: invalid argument 1.\n");
  }

  normb = norm(f.array(1), &ok);

  if (!ok) {
    f.ret_void();
    error("angle: invalid argument 2.\n");
  }

  f.ret(acos(dot / (norma * normb)));
}
#endif
