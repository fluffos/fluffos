/*
	eoperators.c: this file contains all of the operators called from
	inside eval_instruction() in interpret.c.
*/

#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "lex.h"
#include "backend.h"
#include "eoperators.h"
#include "parse.h"
#include "swap.h"
#ifdef TRACE
#include "comm.h"
#endif
#include "compiler.h"

INLINE void
dealloc_funp P1(funptr_t *, fp)
{
    free_object(fp->hdr.owner, "free_funp");
    if (fp->hdr.args)
	free_array(fp->hdr.args);
    if ((fp->hdr.type & 0x0f) == FP_FUNCTIONAL) {
    	fp->f.functional.prog->func_ref--;
#ifdef DEBUG
	if (d_flag)
	    printf("subtr func ref %s: now %i\n",
		   fp->f.functional.prog->name,
		   fp->f.functional.prog->func_ref);
#endif
	if (fp->f.functional.prog->func_ref == 0 
	    && fp->f.functional.prog->ref == 0)
	    deallocate_program(fp->f.functional.prog);
    }
    FREE(fp);
}

INLINE void
free_funp P1(funptr_t *, fp)
{
    fp->hdr.ref--;
    if (fp->hdr.ref > 0) {
	return;
    }
    dealloc_funp(fp);
}

INLINE void f_and()
{
    if (sp->type == T_ARRAY && (sp - 1)->type == T_ARRAY) {
	sp--;
	sp->u.arr = intersect_array((sp + 1)->u.arr, sp->u.arr);
	return;
    }
    CHECK_TYPES(sp - 1, T_NUMBER, 1, F_AND);
    CHECK_TYPES(sp, T_NUMBER, 2, F_AND);
    sp--;
    sp->u.number &= (sp + 1)->u.number;
}

INLINE void
f_and_eq()
{
    svalue_t *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to &=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to &=\n");
    sp->u.number = argp->u.number &= sp->u.number;
}

INLINE void
f_div_eq()
{
    svalue_t *argp = (sp--)->u.lvalue;

    switch (argp->type | sp->type){

	case T_NUMBER:
	{
	    if (!sp->u.number) error("Division by 0nn\n");
	    sp->u.number = argp->u.number /= sp->u.number;
	    break;
	}

	case T_REAL:
	{
	    if (sp->u.real == 0.0) error("Division by 0rr\n");
	    sp->u.real = argp->u.real /= sp->u.real;
	    break;
	}

	case T_NUMBER|T_REAL:
	{
	    if (sp->type == T_NUMBER){
		if (!sp->u.number) error("Division by 0rn\n");
		sp->u.real = argp->u.real /= sp->u.number;
		sp->type = T_REAL;
	    } else {
		if (sp->u.real == 0.0) error("Division by 0nr\n");
		sp->u.real = argp->u.number /= sp->u.real;
	    }
	    break;
	}

	default:
	{
	    if (!(sp->type & (T_NUMBER|T_REAL))) error("Bad right type to /=\n");
	    else error("Bad left type to /=\n");
	}
    }
}

INLINE void
f_eq()
{
    int i;

    switch (sp->type|(sp-1)->type) {
    case T_NUMBER:
	{
	    --sp;
	    sp->u.number = sp->u.number == (sp+1)->u.number;
	    return;
	}
	
    case T_REAL:
	{
	    --sp;
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real == (sp+1)->u.real;
	    return;
	}
	
    case T_NUMBER|T_REAL:
	{
	    if ((--sp)->type == T_NUMBER){
		sp->u.number = sp->u.number == (sp+1)->u.real;
	    }
	    else {
		sp->u.number = sp->u.real == (sp+1)->u.number;
		sp->type = T_NUMBER;
	    }
	    return;
	}
	
    case T_ARRAY:
	{
	    i = (sp-1)->u.arr == sp->u.arr;
	    free_array((sp--)->u.arr);
	    free_array(sp->u.arr);
	    break;
	}
	
    case T_MAPPING:
	{
	    i = (sp-1)->u.map == sp->u.map;
	    free_mapping((sp--)->u.map);
	    free_mapping(sp->u.map);
	    break;
	}
	
    case T_STRING:
	{
	    if (SVALUE_STRLEN_DIFFERS(sp-1,sp))
		i = 0;
	    else
		i = !strcmp((sp-1)->u.string, sp->u.string);
	    free_string_svalue(sp--);
	    free_string_svalue(sp);
	    break;
	}
	
    case T_OBJECT:
	{
	    i = (sp-1)->u.ob == sp->u.ob;
	    free_object((sp--)->u.ob, "f_eq: 1");
	    free_object(sp->u.ob, "f_eq: 2");
	    break;
	}
	
    case T_FUNCTION:
	{
	    i = (sp-1)->u.fp == sp->u.fp;
	    free_funp((sp--)->u.fp);
	    free_funp(sp->u.fp);
	    break;
	}
    case T_BUFFER:
	{
	    i = (sp-1)->u.buf == sp->u.buf;
	    free_buffer((sp--)->u.buf);
	    free_buffer(sp->u.buf);
	    break;
	}
    default:
	pop_stack();
	free_svalue(sp, "f_eq");
	i = 0;
    }
    /* args are freed, stack pointer points to spot for return value */
    sp->type = T_NUMBER;
    sp->subtype = 0;
    sp->u.number = i;
}

INLINE void
f_ge()
{
    int i = sp->type;
    switch ((--sp)->type | i) {
    case T_NUMBER:
	sp->u.number = sp->u.number >= (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real >= (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER | T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real >= (sp+1)->u.number;
	} else sp->u.number = sp->u.number >= (sp+1)->u.real;
	break;
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) >= 0;
	free_string_svalue(sp + 1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	{
	    switch ((sp++)->type) {
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_GE);
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_GE);
	    default:
		bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, F_GE);
	    }
	}
    }
}

INLINE void
f_gt() {
    int i = sp->type;
    switch ((--sp)->type | i) {
    case T_NUMBER:
	sp->u.number = sp->u.number > (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real > (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER | T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real > (sp+1)->u.number;
	} else sp->u.number = sp->u.number > (sp+1)->u.real;
	break;
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) > 0;
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	{
	    switch ((sp++)->type) {
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_GT);
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_GT);
	    default:
		bad_argument(sp-1, T_NUMBER | T_REAL | T_STRING, 1, F_GT);
	    }
	}
    }
}

INLINE void
f_le()
{
    int i = sp->type;
    switch((--sp)->type|i){
    case T_NUMBER:
	sp->u.number = sp->u.number <= (sp+1)->u.number;
	break;
	
    case T_REAL:
	sp->u.number = sp->u.real <= (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
	
    case T_NUMBER|T_REAL:
	if (i == T_NUMBER){
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real <= (sp+1)->u.number;
	} else sp->u.number = sp->u.number <= (sp+1)->u.real;
	break;
	
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) <= 0;
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
	
    default:
	{
	    switch((sp++)->type){
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_LE);
		
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_LE);
		
	    default:
		bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, F_LE);
	    }
	}
    }
}

INLINE void
f_lt() {
    int i = sp->type;
    switch (i | (--sp)->type) {
    case T_NUMBER:
	sp->u.number = sp->u.number < (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real < (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER|T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real < (sp+1)->u.number;
	} else sp->u.number = sp->u.number < (sp+1)->u.real;
	break;
    case T_STRING:
	i = (strcmp(sp->u.string, (sp + 1)->u.string) < 0);
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	switch ((sp++)->type) {
	case T_NUMBER:
	case T_REAL:
	    bad_argument(sp, T_NUMBER | T_REAL, 2, F_LT);
	case T_STRING:
	    bad_argument(sp, T_STRING, 2, F_LT);
	default:
	    bad_argument(sp-1, T_NUMBER | T_STRING | T_REAL, 1, F_LT);
	}
    }
}

INLINE void
f_lsh()
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, F_LSH);
    CHECK_TYPES(sp, T_NUMBER, 2, F_LSH);
    sp--;
    sp->u.number <<= (sp + 1)->u.number;
}

INLINE void
f_lsh_eq()
{
    svalue_t *argp;


    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to <<=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to <<=\n");
    sp->u.number = argp->u.number <<= sp->u.number;
}

INLINE void
f_mod_eq()
{
    svalue_t *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to %=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to %=\n");
    if (sp->u.number == 0)
	error("Modulo by 0\n");
    sp->u.number = argp->u.number %= sp->u.number;
}

INLINE void
f_mult_eq()
{
    svalue_t *argp = (sp--)->u.lvalue;

    switch(argp->type | sp->type) {
	case T_NUMBER:
	{
	    sp->u.number = argp->u.number *= sp->u.number;
	    break;
	}

	case T_REAL:
	{
	    sp->u.real = argp->u.real *= sp->u.real;
	    break;
	}

	case T_NUMBER|T_REAL:
	{
	    if (sp->type == T_NUMBER){
		sp->type = T_REAL;
		sp->u.real = argp->u.real *= sp->u.number;
	    }
	    else {
		sp->u.real = argp->u.number *= sp->u.real;
	    }
	    break;
	}

	case T_MAPPING:
	{
	    mapping_t *m = compose_mapping(argp->u.map, sp->u.map,0);
	    pop_stack();
	    push_mapping(m);
	    assign_svalue(argp, sp);
	    break;
	}

	default:
	{
            if (!(sp->type & (T_NUMBER|T_REAL|T_MAPPING))) error("Bad right type to *=\n");
            else error("Bad left type to *=\n");
	}
    }
}


INLINE void
f_ne()
{
    int i;

    switch (sp->type|(sp-1)->type) {
    case T_NUMBER:
        {
            --sp;
            sp->u.number = sp->u.number != (sp+1)->u.number;
            return;
	}

    case T_REAL:
        {
            --sp;
            sp->type = T_NUMBER;
            sp->u.number = sp->u.real != (sp+1)->u.real;
            return;
	}

    case T_NUMBER|T_REAL:
        {
            if ((--sp)->type == T_NUMBER){
                sp->u.number = sp->u.number != (sp+1)->u.real;
	    }
            else {
                sp->u.number = sp->u.real != (sp+1)->u.number;
                sp->type = T_NUMBER;
	    }
            return;
	}

    case T_ARRAY:
        {
            i = (sp-1)->u.arr != sp->u.arr;
	    free_array((sp--)->u.arr);
	    free_array(sp->u.arr);
            break;
	}

    case T_MAPPING:
        {
            i = (sp-1)->u.map != sp->u.map;
	    free_mapping((sp--)->u.map);
	    free_mapping(sp->u.map);
            break;
	}

    case T_STRING:
        {
	    if (SVALUE_STRLEN_DIFFERS(sp-1, sp))
		i = 1;
	    else
		i = !!strcmp((sp-1)->u.string, sp->u.string);
	    free_string_svalue(sp--);
	    free_string_svalue(sp);
            break;
	}

    case T_OBJECT:
        {
            i = (sp-1)->u.ob != sp->u.ob;
	    free_object((sp--)->u.ob, "f_ne: 1");
	    free_object(sp->u.ob, "f_ne: 2");
            break;
	}

    case T_FUNCTION:
        {
            i = (sp-1)->u.fp != sp->u.fp;
	    free_funp((sp--)->u.fp);
	    free_funp(sp->u.fp);
            break;
	}

    case T_BUFFER:
        {
            i = (sp-1)->u.buf != sp->u.buf;
	    free_buffer((sp--)->u.buf);
	    free_buffer(sp->u.buf);
            break;
	}

    default:
	pop_stack();
	free_svalue(sp, "f_ne");
	i = 1;
    }
    sp->type = T_NUMBER;
    sp->subtype = 0;
    sp->u.number = i;
}

INLINE void
f_or()
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, F_OR);
    CHECK_TYPES(sp, T_NUMBER, 2, F_OR);
    sp--;
    sp->u.number |= (sp + 1)->u.number;
}

INLINE void
f_or_eq()
{
    svalue_t *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to |=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to |=\n");
    sp->u.number = argp->u.number |= sp->u.number;
}

INLINE void
f_parse_command()
{
    svalue_t *arg;
    svalue_t *fp;
    int i;
    int num_arg;

    /*
     * get number of lvalue args
     */
    num_arg = EXTRACT_UCHAR(pc);
    pc++;

    /*
     * type checking on first three required parameters to parse_command()
     */
    arg = sp - 2;
    CHECK_TYPES(&arg[0], T_STRING, 1, F_PARSE_COMMAND);
    CHECK_TYPES(&arg[1], T_OBJECT | T_ARRAY, 2, F_PARSE_COMMAND);
    CHECK_TYPES(&arg[2], T_STRING, 3, F_PARSE_COMMAND);

    /*
     * allocate stack frame for rvalues and return value (number of matches);
     * perform some stack manipulation;
     */
    fp = sp;
    sp += num_arg + 1;
    arg = sp;
    *(arg--) = *(fp--);		/* move pattern to top of stack */
    *(arg--) = *(fp--);		/* move source object or array to just below 
				   the pattern */
    *(arg) = *(fp);		/* move source string just below the object */
    fp->type = T_NUMBER;

    /*
     * prep area for rvalues
     */
    for (i = 1; i <= num_arg; i++)
	fp[i].type = T_INVALID;

    /*
     * do it...
     */
    i = parse(arg[0].u.string, &arg[1], arg[2].u.string, &fp[1], num_arg);

    /*
     * remove mandatory parameters
     */
    pop_3_elems();

    /*
     * save return value on stack
     */
    fp->u.number = i;
}

INLINE void
f_range P1(int, code)
{
    int from, to, len;

    if ((sp-2)->type != T_NUMBER)
        error("Start of range [ .. ] interval must be a number.\n");
    if ((sp-1)->type != T_NUMBER)
        error("End of range [ .. ] interval must be a number.\n");

    switch(sp->type){
        case T_STRING:
        {
            char *res = sp->u.string;

            len = SVALUE_STRLEN(sp);
            to = (--sp)->u.number;
            if (code & 0x01) to = len - to;
#ifdef OLD_RANGE_BEHAVIOR
            if (to < 0) to += len;
#endif
            from = (--sp)->u.number;
            if (code & 0x10) from = len - from;
#ifdef OLD_RANGE_BEHAVIOR
            if (from < 0){
                if ((from += len) < 0) from = 0;
            }
#else
            if (from < 0) from = 0;
#endif
            if (to < from || from >= len){
                free_string_svalue(sp+2);
		sp->type = T_STRING;
		sp->subtype = STRING_CONSTANT;
		sp->u.string = "";
                return;
            }

            if (to >= len - 1){
                put_malloced_string(string_copy(res + from, "f_range"));
            } else {
                char *tmp;
                tmp = new_string(to - from + 1, "f_range");
                strncpy(tmp, res + from, to - from + 1);
                tmp[to - from + 1] = '\0';
                put_malloced_string(tmp);
            }
            free_string_svalue(sp + 2);
            break;
        }
        case T_BUFFER:
        {
            buffer_t *rbuf = sp->u.buf;

            len = rbuf->size;
            to = (--sp)->u.number;
            if (code & 0x01) to = len - to;
#ifdef OLD_RANGE_BEHAVIOR
            if (to < 0) to += len;
#endif
            from = (--sp)->u.number;
            if (code & 0x10) from = len - from;
#ifdef OLD_RANGE_BEHAVIOR
            if (from < 0){
                if ((from += len) < 0) from = 0;
            }
#else
            if (from < 0) from = 0;
#endif
            if (to < from || from >= len){
                free_buffer(rbuf);
                put_buffer(null_buffer());
                return;
            }
            if (to >= len) to = len - 1;
            {
                buffer_t *nbuf = allocate_buffer(to - from + 1);
                memcpy(nbuf->item, rbuf->item + from, to - from + 1);
                free_buffer(rbuf);
                put_buffer(nbuf);
            }
            break;
        }

        case T_ARRAY:
        {
            array_t *v = sp->u.arr;
            to = (--sp)->u.number;
            if (code & 0x01) to = v->size - to;
            from = (--sp)->u.number;
            if (code & 0x10) from = v->size - from;
            put_array(slice_array(v, from, to));
            break;
        }

        default:
            error("Bad argument to [ .. ] range operator.\n");
    }
}

INLINE void
f_extract_range P1(int, code)
{
    int from,  len;

    if ((sp-1)->type != T_NUMBER)
        error("Start of range [ .. ] interval must be a number.\n");

    switch(sp->type){
        case T_STRING:
        {
            char *res = sp->u.string;

            len = SVALUE_STRLEN(sp);
            from = (--sp)->u.number;
            if (code) from = len - from;
#ifdef OLD_RANGE_BEHAVIOR
            if (from < 0){
                if ((from += len) < 0) from = 0;
            }
#else
            if (from < 0) from = 0;
#endif
            if (from >= len) {
		sp->type = T_STRING;
		sp->subtype = STRING_CONSTANT;
		sp->u.string = "";
	    } else 
		put_malloced_string(string_copy(res + from, "f_extract_range"));
            free_string_svalue(sp + 1);
            break;
        }
        case T_BUFFER:
        {
            buffer_t *rbuf = sp->u.buf;
            buffer_t *nbuf;


            len = rbuf->size;
            from = (--sp)->u.number;
            if (code) from = len - from;
#ifdef OLD_RANGE_BEHAVIOR
            if (from < 0){
                if ((from += len) < 0) from = 0;
            }
#else
            if (from < 0) from = 0;
#endif
	    if (from > len) from = len;
            nbuf = allocate_buffer(len - from);
            memcpy(nbuf->item, rbuf->item + from, len - from);
            free_buffer(rbuf);
            put_buffer(nbuf);
            break;
        }

        case T_ARRAY:
        {
            array_t *v = sp->u.arr;
            from = (--sp)->u.number;
            if (code) from = v->size - from;
            put_array(slice_array(v, from, v->size - 1));
            break;
        }

        default:
            error("Bad argument to [ .. ] range operator.\n");
    }
}

INLINE void
f_rsh()
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, F_RSH);
    CHECK_TYPES(sp, T_NUMBER, 2, F_RSH);
    sp--;
    sp->u.number >>= (sp + 1)->u.number;
}

INLINE void
f_rsh_eq()
{
    svalue_t *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to >>=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to >>=\n");
    sp->u.number = argp->u.number >>= sp->u.number;
}

INLINE void
f_sub_eq()
{
    svalue_t *argp = (sp--)->u.lvalue;

    switch(argp->type | sp->type){
	case T_NUMBER:
	{
	    sp->u.number = argp->u.number -= sp->u.number;
	    break;
	}

	case T_REAL:
	{
	    sp->u.real = argp->u.real -= sp->u.real;
	    break;
	}

	case T_NUMBER|T_REAL:
	{
	    if (sp->type == T_NUMBER){
		sp->type = T_REAL;
		sp->u.real = argp->u.real -= sp->u.number;
	    } else sp->u.real = argp->u.number -= sp->u.real;
	    break;
	}

	case T_ARRAY:
	{
	    sp->u.arr = argp->u.arr = subtract_array(argp->u.arr, sp->u.arr);
	    sp->u.arr->ref++;
	    break;
	}

	default:
	{
	    if (!(sp->type & (T_NUMBER|T_REAL|T_ARRAY))) error("Bad right type to -=\n");
	    else if (!(argp->type & (T_NUMBER|T_REAL|T_ARRAY))) error("Bad left type to -=\n");
	    else error("Arguments to -= do not match in type.\n");
	}
    }	   
}

/*
 * Structure of F_SWITCH:
 *   table type (1 byte)
 *   address of table (1 short)
 *   address of break (1 short)
 *   address of default (1 short)
 *     then all the switch code
 *   switch table (varies)
 *
 * Table type is either
 *   0xfe  - integer labels, direct lookup.
 *           Table is followed by 1 int that is minimum key value.
 *           Each table entry is a short address to jump to.
 *   0xfN  - integer labels.  N is size as a power of 2.
 *           Each table entry is 1 long (key) followed by 1 short (address).
 *   0xNf  - string labels.  Otherwise same as for integer labels.
 *
 * For normal string or integer tables, if the address is 0 or 1,
 * the key is the lower end of a range, and the upper end is in
 * the next entry.  If it's a 0, the second address indicates a
 * direct lookup table (currently this case is never generated by
 * the compiler).  If it's a 1, the second address is used for
 * all keys in the range (corresponds to 'case x..y:' labels).
 *
 * Binary search is used on the normal tables.
 */

/* offsets from 'pc' */
#define SW_TYPE		0
#define SW_TABLE	1
#define SW_ENDTAB       3
#define SW_DEFAULT	5

/* offsets used for range (L_ for lower member, U_ for upper member) */
#define L_LOWER	0
#define L_TYPE	(sizeof(char *))
#define L_UPPER	(SWITCH_CASE_SIZE)
#define L_ADDR	(SWITCH_CASE_SIZE + sizeof(char *))
#define U_LOWER	(-SWITCH_CASE_SIZE)
#define U_TYPE	(-SWITCH_CASE_SIZE + sizeof(char *))
#define U_UPPER	0
#define U_ADDR	(sizeof(char *))

INLINE void
f_switch()
{
    unsigned short offset, end_off;
    int d;
    POINTER_INT s = 0;
    POINTER_INT r;
    int i;
    char *l, *end_tab;
    static unsigned short off_tab[] =
    {
	0 * SWITCH_CASE_SIZE, 1 * SWITCH_CASE_SIZE, 3 * SWITCH_CASE_SIZE,
	7 * SWITCH_CASE_SIZE, 15 * SWITCH_CASE_SIZE, 31 * SWITCH_CASE_SIZE,
	63 * SWITCH_CASE_SIZE, 127 * SWITCH_CASE_SIZE, 
	255 * SWITCH_CASE_SIZE, 511 * SWITCH_CASE_SIZE, 
	1023 * SWITCH_CASE_SIZE, 2047 * SWITCH_CASE_SIZE, 
	4095 * SWITCH_CASE_SIZE, 8191 * SWITCH_CASE_SIZE,
    };

    COPY_SHORT(&offset, pc + SW_TABLE);
    COPY_SHORT(&end_off, pc + SW_ENDTAB);
    if ((i = EXTRACT_UCHAR(pc) >> 4) != 0xf) {	/* String table, find correct
						 * key */
	if (sp->type == T_NUMBER && !sp->u.number) {
	    /* special case: 0 as a string */
	    s = 0;
	} else if (sp->type == T_STRING) {
	    switch (sp->subtype) {
	    case STRING_SHARED:
		s = (POINTER_INT) sp->u.string;
		break;
	    default:
		s = (POINTER_INT) findstring(sp->u.string);
		break;
	    }
	    if (s == 0) {
		/*
		 * Take default case now - else we could be get confused with
		 * ZERO_AS_STR_CASE_LABEL.
		 */
		COPY_SHORT(&offset, pc + SW_DEFAULT);
		pop_stack();
		pc = current_prog->program + offset;
		return;
	    }
	} else {
	    bad_argument(sp, T_STRING, 1, F_SWITCH);
	}
    } else {			/* Integer table, check type */
	CHECK_TYPES(sp, T_NUMBER, 1, F_SWITCH);
	s = sp->u.number;
	i = (int) pc[0] & 0xf;
    }
    pop_stack();
    end_tab = current_prog->program + end_off;
    /*
     * i is the table size as a power of 2.  Tells us where to start
     * searching.  i==14 is a special case.
     */
    if (i >= 14)
	if (i == 14) {
	    /* fastest switch format : lookup table */
	    l = current_prog->program + offset;
	    COPY_INT(&d, end_tab - 4);
	    /* d is minimum value - see if in range or not */
	    if (s >= d && l + (s = (s - d) * sizeof(short)) < (end_tab - 4)) {
		COPY_SHORT(&offset, &l[s]);
		if (offset) {
		    pc = current_prog->program + offset;
		    return;
		}
	    }
	    /* default */
	    COPY_SHORT(&offset, pc + SW_DEFAULT);
	    pc = current_prog->program + offset;
	    return;
	} else
	    fatal("unsupported switch table format.\n");

    /*
     * l - current entry we are looking at. 
     * d - size to add/subtract from l each iteration. 
     * s - key we're looking for 
     * r - key l is pointing at
     */
    l = current_prog->program + offset + off_tab[i];
    d = (int) (off_tab[i] + SWITCH_CASE_SIZE) >> 1;
    if (d < SWITCH_CASE_SIZE)
	d = 0;
    for (;;) {
	COPY_PTR(&r, l);
	if (s < r) {
	    if (d < SWITCH_CASE_SIZE) {
		/* test if entry is part of a range */
		/* Don't worry about reading from F_BREAK (byte before table) */
		COPY_SHORT(&offset, l + U_TYPE);
		if (offset <= 1) {
		    COPY_PTR(&r, l + U_LOWER);
		    if (s >= r) {
			/* s is in the range */
			COPY_SHORT(&offset, l + U_ADDR);
			if (!offset) {
			    /* range with lookup table */
			    l = current_prog->program + offset +
				(s - r) * sizeof(short);
			    COPY_SHORT(&offset, l);
			}	/* else normal range and offset is correct */
			break;
		    }
		}
		/* key not found, use default address */
		COPY_SHORT(&offset, pc + SW_DEFAULT);
		break;
	    } else {
		/* d >= SWITCH_CASE_SIZE */
		l -= d;
		d >>= 1;
	    }
	} else if (s > r) {
	    if (d < SWITCH_CASE_SIZE) {
		/* test if entry is part of a range */
		COPY_SHORT(&offset, l + L_TYPE);
		if (offset <= 1) {
		    COPY_PTR(&r, l + L_UPPER);
		    if (s <= r) {
			/* s is in the range */
			COPY_SHORT(&offset, l + L_ADDR);
			if (!offset) {
			    /* range with lookup table */
			    l = current_prog->program + offset +
				(s - r) * sizeof(short);
			    COPY_SHORT(&offset, l);
			}	/* else normal range and offset is correct */
			break;
		    }
		}
		/* use default address */
		COPY_SHORT(&offset, pc + SW_DEFAULT);
		break;
	    } else {		/* d >= SWITCH_CASE_SIZE */
		l += d;
		/* if table isn't a power of 2 in size, fix us up */
		while (l >= end_tab) {
		    d >>= 1;
		    if (d < SWITCH_CASE_SIZE) {
			d = 0;
			break;
		    }
		    l -= d;
		}
		if (l == end_tab) {
		    /* use default address */
		    COPY_SHORT(&offset, pc + SW_DEFAULT);
		    break;
		}
		d >>= 1;
	    }
	} else {
	    /* s == r */
	    COPY_SHORT(&offset, l + U_ADDR);
	    /* found the key - but could be part of a range... */
	    if (!l[U_TYPE] && !l[U_TYPE + 1]) {
		/* end of range with lookup table */
		COPY_PTR(&r, l + U_LOWER);
		l = current_prog->program + offset + (s - r) * sizeof(short);
		COPY_SHORT(&offset, l);
	    }
	    if (offset <= 1) {
		COPY_SHORT(&offset, l + L_ADDR);
		if (!offset) {
		    /* start of range with lookup table */
		    l = current_prog->program + offset;
		    COPY_SHORT(&offset, l);
		}		/* else normal range, offset is correct */
	    }
	    break;
	}
    }
    /* now do jump */
    pc = current_prog->program + offset;
}

void
call_simul_efun P2(unsigned short, index, int, num_arg)
{
    function_t *funp;
    extern object_t *simul_efun_ob;
    extern function_t **simuls;

    if (current_object->flags & O_DESTRUCTED) {	/* No external calls allowed */
	pop_n_elems(num_arg);
	push_undefined();
	return;
    }

#ifdef TRACE
    if (TRACEP(TRACE_CALL_OTHER)) {
	do_trace("simul_efun ", simuls[index]->name, "\n");
    }
#endif
    if (simuls[index]) {
	/* Don't need to use apply() since we have the pointer directly;
	 * this saves function lookup.
	 */
	if (simul_efun_ob->flags & O_SWAPPED)
	    load_ob_from_swap(simul_efun_ob);
	simul_efun_ob->time_of_ref = current_time;
	push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE, simuls[index]);
	caller_type = ORIGIN_SIMUL_EFUN;
	csp->num_local_variables = num_arg;
	current_prog = simul_efun_ob->prog;
	funp = setup_new_frame(simuls[index]);
#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
	if (current_object != simul_efun_ob)
#endif
	    previous_ob = current_object;
	current_object = simul_efun_ob;
	call_program(current_prog, funp->offset);
    } else error("Function is no longer a simul_efun.\n");
}

INLINE void
f_xor()
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, F_XOR);
    CHECK_TYPES(sp, T_NUMBER, 2, F_XOR);
    sp--;
    sp->u.number ^= (sp + 1)->u.number;
}

INLINE void
f_xor_eq()
{
    svalue_t *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to ^=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to ^=\n");
    sp->u.number = argp->u.number ^= sp->u.number;
}

INLINE funptr_t *
make_efun_funp P2(int, opcode, svalue_t *, args)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(efun_ptr_t),
			     TAG_FUNP, "make_efun_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->hdr.type = FP_EFUN;
    
    fp->f.efun.index = opcode;
    
    if (args->type == T_ARRAY) {
	fp->hdr.args = args->u.arr;
	args->u.arr->ref++;
    } else
	fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_lfun_funp P2(int, index, svalue_t *, args)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(local_ptr_t),
			     TAG_FUNP, "make_efun_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->hdr.type = FP_LOCAL | FP_NOT_BINDABLE;
    
    fp->f.local.index = index + function_index_offset;
    
    if (args->type == T_ARRAY) {
	fp->hdr.args = args->u.arr;
	args->u.arr->ref++;
    } else
	fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_simul_funp P2(int, index, svalue_t *, args)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(simul_ptr_t),
			     TAG_FUNP, "make_efun_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->hdr.type = FP_SIMUL;
    
    fp->f.simul.index = index;
    
    if (args->type == T_ARRAY) {
	fp->hdr.args = args->u.arr;
	args->u.arr->ref++;
    } else
	fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_functional_funp P5(short, num_arg, short, num_local, short, len, svalue_t *, args, int, flag)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(functional_t), 
			     TAG_FUNP, "make_functional_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_functional_funp" );
    fp->hdr.type = FP_FUNCTIONAL + flag;

    current_prog->func_ref++;
#ifdef DEBUG
	if (d_flag)
	    printf("add func ref %s: now %i\n",
		   current_prog->name,
		   current_prog->func_ref);
#endif
    
    fp->f.functional.prog = current_prog;
    fp->f.functional.offset = pc - current_prog->program;
    fp->f.functional.num_arg = num_arg;
    fp->f.functional.num_local = num_local;
    fp->f.functional.fio = function_index_offset;
    fp->f.functional.vio = variable_index_offset;
    pc += len;
    
    if (args && args->type == T_ARRAY) {
	fp->hdr.args = args->u.arr;
	args->u.arr->ref++;
	fp->f.functional.num_arg += args->u.arr->size;
    } else
	fp->hdr.args = 0;

    fp->hdr.ref = 1;
    return fp;
}

INLINE void
push_refed_funp P1(funptr_t *, fp)
{
    sp++;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
}

INLINE void
push_funp P1(funptr_t *, fp)
{
    sp++;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
    fp->hdr.ref++;
}

INLINE void
f_function_constructor()
{
    funptr_t *fp;
    int kind;
    unsigned short index;

    kind = EXTRACT_UCHAR(pc++);

    switch (kind) {
    case FP_EFUN:
	LOAD_SHORT(index, pc);
	fp = make_efun_funp(index, sp);
	pop_stack();
	break;
    case FP_LOCAL:
	LOAD_SHORT(index, pc);
	fp = make_lfun_funp(index, sp); 
	pop_stack();
	break;
    case FP_SIMUL:
	LOAD_SHORT(index, pc);
	fp = make_simul_funp(index, sp); 
	pop_stack();
	break;
    case FP_FUNCTIONAL:
    case FP_FUNCTIONAL | FP_NOT_BINDABLE:
	{
	    int num_arg;

	    num_arg = EXTRACT_UCHAR(pc++);  /* number of arguments */
	    LOAD_SHORT(index, pc);       /* length of functional */
	    fp = make_functional_funp(num_arg, 0, index, sp, kind & FP_NOT_BINDABLE);
	    pop_stack();
	    break;
	}
    case FP_ANONYMOUS:
	{
	    int num_arg, locals;
	    
	    num_arg = EXTRACT_UCHAR(pc++);
	    locals = EXTRACT_UCHAR(pc++);
	    LOAD_SHORT(index, pc); /* length */
	    fp = make_functional_funp(num_arg, locals, index, 0, FP_NOT_BINDABLE);
	    break;
	}
    default:
	fatal("Tried to make unknown type of function pointer.\n");
    }
    push_refed_funp(fp);
}

INLINE void
f_evaluate PROT((void))
{
    svalue_t *v;
    svalue_t *arg = sp - st_num_arg + 1;

    if (arg->type != T_FUNCTION) {
	pop_n_elems(st_num_arg-1);
	return;
    }
    if (current_object->flags & O_DESTRUCTED) {
	pop_n_elems(st_num_arg);
	push_undefined();
	return;
    }
    v = call_function_pointer(arg->u.fp, st_num_arg - 1);
    free_funp(arg->u.fp);
    assign_svalue_no_free(sp, v);
}

INLINE void
f_sscanf()
{
    svalue_t *fp;
    int i;
    int num_arg;

    /*
     * get number of lvalue args
     */
    num_arg = EXTRACT_UCHAR(pc);
    pc++;

    /*
     * allocate stack frame for rvalues and return value (number of matches);
     * perform some stack manipulation; note: source and template strings are
     * already on the stack by this time
     */
    fp = sp;
    sp += num_arg + 1;
    *sp = *(fp--);		/* move format description to top of stack */
    *(sp - 1) = *(fp);		/* move source string just below the format
				 * desc. */
    fp->type = T_NUMBER;	/* this svalue isn't invalidated below, and
				 * if we don't change it to something safe,
				 * it will get freed twice if an error occurs */
    /*
     * prep area for rvalues
     */
    for (i = 1; i <= num_arg; i++)
	fp[i].type = T_INVALID;

    /*
     * do it...
     */
    i = inter_sscanf(sp - 2, sp - 1, sp, num_arg);

    /*
     * remove source & template strings from top of stack
     */
    pop_2_elems();

    /*
     * save number of matches on stack
     */
    fp->u.number = i;
}
