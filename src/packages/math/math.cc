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

/* float -> float efuns. */
#define LPC_EFUN_UNARY_REAL(name, expr) \
  vm::Value f_##name(vm::EfunArgs&, LPC_FLOAT x) { return (expr)(x); }

#define SQUARE(n) ((n) * (n))

#ifdef F_COS
LPC_EFUN_UNARY_REAL(cos, cos)
#endif

#ifdef F_SIN
LPC_EFUN_UNARY_REAL(sin, sin)
#endif
#ifdef F_TAN
/* maybe should try to check that tan won't blow up (x != (Pi/2 + N*Pi)) */
LPC_EFUN_UNARY_REAL(tan, tan)
#endif

#ifdef F_ASIN
vm::Value f_asin(vm::EfunArgs&, LPC_FLOAT x) {
  if (x < -1.0) {
    error("math: asin(x) with (x < -1.0)\n");
  }
  if (x > 1.0) {
    error("math: asin(x) with (x > 1.0)\n");
  }
  return asin(x);
}
#endif

#ifdef F_ACOS
vm::Value f_acos(vm::EfunArgs&, LPC_FLOAT x) {
  if (x < -1.0) {
    error("math: acos(x) with (x < -1.0)\n");
  }
  if (x > 1.0) {
    error("math: acos(x) with (x > 1.0)\n");
  }
  return acos(x);
}
#endif

#ifdef F_ATAN
LPC_EFUN_UNARY_REAL(atan, atan)
#endif

#ifdef F_SQRT
vm::Value f_sqrt(vm::EfunArgs&, vm::Arg x) {
  LPC_FLOAT val = x.as_real();

  if (val < 0.0) {
    error("math: sqrt(x) with (x < 0.0)\n");
  }
  return sqrt(val);
}
#endif

#ifdef F_LOG
vm::Value f_log(vm::EfunArgs&, LPC_FLOAT x) {
  if (x <= 0.0) {
    error("math: log(x) with (x <= 0.0)\n");
  }
  return log(x);
}
#endif

#ifdef F_LOG10
vm::Value f_log10(vm::EfunArgs&, vm::Arg x) {
  LPC_FLOAT val = x.as_real();

  if (val <= 0.0) {
    error("math: log10(x) with (x <= 0.0)\n");
  }
  return log10(val);
}
#endif

#ifdef F_LOG2
vm::Value f_log2(vm::EfunArgs&, vm::Arg x) {
  LPC_FLOAT val = x.as_real();

  if (val <= 0.0) {
    error("math: log2(x) with (x <= 0.0)\n");
  }
  return log2(val);
}
#endif

#ifdef F_POW
vm::Value f_pow(vm::EfunArgs&, vm::Arg base, vm::Arg exponent) {
  return pow(base.as_real(), exponent.as_real());
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

vm::Value f_norm(vm::EfunArgs&, array_t* vec) {
  bool ok;
  LPC_FLOAT const val = norm(vec, &ok);

  if (!ok) {
    error("norm: invalid argument 1.\n");
  }

  return val;
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

vm::Value f_dotprod(vm::EfunArgs&, array_t* a, array_t* b) {
  int status;
  LPC_FLOAT const total = vector_op(a, b, dotprod_mult, &status);

  if (status == VEC_ERR_SIZE_MISMATCH) {
    error("dotprod: cannot take the dotprod of vectors of different sizes.\n");
  }

  if (status == VEC_ERR_ARG1 || status == VEC_ERR_ARG2) {
    error("dotprod: invalid arg %d.\n", status);
  }

  return total;
}
#endif

#ifdef F_DISTANCE
static LPC_FLOAT distance_mult(const LPC_FLOAT a, const LPC_FLOAT b) { return SQUARE(b - a); }

/* The (Euclidian) distance between two points */
vm::Value f_distance(vm::EfunArgs&, array_t* a, array_t* b) {
  int status;
  LPC_FLOAT const total = vector_op(a, b, distance_mult, &status);

  if (status == VEC_ERR_SIZE_MISMATCH) {
    error("distance: cannot take the distance of vectors of different sizes.\n");
  }

  if (status == VEC_ERR_ARG1 || status == VEC_ERR_ARG2) {
    error("distance: invalid arg %d.\n", status);
  }

  return sqrt(total);
}
#endif

#ifdef F_ANGLE
vm::Value f_angle(vm::EfunArgs&, array_t* a, array_t* b) {
  LPC_FLOAT dot, norma, normb;
  int status;
  bool ok;

  dot = dotprod(a, b, &status);

  if (status != VEC_OK) {
    if (status == VEC_ERR_SIZE_MISMATCH) {
      error(
          "angle: cannot calculate the angle between vectors of different "
          "sizes.\n");
    } else {
      error("angle: invalid arg %d.\n", status);
    }
  }

  norma = norm(a, &ok);

  if (!ok) {
    error("angle: invalid argument 1.\n");
  }

  normb = norm(b, &ok);

  if (!ok) {
    error("angle: invalid argument 2.\n");
  }

  return acos(dot / (norma * normb));
}
#endif
