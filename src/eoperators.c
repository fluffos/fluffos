/*
	eoperators.c: this file contains all of the operators called from
	inside eval_instruction() in interpret.c.
*/

#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "backend.h"
#include "eoperators.h"
#include "parse.h"
#include "swap.h"
#ifdef TRACE
#include "comm.h"
#endif

INLINE void
free_funp P1(struct funp *, fp)
{
    fp->ref--;
    if (fp->ref > 0) {
	return;
    }
#ifdef NEW_FUNCTIONS
    free_object(fp->owner, "free_funp");
    if (fp->type == FP_CALL_OTHER)
	free_svalue(&fp->f.obj, "free_funp");
    else if (fp->type == FP_FUNCTIONAL 
	  || fp->type == (FP_FUNCTIONAL | FP_NOT_BINDABLE)) {
	fp->f.a.prog->p.i.func_ref--;
	if (fp->f.a.prog->p.i.func_ref == 0 && fp->f.a.prog->p.i.ref == 0)
	    deallocate_program(fp->f.a.prog);
    }
    free_svalue(&fp->args, "free_funp");
#else
    free_svalue(&fp->obj, "free_funp");
    free_svalue(&fp->fun, "free_funp");
#endif
    FREE(fp);
}

INLINE void f_and()
{
    if (sp->type == T_POINTER && (sp - 1)->type == T_POINTER) {
	sp--;
	sp->u.vec = intersect_array((sp + 1)->u.vec, sp->u.vec);
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
    struct svalue *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to &=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to &=\n");
    sp->u.number = argp->u.number &= sp->u.number;
}

INLINE void
f_div_eq()
{
    struct svalue *argp = (sp--)->u.lvalue;

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
	    if (sp->type & T_NUMBER){
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
	    if ((--sp)->type & T_NUMBER){
		sp->u.number = sp->u.number == (sp+1)->u.real;
	    }
	    else {
		sp->u.number = sp->u.real == (sp+1)->u.number;
		sp->type = T_NUMBER;
	    }
	    return;
	}
	
    case T_POINTER:
	{
	    i = (sp-1)->u.vec == sp->u.vec;
	    free_vector((sp--)->u.vec);
	    free_vector(sp->u.vec);
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
    struct svalue *argp;


    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to <<=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to <<=\n");
    sp->u.number = argp->u.number <<= sp->u.number;
}

INLINE void
f_mod_eq()
{
    struct svalue *argp;

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
    struct svalue *argp = (sp--)->u.lvalue;

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
	    if (sp->type & T_NUMBER){
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
	    struct mapping *m = compose_mapping(argp->u.map, sp->u.map,0);
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
            if ((--sp)->type & T_NUMBER){
                sp->u.number = sp->u.number != (sp+1)->u.real;
	    }
            else {
                sp->u.number = sp->u.real != (sp+1)->u.number;
                sp->type = T_NUMBER;
	    }
            return;
	}

    case T_POINTER:
        {
            i = (sp-1)->u.vec != sp->u.vec;
	    free_vector((sp--)->u.vec);
	    free_vector(sp->u.vec);
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
    struct svalue *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to |=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to |=\n");
    sp->u.number = argp->u.number |= sp->u.number;
}

INLINE void
f_parse_command()
{
    struct svalue *arg;
    struct svalue *fp;
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
    CHECK_TYPES(&arg[1], T_OBJECT | T_POINTER, 2, F_PARSE_COMMAND);
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
f_range(int code)
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
                put_constant_string("");
                return;
            }

            if (to >= len - 1){
                put_malloced_string(string_copy(res + from, "f_range"));
            } else {
                char *tmp;
                tmp = DXALLOC(to - from + 2, TAG_STRING, "f_range");
                strncpy(tmp, res + from, to - from + 1);
                tmp[to - from + 1] = '\0';
                put_malloced_string(tmp);
            }
            free_string_svalue(sp + 2);
            break;
        }
        case T_BUFFER:
        {
            struct buffer *rbuf = sp->u.buf;

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
                struct buffer *nbuf = allocate_buffer(to - from + 1);
                memcpy(nbuf->item, rbuf->item + from, to - from + 1);
                free_buffer(rbuf);
                put_buffer(nbuf);
            }
            break;
        }

        case T_POINTER:
        {
            struct vector *v = sp->u.vec;
            to = (--sp)->u.number;
            if (code & 0x01) to = v->size - to;
            from = (--sp)->u.number;
            if (code & 0x10) from = v->size - from;
            put_vector(slice_array(v, from, to));
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
            put_malloced_string(string_copy(res + from, "f_extract_range"));
            free_string_svalue(sp + 1);
            break;
        }
        case T_BUFFER:
        {
            struct buffer *rbuf = sp->u.buf;
            struct buffer *nbuf;


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
            nbuf = allocate_buffer(len - from);
            memcpy(nbuf->item, rbuf->item + from, len - from);
            free_buffer(rbuf);
            put_buffer(nbuf);
            break;
        }

        case T_POINTER:
        {
            struct vector *v = sp->u.vec;
            from = (--sp)->u.number;
            if (code) from = v->size - from;
            put_vector(slice_array(v, from, v->size - 1));
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
    struct svalue *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to >>=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to >>=\n");
    sp->u.number = argp->u.number >>= sp->u.number;
}

INLINE void
f_sub_eq()
{
    struct svalue *argp = (sp--)->u.lvalue;

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
	    if (sp->type & T_NUMBER){
		sp->type = T_REAL;
		sp->u.real = argp->u.real -= sp->u.number;
	    } else sp->u.real = argp->u.number -= sp->u.real;
	    break;
	}

	case T_POINTER:
	{
	    sp->u.vec = argp->u.vec = subtract_array(argp->u.vec, sp->u.vec);
	    sp->u.vec->ref++;
	    break;
	}

	default:
	{
	    if (!(sp->type & (T_NUMBER|T_REAL|T_POINTER))) error("Bad right type to -=\n");
	    else if (!(argp->type & (T_NUMBER|T_REAL|T_POINTER))) error("Bad left type to -=\n");
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
#define SW_BREAK	3
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
    unsigned short offset, break_adr;
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
    COPY_SHORT(&break_adr, pc + SW_BREAK);
    if (--break_sp == start_of_switch_stack)
	fatal("Switch stack underflow!\n");
    *break_sp = break_adr;
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
		pc = current_prog->p.i.program + offset;
		return;
	    }
	} else {
	    bad_argument(sp, T_STRING | T_NUMBER, 1, F_SWITCH);
	}
    } else {			/* Integer table, check type */
	CHECK_TYPES(sp, T_NUMBER, 1, F_SWITCH);
	s = sp->u.number;
	i = (int) pc[0] & 0xf;
    }
    pop_stack();
    end_tab = current_prog->p.i.program + break_adr;
    /*
     * i is the table size as a power of 2.  Tells us where to start
     * searching.  i==14 is a special case.
     */
    if (i >= 14)
	if (i == 14) {
	    /* fastest switch format : lookup table */
	    l = current_prog->p.i.program + offset;
	    COPY_INT(&d, end_tab - 4);
	    /* d is minimum value - see if in range or not */
	    if (s >= d && l + (s = (s - d) * sizeof(short)) < (end_tab - 4)) {
		COPY_SHORT(&offset, &l[s]);
		if (offset) {
		    pc = current_prog->p.i.program + offset;
		    return;
		}
	    }
	    /* default */
	    COPY_SHORT(&offset, pc + SW_DEFAULT);
	    pc = current_prog->p.i.program + offset;
	    return;
	} else
	    fatal("unsupported switch table format.\n");

    /*
     * l - current entry we are looking at. 
     * d - size to add/subtract from l each iteration. 
     * s - key we're looking for 
     * r - key l is pointing at
     */
    l = current_prog->p.i.program + offset + off_tab[i];
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
			    l = current_prog->p.i.program + offset +
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
			    l = current_prog->p.i.program + offset +
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
		l = current_prog->p.i.program + offset + (s - r) * sizeof(short);
		COPY_SHORT(&offset, l);
	    }
	    if (offset <= 1) {
		COPY_SHORT(&offset, l + L_ADDR);
		if (!offset) {
		    /* start of range with lookup table */
		    l = current_prog->p.i.program + offset;
		    COPY_SHORT(&offset, l);
		}		/* else normal range, offset is correct */
	    }
	    break;
	}
    }
    /* now do jump */
    pc = current_prog->p.i.program + offset;
}

int simul_efun_is_loading = 0;

void
call_simul_efun P2(unsigned short, index, int, num_arg)
{
    struct function *funp;
    extern struct object *simul_efun_ob;
    extern char *simul_efun_file_name;
    extern struct function **simuls;

    if (current_object->flags & O_DESTRUCTED) {	/* No external calls allowed */
	pop_n_elems(num_arg);
	push_undefined();
	return;
    }

    if (!simul_efun_ob || (simul_efun_ob->flags & O_DESTRUCTED)) {
	if (simul_efun_is_loading)
	    error("Attempt to call a simul_efun while compiling the simul_efun object.\n");
	(void) load_object(simul_efun_file_name, 0);
	/* if it didn't load we're in trouble. */
	if (!simul_efun_ob || (simul_efun_ob->flags & O_DESTRUCTED))
	    error("No simul_efun object for simul_efun.\n");
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
	push_control_stack(FRAME_FUNCTION, simuls[index]);
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
    } else error("Function is no longer a simul_efun.");
}

INLINE void
f_simul_efun()
{
    unsigned short index;

    LOAD_SHORT(index, pc);
    call_simul_efun(index, EXTRACT_UCHAR(pc++));
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
    struct svalue *argp;

    if ((argp = sp->u.lvalue)->type != T_NUMBER)
	error("Bad left type to ^=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to ^=\n");
    sp->u.number = argp->u.number ^= sp->u.number;
}

#ifdef NEW_FUNCTIONS
INLINE struct funp *
make_efun_funp P2(int, opcode, svalue *, args)
{
    struct funp *fp;
    int i=0;
    
    fp = ALLOCATE(struct funp, TAG_FUNP, "make_efun_funp");
    fp->owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->type = FP_EFUN;
    
#ifdef NEEDS_CALL_EXTRA
    if (opcode >= 0xff) {
	fp->f.opcodes[i++] = F_CALL_EXTRA;
	fp->f.opcodes[i++] = opcode - 0xff;
    } else { 
#endif
	fp->f.opcodes[i++] = opcode;
#ifdef NEEDS_CALL_EXTRA
    }
#endif
    
    if (instrs[opcode].min_arg != instrs[opcode].max_arg)
	fp->f.opcodes[i++] = 0; /* filled in when evaluated */
    
    fp->f.opcodes[i++] = F_RETURN;
    
    if (args) 
	assign_svalue_no_free(&fp->args, args);
    else
	fp->args.type = T_NUMBER;
    
    fp->ref = 1;
    return fp;
}

INLINE struct funp *
make_lfun_funp P2(int, index, svalue *, args)
{
    struct funp *fp;
    
    fp = ALLOCATE(struct funp, TAG_FUNP, "make_efun_funp");
    fp->owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->type = FP_LOCAL | FP_NOT_BINDABLE;
    
    fp->f.index = index + function_index_offset;
    
    if (args) 
	assign_svalue_no_free(&fp->args, args);
    else
	fp->args.type = T_NUMBER;
    
    fp->ref = 1;
    return fp;
}

INLINE struct funp *
make_simul_funp P2(int, index, svalue *, args)
{
    struct funp *fp;
    
    fp = ALLOCATE(struct funp, TAG_FUNP, "make_efun_funp");
    fp->owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->type = FP_SIMUL;
    
    fp->f.index = index;
    
    if (args) 
	assign_svalue_no_free(&fp->args, args);
    else
	fp->args.type = T_NUMBER;
    
    fp->ref = 1;
    return fp;
}

INLINE struct funp *
make_functional_funp P5(short, num_arg, short, num_local, short, len, svalue *, args, int, flag)
{
    struct funp *fp;
    
    fp = ALLOCATE(struct funp, TAG_FUNP, "make_functional_funp");
    fp->owner = current_object;
    add_ref( current_object, "make_functional_funp" );
    fp->type = FP_FUNCTIONAL + flag;

    current_prog->p.i.func_ref++;
    
    fp->f.a.prog = current_prog;
    fp->f.a.offset = pc - current_prog->p.i.program;
    fp->f.a.num_args = num_arg;
    fp->f.a.num_locals = num_local;
    fp->f.a.fio = function_index_offset;
    fp->f.a.vio = variable_index_offset;
    pc += len;
    
    if (args) {
	assign_svalue_no_free(&fp->args, args);
	if (args->type == T_POINTER)
	    fp->f.a.num_args += args->u.vec->size;
    } else
	fp->args.type = T_NUMBER;

    fp->ref = 1;
    return fp;
}
#endif

INLINE struct funp *
make_funp P2(struct svalue *,sobj, struct svalue *,sfun)
{
    struct funp *fp;
    
    fp = ALLOCATE(struct funp, TAG_FUNP, "make_funp");
#ifdef NEW_FUNCTIONS
    fp->owner = current_object;
    add_ref( current_object, "make_funp" );
    fp->type = FP_CALL_OTHER;
    
    assign_svalue_no_free(&fp->f.obj, sobj);
    assign_svalue_no_free(&fp->args, sfun);
#else
#ifndef NO_UIDS
    if (current_object) {
	if (current_object->euid) {
	    fp->euid = current_object->euid;
	} else {
	    fp->euid = current_object->uid;
	}
    } else {
	fp->euid = NULL;
    }
#endif
    assign_svalue_no_free(&fp->obj, sobj);
    assign_svalue_no_free(&fp->fun, sfun);
#endif
    fp->ref = 1;
    return fp;
}

INLINE void
push_refed_funp P1(struct funp *, fp)
{
    sp++;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
}

INLINE void
push_funp P1(struct funp *, fp)
{
    sp++;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
    fp->ref++;
}

INLINE void
f_function_constructor()
{
    struct funp *fp;
    int kind;
    unsigned short index;

    kind = EXTRACT_UCHAR(pc++);

    switch (kind) {
#ifdef NEW_FUNCTIONS
    case FP_EFUN:
	kind = EXTRACT_UCHAR(pc++);
#ifdef NEEDS_CALL_EXTRA
	if (kind == F_CALL_EXTRA) {
	    kind = EXTRACT_UCHAR(pc) + 0xff;
	    pc++;
	}
#endif
	fp = make_efun_funp(kind, sp);
	pop_stack();
	break;
#endif
    case FP_CALL_OTHER:
	fp = make_funp(sp - 1, sp);
	pop_2_elems();
	break;
    case FP_CALL_OTHER | FP_THIS_OBJECT:
	if (current_object->flags & O_DESTRUCTED)
	    push_number(0);
	else
	    push_object(current_object);
	fp = make_funp(sp, sp - 1);
	pop_2_elems();
	break;
#ifdef NEW_FUNCTIONS
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
#endif
    default:
	fatal("Tried to make unknown type of function pointer.\n");
    }
    push_refed_funp(fp);
}

#ifdef NEW_FUNCTIONS
void
f_apply PROT((void))
{
    struct svalue *v;
    int num;
    struct funp *f;
    struct vector *vec;

    if ((sp - 1)->type != T_FUNCTION) {
	pop_stack();
	return;
    }
    if (current_object->flags & O_DESTRUCTED) {
	pop_2_elems();
	push_undefined();
	return;
    }

    vec = (sp--)->u.vec;
    f = sp->u.fp;
    if (vec->ref == 1) {
	transfer_push_some_svalues(vec->item, num = vec->size);
	free_empty_vector(vec);
    } else {
	push_some_svalues(vec->item, num = vec->size);
	vec->ref--;
    }
    v = call_function_pointer(f, num);
    free_funp(f);
    assign_svalue_no_free(sp, v);
}

INLINE void
f_evaluate PROT((void))
{
    struct svalue *v;
    struct svalue *arg = sp - st_num_arg + 1;

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
#else
INLINE void
f_evaluate()
{
    struct svalue *obj, *fun;
    struct funp *tmp;

    CHECK_TYPES(sp, T_FUNCTION, 1, F_EVALUATE);
    obj = &sp->u.fp->obj;
    fun = &sp->u.fp->fun;
    tmp = sp->u.fp;
    sp--;			/* don't free the funp here since that would
				 * also free obj and fun */
    if ((obj->type == T_OBJECT) && (obj->u.ob->flags & O_DESTRUCTED)) {
	assign_svalue(obj, &const0n);
	push_null();
    } else {
	push_svalue(obj);
    }
    push_svalue(fun);
    free_funp(tmp);		/* go ahead and free it here since the pushes
				 * make it ok */
}
#endif

INLINE void
f_sscanf()
{
    struct svalue *fp;
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













