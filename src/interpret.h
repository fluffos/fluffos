/* interpret.h */

#ifndef _INTERPRET_H
#define _INTERPRET_H

#include "uid.h"

/* Trace defines */
#ifdef TRACE
#  define TRACE_CALL 1
#  define TRACE_CALL_OTHER 2
#  define TRACE_RETURN 4
#  define TRACE_ARGS 8
#  define TRACE_EXEC 16
#  define TRACE_HEART_BEAT 32
#  define TRACE_APPLY 64
#  define TRACE_OBJNAME 128
#  ifdef LPC_TO_C
#    define TRACE_COMPILED 256
#    define TRACE_LPC_EXEC 512
#  endif
#  define TRACETST(b) (command_giver->interactive->trace_level & (b))
#  define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#  define TRACEHB (current_heart_beat == 0 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))
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
#ifdef DEBUG
#define T_FREED         0x400
#endif

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

#ifdef NEW_FUNCTIONS
struct afp {
    char *start;
    short num_args;
};

struct funp {
    unsigned short ref;
    struct object *owner;
    short type;                 /* ORIGIN_* is used */
    union {
	struct svalue obj;      /* for call_other function pointers */
	short index;            /* lfuns and simul_efuns */
	/* worst case here is F_CALL_EXTRA F_EFUN <num_arg> F_RETURN */
	unsigned char opcodes[4]; /* for efun function pointers */
	struct afp a;           /* for functionals */
    } f;
    struct svalue args;         /* includes the function for call_other */
};
#else
struct funp {	
    unsigned short ref;
    struct svalue obj, fun;
#ifndef NO_UIDS
    userid_t *euid;
#endif
};
#endif

union string_or_func {
    struct funp *f;
    char *s;
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

/* for apply_master_ob */
#define MASTER_APPROVED(x) (((x)==(struct svalue *)-1) || ((x) && (((x)->type != T_NUMBER) || (x)->u.number))) 

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))
#define IS_NULL(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_NULLVALUE) && ((x)->u.number == 0)))

#define CHECK_TYPES(val, t, arg, inst) \
  if (!((val)->type & (t))) bad_argument(val, t, arg, inst);

/* macro calls */
#ifndef LPC_TO_C
#define call_program(prog, offset) \
        eval_instruction(prog->p.i.program + offset)
#define call_absolute(offset) eval_instruction(offset)
#endif

#ifdef DEBUG
#define free_svalue(x,y) int_free_svalue(x,y)
#else
#define free_svalue(x,y) int_free_svalue(x)
#endif

#endif				/* _INTERPRET_H */
