/*
	math.c: this file contains the math efunctions called from
	inside eval_instruction() in interpret.c.
    -- coded by Truilkan 93/02/21
*/

#include <math.h>
#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#include "../efun_protos.h"
#endif

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
{
    if (sp->u.real < 0.0) {
        error("math: sqrt(x) with (x < 0.0)\n");
        return;
    }
    sp->u.real = sqrt(sp->u.real);
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
    if (sp->u.real <= 0.0) {
        error("math: log10(x) with (x <= 0.0)\n");
        return;
    }
    sp->u.real = log10(sp->u.real);
}
#endif

#ifdef F_POW
void
f_pow (void)
{
    (sp - 1)->u.real = pow((sp - 1)->u.real, sp->u.real);
    sp--;
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
