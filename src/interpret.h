/* interpret.h */

#ifndef _INTERPRET_H
#define _INTERPRET_H


union u {
    char *string;
    int number;
    struct object *ob;
    struct vector *vec;
    struct mapping *map;
    struct funp    *fp;
    struct svalue *lvalue;
};

/*
 * The value stack element.
 * If it is a string, then the way that the string has been allocated
 * differently, which will affect how it should be freed.
 */
struct svalue {
    short type;
    short subtype;
    union u u;
};

/* values for type field of svalue struct */
#define T_INVALID	0x0
#define T_LVALUE	0x1
#define T_NUMBER	0x2
#define T_STRING	0x4
#define T_POINTER	0x8
#define T_OBJECT	0x10
#define T_MAPPING	0x20
#define T_FUNCTION  0x40
#define T_ANY       T_STRING|T_NUMBER|T_POINTER|T_OBJECT|T_MAPPING|T_FUNCTION

/* values for subtype field of svalue struct */
#define STRING_MALLOC	0x0	   /* Allocated by malloc() */
#define STRING_CONSTANT	0x1	   /* Do not has to be freed at all */
#define STRING_SHARED	0x2	   /* Allocated by the shared string library */
#define T_UNDEFINED     0x4    /* undefinedp() returns true */
#define T_NULLVALUE     0x8    /* nullp() returns true */
#define T_REMOTE        0x10   /* remote object (subtype of object) */

struct funp {
	struct svalue obj, fun;
	short ref;
};

struct vector {
    short size;
    short ref;
#ifdef DEBUG
    int extra_ref;
#endif
    statgroup_t stats;      /* creator of the array */
    struct svalue item[1];
};

#define ALLOC_VECTOR(nelem) \
    (struct vector *)DXALLOC(sizeof (struct vector) + \
			    sizeof(struct svalue) * (nelem - 1), 8, "ALLOC_VECTOR")

struct lnode_def;

/*
 * Control stack element.
 * 'prog' is usually same as 'ob->prog' (current_object), except when
 * when the current function is defined by inheritance.
 * The pointer, csp, will point to the values that will be used at return.
 */
struct control_stack {
    struct object *ob;		/* Current object */
    struct object *prev_ob;	/* Save previous object */
    struct program *prog;	/* Current program */
    int num_local_variables;	/* Local + arguments */
    char *pc;
    struct svalue *fp;
    int extern_call;		/* Flag if evaluator should return */
    struct function *funp;	/* Only used for tracebacks */
    int function_index_offset;	/* Used when executing functions in inherited
				   programs */
    int variable_index_offset;	/* Same */
    short *break_sp;
};

#define IS_ZERO(x) (!(x) || (((x)->type == T_NUMBER) && ((x)->u.number == 0)))
#define IS_UNDEFINED(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_UNDEFINED) && ((x)->u.number == 0)))
#define IS_NULL(x) (!(x) || (((x)->type == T_NUMBER) && \
	((x)->subtype == T_NULLVALUE) && ((x)->u.number == 0)))

#endif /* _INTERPRET_H */
