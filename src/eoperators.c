/*
	eoperators.c: this file contains all of the operators called from
	inside eval_instruction() in interpret.c.
*/

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifndef LATTICE
#include <sys/time.h>
#else
#include <time.h>
#endif
#if !defined(LATTICE) && !defined(OS2)
#include <sys/ioctl.h>
#include <netdb.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifdef LATTICE
#include <nsignal.h>
#include <stdlib.h>
#endif
#ifndef LATTICE
#include <memory.h>
#endif
#include <setjmp.h>

#include "lint.h"
#include "eoperators.h"
#include "interpret.h"
#include "buffer.h"
#include "mapping.h"
#include "object.h"
#include "exec.h"
#include "efun_protos.h"
#include "comm.h"
#include "opcodes.h"
#include "switch.h"
#include "stralloc.h"
#include "debug.h"

/* Needed these from interpret.c. But what for? -SH */
#define TRACE_CALL_OTHER 2
#define TRACE_RETURN 4
#define TRACE_ARGS 8
#define TRACE_HEART_BEAT 32
#define TRACETST(b) (command_giver->interactive->trace_level & (b))
#define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#define TRACEHB (current_heart_beat == 0 \
	 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))

extern int d_flag;
extern char *pc;
extern int tracedepth;
extern int current_time;
extern char *last_verb;
extern struct svalue *fp;	/* Pointer to first argument. */
extern int function_index_offset;	/* Needed for inheritance */
extern int variable_index_offset;	/* Needed for inheritance */
extern struct object *previous_ob;
extern struct object *master_ob;
extern userid_t *backbone_uid;
extern struct svalue const0, const1, const0u, const0n;
extern struct object *current_heart_beat, *current_interactive;
extern short *break_sp;		/* Points to address to branch to at next
				 * F_BREAK			 */
extern struct control_stack *csp;	/* Points to last element pushed */

/* Points to value of last push. */
extern struct svalue *sp;

extern int eval_cost;

static void push_funp PROT((struct funp *));

INLINE void f_and P2(int, num_arg, int, instruction)
{
    if (sp->type == T_POINTER && (sp - 1)->type == T_POINTER) {
	extern struct vector *intersect_array
	       PROT((struct vector *, struct vector *));

	sp--;
	sp->u.vec = intersect_array((sp + 1)->u.vec, sp->u.vec);
	return;
    }
    CHECK_TYPES(sp - 1, T_NUMBER, 1, instruction);
    CHECK_TYPES(sp, T_NUMBER, 2, instruction);
    sp--;
    sp->u.number &= (sp + 1)->u.number;
}

INLINE void
f_and_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to &=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to &=\n");
    sp->u.number = argp->u.number &= sp->u.number;
}

INLINE void
f_parse_command P2(int, num_arg, int, instruction)
{
    struct svalue *arg;
    struct svalue *fp;
    int i;

    /*
     * get number of lvalue args
     */
    num_arg = EXTRACT_UCHAR(pc);
    pc++;

    /*
     * type checking on first three required parameters to parse_command()
     */
    arg = sp - 2;
    CHECK_TYPES(&arg[0], T_STRING, 1, instruction);
    CHECK_TYPES(&arg[1], T_OBJECT | T_POINTER, 2, instruction);
    CHECK_TYPES(&arg[2], T_STRING, 3, instruction);

    if (arg[1].type == T_POINTER)
	check_for_destr(arg[1].u.vec);

    /*
     * allocate stack frame for rvalues and return value (number of matches);
     * perform some stack manipulation;
     */
    fp = sp;
    sp += num_arg + 1;
    arg = sp;
    *(arg--) = *(fp--);		/* move pattern to top of stack */
    *(arg--) = *(fp--);		/* move source object to just below the
				 * pattern */
    *(arg) = *(fp);		/* move source string just below the object */

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
    pop_n_elems(3);

    /*
     * save return value on stack
     */
    fp->type = T_NUMBER;
    fp->u.number = i;
}

INLINE void
f_div_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    sp--;
    if (argp->type == T_NUMBER) {
	if (sp->type == T_NUMBER) {
	    if (sp->u.number == 0)
		error("Division by 0nn\n");
	    sp->u.number = argp->u.number /= sp->u.number;
	} else if (sp->type == T_REAL) {
	    if (sp->u.real == 0.)
		error("Division by 0.nr\n");
	    sp->u.real = argp->u.number /= sp->u.real;
	} else {
	    error("Bad right type to /=\n");
	}
    } else if (argp->type == T_REAL) {
	if (sp->type == T_NUMBER) {
	    if (sp->u.number == 0)
		error("Division by 0rn\n");
	    sp->u.real = argp->u.real /= sp->u.number;
	    sp->type = T_REAL;
	} else if (sp->type == T_REAL) {
	    if (sp->u.real == 0.)
		error("Division by 0.rr\n");
	    sp->u.real = argp->u.real /= sp->u.real;
	} else {
	    error("Bad right type to /=\n");
	}
    } else {
	error("Bad left type to /=\n");
    }
}

INLINE void
f_eq P2(int, num_arg, int, instruction)
{
    int i;

    switch (sp->type) {
    case T_NUMBER:
	if ((sp - 1)->type == T_NUMBER) {
	    i = ((sp - 1)->u.number == sp->u.number);
	} else if ((sp - 1)->type == T_REAL) {
	    i = ((sp - 1)->u.real == sp->u.number);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    case T_REAL:
	if ((sp - 1)->type == T_NUMBER) {
	    i = ((sp - 1)->u.number == sp->u.real);
	} else if ((sp - 1)->type == T_REAL) {
	    i = ((sp - 1)->u.real == sp->u.real);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    case T_POINTER:
	if ((sp - 1)->type == T_POINTER) {
	    i = ((sp - 1)->u.vec == sp->u.vec);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    case T_STRING:
	if ((sp - 1)->type == T_STRING) {
	    i = (strcmp((sp - 1)->u.string, sp->u.string) == 0);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    case T_OBJECT:
	if ((sp - 1)->type == T_OBJECT) {
	    i = ((sp - 1)->u.ob == sp->u.ob);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    case T_MAPPING:
	if ((sp - 1)->type == T_MAPPING) {
	    i = ((sp - 1)->u.map == sp->u.map);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    return;
	}
	break;
    default:
	i = 0;
	break;
    }
    pop_n_elems(2);
    push_number(i);
}

INLINE void
f_lsh P2(int, num_arg, int, instruction)
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, instruction);
    CHECK_TYPES(sp, T_NUMBER, 2, instruction);
    sp--;
    sp->u.number <<= (sp + 1)->u.number;
}

INLINE void
f_lsh_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to <<=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to <<=\n");
    sp->u.number = argp->u.number <<= sp->u.number;
}

INLINE void
f_mod_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to %=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to %=\n");
    if (sp->u.number == 0)
	error("Division by 0\n");
    sp->u.number = argp->u.number %= sp->u.number;
}

INLINE void
f_mult_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    sp--;
    if (argp->type == T_NUMBER) {
	if (sp->type == T_NUMBER) {
	    sp->u.number = argp->u.number *= sp->u.number;
	} else if (sp->type == T_REAL) {
	    sp->u.real = argp->u.number *= sp->u.real;
	} else {
	    error("Bad right type to *=\n");
	}
    } else if (argp->type == T_REAL) {
	if (sp->type == T_NUMBER) {
	    sp->u.real = argp->u.real *= sp->u.number;
	    sp->type = T_REAL;
	} else if (sp->type == T_REAL) {
	    sp->u.real = argp->u.real *= sp->u.real;
	} else {
	    error("Bad right type to *=\n");
	}
    } else if (argp->type == T_MAPPING) {
	if (sp->type == T_MAPPING) {
	    struct mapping *m;

	    m = compose_mapping(argp->u.map, sp->u.map);
	    pop_stack();
	    push_mapping(m);
	    assign_svalue(argp, sp);
	} else {
	    error("Bad right type to *=\n");
	}
    } else {
	error("Bad left type to *=\n");
    }
}

INLINE void
f_ne P2(int, num_arg, int, instruction)
{
    int i;

    switch (sp->type) {
    case T_NUMBER:
	if ((sp - 1)->type == T_NUMBER) {
	    i = ((sp - 1)->u.number != sp->u.number);
	} else if ((sp - 1)->type == T_REAL) {
	    i = ((sp - 1)->u.real != sp->u.number);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_REAL:
	if ((sp - 1)->type == T_NUMBER) {
	    i = ((sp - 1)->u.number != sp->u.real);
	} else if ((sp - 1)->type == T_REAL) {
	    i = ((sp - 1)->u.real != sp->u.real);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_POINTER:
	if ((sp - 1)->type == T_POINTER) {
	    i = ((sp - 1)->u.vec != sp->u.vec);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_STRING:
	if ((sp - 1)->type == T_STRING) {
	    i = (strcmp((sp - 1)->u.string, sp->u.string) != 0);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_OBJECT:
	if ((sp - 1)->type == T_OBJECT) {
	    i = ((sp - 1)->u.ob != sp->u.ob);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_MAPPING:
	if ((sp - 1)->type == T_MAPPING) {
	    i = ((sp - 1)->u.map != sp->u.map);
	} else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
	break;
    case T_FUNCTION:
	if ((sp - 1)->type == T_FUNCTION)
	    i = ((sp - 1)->u.fp != sp->u.fp);
	else {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    return;
	}
    default:
	i = 1;
	break;
    }
    pop_n_elems(2);
    push_number(i);
}

INLINE void
f_or P2(int, num_arg, int, instruction)
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, instruction);
    CHECK_TYPES(sp, T_NUMBER, 2, instruction);
    sp--;
    sp->u.number |= (sp + 1)->u.number;
}

INLINE void
f_or_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to |=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to |=\n");
    sp->u.number = argp->u.number |= sp->u.number;
}

INLINE void
f_range P2(int, num_arg, int, instruction)
{
    if (sp[-1].type != T_NUMBER)
	error("Start of range [ .. ] interval must be a number.\n");
    if (sp[0].type != T_NUMBER)
	error("End of range [ .. ] interval must be a number.\n");
    if (sp[-2].type == T_STRING) {
	int len, from, to;
	char *res;

	len = strlen(sp[-2].u.string);
	from = sp[-1].u.number;
	if (from < 0)
	    from = len + from;
	if (from < 0)
	    from = 0;
	if (from >= len) {
	    pop_n_elems(3);
	    push_string("", STRING_CONSTANT);
	    return;
	}
	to = sp[0].u.number;
	if (to < 0)
	    to = len + to;
	if (to < 0)
	    to = 0;
	if (to < from) {
	    pop_n_elems(3);
	    push_string("", STRING_CONSTANT);
	    return;
	}
	if (to >= len)
	    to = len - 1;
	if (to == len - 1) {
	    res = string_copy(sp[-2].u.string + from);
	    pop_n_elems(3);
	    push_malloced_string(res);
	    return;
	}
	res = DXALLOC(to - from + 2, 37, "f_range");
	strncpy(res, sp[-2].u.string + from, to - from + 1);
	res[to - from + 1] = '\0';
	pop_n_elems(3);
	push_malloced_string(res);
    } else if (sp[-2].type == T_BUFFER) {
	int len, from, to;
	struct buffer *rbuf;

	len = sp[-2].u.buf->size;
	from = sp[-1].u.number;
	if (from < 0)
	    from = len + from;
	if (from >= len) {
	    pop_n_elems(3);
	    push_buffer(null_buffer());
	    return;
	}
	to = sp[0].u.number;
	if (to < 0)
	    to = len + to;
	if (to < from) {
	    pop_n_elems(3);
	    push_buffer(null_buffer());
	    return;
	}
	if (to >= len)
	    to = len - 1;
	rbuf = allocate_buffer(to - from + 1);
	rbuf->ref--;
	memcpy(rbuf->item, sp[-2].u.buf->item + from, to - from + 1);
	pop_n_elems(3);
	push_buffer(rbuf);
    } else if (sp[-2].type == T_POINTER) {
	struct vector *v;

	v = slice_array(sp[-2].u.vec, sp[-1].u.number, sp[0].u.number);
	pop_n_elems(3);
	if (v) {
	    push_vector(v);
	    v->ref--;		/* Will make ref count == 1 */
	} else
	    push_number(0);
    } else
	error("Bad argument to [ .. ] range operator.\n");
}

INLINE void
f_rsh P2(int, num_arg, int, instruction)
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, instruction);
    CHECK_TYPES(sp, T_NUMBER, 2, instruction);
    sp--;
    sp->u.number >>= (sp + 1)->u.number;
}

INLINE void
f_rsh_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to >>=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to >>=\n");
    sp->u.number = argp->u.number >>= sp->u.number;
}

INLINE void
f_sub_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    sp--;
    switch (argp->type) {
    case T_NUMBER:
	if (sp->type == T_NUMBER) {
	    argp->u.number -= sp->u.number;
	} else if (sp->type == T_REAL) {
	    argp->u.number -= sp->u.real;
	} else {
	    error("Left hand side of -= is a number (or zero); right side is not a number.\n");
	}
	break;
    case T_REAL:
	if (sp->type == T_NUMBER) {
	    argp->u.real -= sp->u.number;
	} else if (sp->type == T_REAL) {
	    argp->u.real -= sp->u.real;
	} else {
	    error("Left hand side of -= is a number (or zero); right side is not a number.\n");
	}
	break;
    case T_POINTER:
	if (sp->type == T_POINTER) {
	    struct vector *subtract_array PROT((struct vector *, struct vector *));
	    struct vector *v, *w;

	    v = sp->u.vec;
	    if (v->ref > 1) {
		v = slice_array(v, 0, v->size - 1);
		free_vector(sp->u.vec);
	    }
	    w = subtract_array(argp->u.vec, v);
	    free_vector(argp->u.vec);
	    free_vector(v);	/* no longer freed in subtract_array() */
	    argp->u.vec = w;
	} else {
	    error("Left hand side of -= is an array, right hand side is not.\n");
	}
	break;
    default:
	error("Bad left type to -=\n");
    }
    assign_svalue_no_free(sp, argp);
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
 *           Table is followed by 1 long that is minimum key value.
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

#define ENTRY_SIZE	6

/* offsets used for range (L_ for lower member, U_ for upper member) */
#define L_LOWER	0
#define L_TYPE	4
#define L_UPPER	6
#define L_ADDR	10
#define U_LOWER	-6
#define U_TYPE	-2
#define U_UPPER	0
#define U_ADDR	4

#define COPY_SHORT(dst, src) \
    { ((char *)(dst))[0] = ((char *)(src))[0]; \
      ((char *)(dst))[1] = ((char *)(src))[1]; }
#define COPY_LONG(dst, src) \
    { ((char *)(dst))[0] = ((char *)(src))[0]; \
      ((char *)(dst))[1] = ((char *)(src))[1]; \
      ((char *)(dst))[2] = ((char *)(src))[2]; \
      ((char *)(dst))[3] = ((char *)(src))[3]; }

INLINE void
f_switch P2(int, num_arg, int, instruction)
{
    extern char *findstring PROT((char *));
    unsigned short offset, break_adr;
    int d, s = 0, r;
    int i;
    char *l, *end_tab;
    static unsigned short off_tab[] =
    {
	0 * ENTRY_SIZE, 1 * ENTRY_SIZE, 3 * ENTRY_SIZE, 7 * ENTRY_SIZE, 15 * ENTRY_SIZE,
	31 * ENTRY_SIZE, 63 * ENTRY_SIZE, 127 * ENTRY_SIZE, 255 * ENTRY_SIZE,
	511 * ENTRY_SIZE, 1023 * ENTRY_SIZE, 2047 * ENTRY_SIZE, 4095 * ENTRY_SIZE,
	8191 * ENTRY_SIZE,
    };

    COPY_SHORT(&offset, pc + SW_TABLE);
    COPY_SHORT(&break_adr, pc + SW_BREAK);
    *--break_sp = break_adr;
    if ((i = EXTRACT_UCHAR(pc) >> 4) != 0xf) {	/* String table, find correct
						 * key */
	if (sp->type == T_NUMBER && !sp->u.number) {
	    /* special case: 0 as a string */
	    s = (int) ZERO_AS_STR_CASE_LABEL;
	} else if (sp->type == T_STRING) {
	    switch (sp->subtype) {
	    case STRING_SHARED:
		s = (int) sp->u.string;
		break;
	    default:
		s = (int) findstring(sp->u.string);
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
	    bad_argument(sp, T_STRING | T_NUMBER, 1, instruction);
	}
    } else {			/* Integer table, check type */
	CHECK_TYPES(sp, T_NUMBER, 1, instruction);
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
	    COPY_LONG(&d, end_tab - 4);
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
     * l - current entry we are looking at. d - size to add/subtract from l
     * each iteration. s - key we're looking for r - key l is pointing at
     */
    l = current_prog->p.i.program + offset + off_tab[i];
    d = (int) (off_tab[i] + ENTRY_SIZE) >> 1;
    if (d < ENTRY_SIZE)
	d = 0;
    for (;;) {
	COPY_LONG(&r, l);
	if (s < r) {
	    if (d < ENTRY_SIZE) {
		/* test if entry is part of a range */
		/* Don't worry about reading from F_BREAK (byte before table) */
		COPY_SHORT(&offset, l + U_TYPE);
		if (offset <= 1) {
		    COPY_LONG(&r, l + U_LOWER);
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
		/* d >= ENTRY_SIZE */
		l -= d;
		d >>= 1;
	    }
	} else if (s > r) {
	    if (d < ENTRY_SIZE) {
		/* test if entry is part of a range */
		COPY_SHORT(&offset, l + L_TYPE);
		if (offset <= 1) {
		    COPY_LONG(&r, l + L_UPPER);
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
	    } else {		/* d >= ENTRY_SIZE */
		l += d;
		/* if table isn't a power of 2 in size, fix us up */
		while (l >= end_tab) {
		    d >>= 1;
		    if (d < ENTRY_SIZE) {
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
		COPY_LONG(&r, l + U_LOWER);
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

INLINE void
f_xor P2(int, num_arg, int, instruction)
{
    CHECK_TYPES((sp - 1), T_NUMBER, 1, instruction);
    CHECK_TYPES(sp, T_NUMBER, 2, instruction);
    sp--;
    sp->u.number ^= (sp + 1)->u.number;
}

INLINE void
f_xor_eq P2(int, num_arg, int, instruction)
{
    struct svalue *argp;

    if (sp->type != T_LVALUE)
	bad_arg(1, instruction);
    argp = sp->u.lvalue;
    if (argp->type != T_NUMBER)
	error("Bad left type to ^=\n");
    if ((--sp)->type != T_NUMBER)
	error("Bad right type to ^=\n");
    sp->u.number = argp->u.number ^= sp->u.number;
}

INLINE struct funp *
     make_funp P2(struct svalue *, sob, struct svalue *, sfun)
{
    struct funp *fp;

    fp = (struct funp *) DMALLOC(sizeof(struct funp), 38, "make_funp");
    if (current_object) {
	if (current_object->euid) {
	    fp->euid = current_object->euid;
	} else {
	    fp->euid = current_object->uid;
	}
    } else {
	fp->euid = NULL;
    }
    assign_svalue_no_free(&fp->obj, sob);
    assign_svalue_no_free(&fp->fun, sfun);
    fp->ref = 1;
    return fp;
}

static INLINE void
push_funp P1(struct funp *, fp)
{
    fp->ref++;
    sp++;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
}

INLINE void
free_funp P1(struct funp *, fp)
{
    fp->ref--;
    if (fp->ref > 0) {
	return;
    }
    free_svalue(&fp->obj);
    free_svalue(&fp->fun);
    FREE(fp);
}

INLINE void
f_function_constructor P2(int, num_arg, int, instruction)
{
    struct funp *fp;

    fp = make_funp(sp - 1, sp);
    pop_n_elems(2);
    push_funp(fp);
    fp->ref--;
}

INLINE void
f_this_function_constructor P2(int, num_arg, int, instruction)
{
    struct funp *fp;

    if (current_object->flags & O_DESTRUCTED)
	push_number(0);
    else
	push_object(current_object);
/* Note: The stack order is reversed here compared to f_function_constructor */
    fp = make_funp(sp, sp - 1);
    pop_n_elems(2);
    push_funp(fp);
    fp->ref--;
}

INLINE void
f_function_split P2(int, num_arg, int, instruction)
{
    struct svalue *obj, *fun;
    struct funp *tmp;

    CHECK_TYPES(sp, T_FUNCTION, 1, instruction);
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

INLINE void
f_aggregate_assoc P2(int, num_arg, int, instruction)
{
    struct mapping *m;
    unsigned short num;

    ((char *) &num)[0] = pc[0];
    ((char *) &num)[1] = pc[1];
    pc += 2;
    m = load_mapping_from_aggregate(sp - num + 1, num);
    pop_n_elems((int) num);
    push_mapping(m);
    m->ref--;
}

INLINE void
f_aggregate P2(int, num_arg, int, instruction)
{
    struct vector *v;
    unsigned short num;
    int i;

    ((char *) &num)[0] = pc[0];
    ((char *) &num)[1] = pc[1];
    pc += 2;
    v = allocate_array((int) num);
    /*
     * transfer svalues in reverse...popping stack as we go
     */
    for (i = num; i--; sp--)
	v->item[i] = *sp;
    push_vector(v);
    v->ref--;
}

INLINE void
f_sscanf P2(int, num_arg, int, instruction)
{
    struct svalue *fp;
    int i;

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
    pop_n_elems(2);

    /*
     * save number of matches on stack
     */
    fp->type = T_NUMBER;
    fp->u.number = i;
}
