/*
	math.c: this file contains the math efunctions called from
	inside eval_instruction() in interpret.c.
    -- coded by Truilkan 93/02/21
*/

#include <math.h>
#include "efuns.h"

#ifdef MATH

static double result;

#ifdef F_COS
void
f_cos(num_arg, instruction)
int num_arg, instruction;
{
	result = cos(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_SIN
void
f_sin(num_arg, instruction)
int num_arg, instruction;
{
	result = sin(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_TAN
void
f_tan(num_arg, instruction)
int num_arg, instruction;
{
	/*
     * maybe should try to check that tan won't blow up (x != (Pi/2 + N*Pi))
	 */
	result = tan(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_ASIN
void
f_asin(num_arg, instruction)
int num_arg, instruction;
{
	result = asin(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_ACOS
void
f_acos(num_arg, instruction)
int num_arg, instruction;
{
	result = acos(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_ATAN
void
f_atan(num_arg, instruction)
int num_arg, instruction;
{
	result = atan(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_SQRT
void
f_sqrt(num_arg, instruction)
int num_arg, instruction;
{
	if (sp->u.real < 0.0) {
		error("math: sqrt(x) with (x < 0.0)\n");
		return;
	}
	result = sqrt(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_LOG
void
f_log(num_arg, instruction)
int num_arg, instruction;
{
	if (sp->u.real <= 0.0) {
		error("math: log(x) with (x <= 0.0)\n");
		return;
	}
	result = log(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_POW
void
f_pow(num_arg, instruction)
int num_arg, instruction;
{
	result = pow((sp - 1)->u.real, sp->u.real);
	pop_n_elems(2);
	push_real(result);
}
#endif

#ifdef F_EXP
void
f_exp(num_arg, instruction)
int num_arg, instruction;
{
	result = exp(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_FLOOR
void
f_floor(num_arg, instruction)
int num_arg, instruction;
{
	result = floor(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#ifdef F_CEIL
void
f_ceil(num_arg, instruction)
int num_arg, instruction;
{
	result = ceil(sp->u.real);
	pop_stack();
	push_real(result);
}
#endif

#endif
