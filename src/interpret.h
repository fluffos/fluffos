/* interpret.h */

#ifndef _INTERPRET_H
#define _INTERPRET_H

#include "uid.h"

#ifdef LPC_TO_C
#define lpcyylex yylex
#define lpcyytext yytext
#define lpcyylval yylval
#define lpcyyerror yyerror
#endif

union u {
    char *string;
    int number;
    float real;
    struct buffer *buf;
    struct object *ob;
    struct vector *vec;
    struct mapping *map;
    struct funp *fp;
    struct svalue *lvalue;
    unsigned char *lvalue_byte;
};

/*
 * The value stack element.
 * If it is a string, then the way that the string has been allocated
 * differently, which will affect how it should be freed.
 */
typedef struct svalue {
    short type;
    short subtype;
    union u u;
}      svalue;

/* These flags are used by load_object() */
#define LO_DONT_RESET   0x1
#define LO_SAVE_OUTPUT  0x2

/* values for type field of svalue struct */
#define T_INVALID	0x0
#define T_LVALUE	0x1
#define T_NUMBER	0x2
#define T_STRING	0x4
#define T_POINTER	0x8
#define T_OBJECT	0x10
#define T_MAPPING	0x20
#define T_FUNCTION      0x40
#define T_REAL          0x80
#define T_BUFFER        0x100
#define T_LVALUE_BYTE   0x200	/* byte-sized lvalue */
#define T_FREED         0x400
#define T_ANY T_STRING|T_NUMBER|T_POINTER|T_OBJECT|T_MAPPING|T_FUNCTION| \
	T_REAL|T_BUFFER

/* values for subtype field of svalue struct */
#define STRING_MALLOC	0x0	/* Allocated by malloc() */
#define STRING_CONSTANT	0x1	/* Do not has to be freed at all */
#define STRING_SHARED	0x2	/* Allocated by the shared string library */
#define T_UNDEFINED     0x4	/* undefinedp() returns true */
#define T_NULLVALUE     0x8	/* nullp() returns true */
#define T_REMOTE        0x10	/* remote object (subtype of object) */
#define T_ERROR         0x20	/* error code */

struct funp {
    unsigned short ref;
    struct svalue obj, fun;
    userid_t *euid;
};

struct vector {
    unsigned short ref;
#ifdef DEBUG
    int extra_ref;
#endif
    unsigned short size;
#ifndef NO_MUDLIB_STATS
    statgroup_t stats;		/* creator of the array */
#endif
    struct svalue item[1];
};

#define ALLOC_VECTOR(nelem) \
    (struct vector *)DXALLOC(sizeof (struct vector) + \
	  sizeof(struct svalue) * (nelem - 1), 121, "ALLOC_VECTOR")

/*
 * Control stack element.
 * 'prog' is usually same as 'ob->prog' (current_object), except when
 * when the current function is defined by inheritance.
 * The pointer, csp, will point to the values that will be used at return.
 */
struct control_stack {
#ifdef PROFILE_FUNCTIONS
    unsigned long entry_secs, entry_usecs;
#endif
    struct object *ob;		/* Current object */
    struct object *prev_ob;	/* Save previous object */
    struct program *prog;	/* Current program */
    int num_local_variables;	/* Local + arguments */
    char *pc;
    struct svalue *fp;
    int extern_call;		/* Flag if evaluator should return */
    struct function *funp;	/* Only used for tracebacks */
    int function_index_offset;	/* Used when executing functions in inherited
				 * programs */
    int variable_index_offset;	/* Same */
    short *break_sp;
    short caller_type;		/* was this a locally called function? */
};

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))
#define IS_NULL(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_NULLVALUE) && ((x)->u.number == 0)))

#define CHECK_TYPES(val, t, arg, inst) \
  if ((t) && !((val)->type & (t))) bad_argument(val, t, arg, inst);

#endif				/* _INTERPRET_H */
