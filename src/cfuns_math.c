/*
	math.c: this file contains the math efunctions called from
	inside eval_instruction() in interpret.c.
    -- coded by Truilkan 93/02/21
*/

#include <math.h>

#include "efuns.h"
#include "cfuns.h"
#ifdef LPC_TO_C

#ifdef MATH

#ifdef F_COS
void c_cos P2(svalue *, ret, svalue *, s0)
{
    C_REAL(ret, cos(s0->u.real));
}
#endif

#ifdef F_SIN
void c_sin P2(svalue *, ret, svalue *, s0)
{
    C_REAL(ret, sin(s0->u.real));
}
#endif

#ifdef F_TAN
void c_tan P2(svalue *, ret, svalue *, s0)
{
/*
 * maybe should try to check that tan won't blow up (x != (Pi/2 + N*Pi))
 */
    C_REAL(ret, tan(s0->u.real));
}
#endif

#ifdef F_ASIN
void c_asin P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.real < -1.0) {
	error("math: asin(x) with (x < -1.0)\n");
	return;
    } else if (s0->u.real > 1.0) {
	error("math: asin(x) with (x > 1.0)\n");
	return;
    }
    C_REAL(ret, asin(s0->u.real));
}
#endif

#ifdef F_ACOS
void c_acos P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.real < -1.0) {
	error("math: acos(x) with (x < -1.0)\n");
	return;
    } else if (s0->u.real > 1.0) {
	error("math: acos(x) with (x > 1.0)\n");
	return;
    }
    C_REAL(ret, acos(s0->u.real));
}
#endif

#ifdef F_ATAN
void c_atan P2(svalue *, ret, svalue *, s0)
{
    C_REAL(ret, atan(s0->u.real));
}
#endif

#ifdef F_SQRT
void c_sqrt P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.real < 0.0) {
	error("math: sqrt(x) with (x < 0.0)\n");
	return;
    }
    C_REAL(ret, sqrt(s0->u.real));
}
#endif

#ifdef F_LOG
void c_log P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.real <= 0.0) {
	error("math: log(x) with (x <= 0.0)\n");
	return;
    }
    C_REAL(ret, log(s0->u.real));
}
#endif

#ifdef F_POW
void c_pow P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_REAL(ret, pow(s0->u.real, s1->u.real));
}
#endif

#ifdef F_EXP
void c_exp P2(svalue *, ret, svalue *, s0)
{
    C_REAL(ret, exp(s0->u.real));
}
#endif

#ifdef F_FLOOR
void c_floor P2(svalue *, ret, svalue *, s0)
{

    C_REAL(ret, floor(s0->u.real));
}
#endif

#ifdef F_CEIL
void c_ceil P2(svalue *, ret, svalue *, s0)
{
    C_REAL(ret, ceil(s0->u.real));
}
#endif

#endif
#endif
