/* interpret.h */

#ifndef INTERPRET_H
#define INTERPRET_H

#include "uid.h"
#include "mudlib_stats.h"

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

#ifdef HAS_UNSIGNED_CHAR
#define EXTRACT_UCHAR(p) (*(unsigned char *)(p))
#else
#define EXTRACT_UCHAR(p) (*p < 0 ? *p + 0x100 : *p)
#endif				/* HAS_UNSIGNED_CHAR */

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
    void (*error_handler) PROT((void));
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
#define T_ERROR_HANDLER 0x400
#ifdef DEBUG
#define T_FREED         0x800
#endif

#define T_ANY T_STRING|T_NUMBER|T_POINTER|T_OBJECT|T_MAPPING|T_FUNCTION| \
	T_REAL|T_BUFFER

/* values for subtype field of svalue struct */
#define STRING_MALLOC	0x1	/* Allocated by malloc() */
#define STRING_CONSTANT	0x2	/* Do not has to be freed at all */
#define STRING_SHARED	0x4	/* Allocated by the shared string library */
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

#define push_svalue(x) assign_svalue_no_free(++sp, x)
#define put_number(x) sp->type = T_NUMBER;sp->subtype = 0;sp->u.number = (x);
#define put_object(x) sp->type = T_OBJECT; sp->u.ob = (x);
#define put_unrefed_object(x, y) sp->type = T_OBJECT; sp->u.ob = (x); \
   add_ref((x), y);
#define put_constant_string(x) sp->type = T_STRING;sp->subtype = STRING_CONSTANT; \
                sp->u.string = (x);
#define put_malloced_string(x) sp->type = T_STRING; sp->subtype = STRING_MALLOC; \
                sp->u.string = (x);
#define put_vector(x) sp->type = T_POINTER; sp->u.vec = (x);
#define put_shared_string(x) sp->type = T_STRING; sp->subtype = STRING_SHARED; \
                sp->u.string = (x);

extern struct program *current_prog;
extern short caller_type;
extern char *pc;
extern struct svalue *sp;
extern struct svalue *fp;
extern short *break_sp;
extern struct svalue catch_value;
extern struct control_stack control_stack[30];
extern struct control_stack *csp;
extern int too_deep_error;
extern int max_eval_error;
extern unsigned int apply_low_call_others;
extern unsigned int apply_low_cache_hits;
extern unsigned int apply_low_slots_used;
extern unsigned int apply_low_collisions;
extern int function_index_offset;
extern int master_ob_is_loading;
extern int simul_efun_is_loading;
extern struct function fake_func;
extern struct program fake_prog;

/* with LPC_TO_C off, these are defines using eval_instruction */
#ifdef LPC_TO_C
void call_program PROT((struct program *, int));


void call_absolute PROT((char *));
#endif
void eval_instruction PROT((char *p));
INLINE void assign_svalue PROT((struct svalue *, struct svalue *));
INLINE void assign_svalue_no_free PROT((struct svalue *, struct svalue *));
INLINE void copy_some_svalues PROT((struct svalue *, struct svalue *, int));
INLINE void transfer_push_some_svalues PROT((struct svalue *, int));
INLINE void push_some_svalues PROT((struct svalue *, int));
#ifdef DEBUG
INLINE void int_free_svalue PROT((struct svalue *, char *));
#else
INLINE void int_free_svalue PROT((struct svalue *));
#endif
INLINE void free_string_svalue PROT((struct svalue *));
INLINE void free_some_svalues PROT((struct svalue *, int));
INLINE void push_object PROT((struct object *));
INLINE void push_number PROT((int));
INLINE void push_real PROT((double));
INLINE void push_undefined PROT((void));
INLINE void push_null PROT((void));
INLINE void push_string PROT((char *, int));
INLINE void push_svalue PROT((struct svalue *));
INLINE void push_vector PROT((struct vector *));
INLINE void push_refed_vector PROT((struct vector *));
INLINE void push_buffer PROT((struct buffer *));
INLINE void push_refed_buffer PROT((struct buffer *));
INLINE void push_mapping PROT((struct mapping *));
INLINE void push_refed_mapping PROT((struct mapping *));
INLINE void push_malloced_string PROT((char *));
INLINE void push_constant_string PROT((char *));
INLINE void pop_stack PROT((INLINE void));
INLINE void pop_n_elems PROT((int));
INLINE void pop_2_elems PROT((void));
INLINE void pop_3_elems PROT((void));
void remove_object_from_stack PROT((struct object *));
void setup_fake_frame PROT((struct funp *));
void remove_fake_frame PROT((void));

char *type_name PROT((int c));
void bad_arg PROT((int, int));
void bad_argument PROT((struct svalue *, int, int, int));
void check_for_destr PROT((struct vector *));
int is_static PROT((char *, struct object *));
int apply_low PROT((char *, struct object *, int));
struct svalue *apply PROT((char *, struct object *, int, int));
struct svalue *call_function_pointer PROT((struct funp *, int));
struct svalue *safe_apply PROT((char *, struct object *, int, int));
struct vector *call_all_other PROT((struct vector *, char *, int));
char *function_exists PROT((char *, struct object *));
void call_function PROT((struct program *, struct function *));
struct svalue *apply_master_ob PROT((char *, int));
struct svalue *safe_apply_master_ob PROT((char *, int));
int assert_master_ob_loaded PROT((char *, char *));

void translate_absolute_line PROT((int, unsigned short *, int *, int *));
char *add_slash PROT((char *));
int strpref PROT((char *, char *));
struct vector *get_svalue_trace PROT((void));
void do_trace PROT((char *, char *, char *));
char *dump_trace PROT((int));
void opcdump PROT((char *));
int inter_sscanf PROT((struct svalue *, struct svalue *, struct svalue *, int));
char * get_line_number_if_any PROT((void));
void get_line_number_info PROT((char **, int *));
void get_version PROT((char *));
void reset_machine PROT((int));

#ifndef NO_SHADOWS
int validate_shadowing PROT((struct object *));
#endif

#ifdef LAZY_RESETS
void try_reset PROT((struct object *));
#endif

void push_pop_error_context PROT((int));
void pop_control_stack PROT((void));
INLINE struct function *setup_new_frame PROT((struct function *));
INLINE void push_control_stack PROT((struct function *));

#ifdef DEBUG
void check_a_lot_ref_counts PROT((struct program *));
#endif

#endif				/* _INTERPRET_H */
