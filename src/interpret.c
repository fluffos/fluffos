#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "uid.h"
#include "patchlevel.h"
#include "backend.h"
#include "simul_efun.h"
#include "debug.h"
#include "efunctions.h"
#include "lex.h"
#include "functab_tree.h"
#include "eoperators.h"
#include "sprintf.h"
#include "swap.h"
#include "comm.h"
#include "port.h"

#ifdef OPCPROF
#include "opc.h"

static int opc_eoper[BASE];
#endif

#if defined(RUSAGE) && !defined(LATTICE)	/* Defined in config.h */
#include <sys/resource.h>
#ifdef SunOS_5
#include <sys/rusage.h>
#endif
#ifdef sun
extern int getpagesize();
#endif
#ifndef RUSAGE_SELF
#define RUSAGE_SELF	0
#endif
#endif

static char *type_names[] = {
    "int",
    "string",
    "array",
    "object",
    "mapping",
    "function",
    "float",
    "buffer",
};
#define TYPE_CODES_END 0x200
#define TYPE_CODES_START 0x2

int master_ob_is_loading;
#ifndef NO_UIDS
extern userid_t *backbone_uid;
#endif
extern int max_cost;
extern int call_origin;

static INLINE void push_indexed_lvalue PROT((int));
static struct svalue *find_value PROT((int));
#ifdef TRACE
static void do_trace_call PROT((struct function *));
#endif
static void break_point PROT((void));
static INLINE void do_loop_cond PROT((void));
static void do_catch PROT((char *, unsigned short));
#ifdef OPCPROF
static int cmpopc PROT((opc_t *, opc_t *));
#endif
#ifdef DEBUG
int last_instructions PROT((void));
#endif
static float _strtof PROT((char *, char **));
static struct svalue *sapply PROT((char *, struct object *, int));
static char *get_line_number PROT((char *, struct program *));
#ifdef TRACE_CODE
static char *get_arg PROT((int, int));
#endif

int inter_sscanf PROT((struct svalue *, struct svalue *, struct svalue *, int));
struct program *current_prog;
short int caller_type;
static int tracedepth;

/*
 * Inheritance:
 * An object X can inherit from another object Y. This is done with
 * the statement 'inherit "file";'
 * The inherit statement will clone a copy of that file, call reset
 * in it, and set a pointer to Y from X.
 * Y has to be removed from the linked list of all objects.
 * All variables declared by Y will be copied to X, so that X has access
 * to them.
 *
 * If Y isn't loaded when it is needed, X will be discarded, and Y will be
 * loaded separately. X will then be reloaded again.
 */

/*
 * These are the registers used at runtime.
 * The control stack saves registers to be restored when a function
 * will return. That means that control_stack[0] will have almost no
 * interesting values, as it will terminate execution.
 */
char *pc;			/* Program pointer. */
struct svalue *fp;		/* Pointer to first argument. */

struct svalue *sp;

int function_index_offset;	/* Needed for inheritance */
int variable_index_offset;	/* Needed for inheritance */
int st_num_arg;

static struct svalue start_of_stack[EVALUATOR_STACK_SIZE];
struct svalue *end_of_stack = start_of_stack + EVALUATOR_STACK_SIZE - 5;
static short switch_stack[RUNTIME_SWITCH_STACK_SIZE];
short *break_sp = switch_stack + RUNTIME_SWITCH_STACK_SIZE;
/* Points to address to branch to at next F_BREAK */
short *start_of_switch_stack = switch_stack;

/* Used to throw an error to a catch */
struct svalue catch_value =
{T_NUMBER};

/* used by routines that want to return a pointer to an svalue */
struct svalue apply_ret_value = {T_NUMBER};

struct control_stack control_stack[MAX_TRACE];
struct control_stack *csp;	/* Points to last element pushed */

int too_deep_error = 0, max_eval_error = 0;
#define STACK_CHECK if (++sp >= end_of_stack) do { \
		    too_deep_error = 1; error("stack overflow\n"); } \
                    while (0)

void get_version P1(char *, buff)
{
    sprintf(buff, "MudOS %s", PATCH_LEVEL);
}

/*
 * Information about assignments of values:
 *
 * There are three types of l-values: Local variables, global variables
 * and vector elements.
 *
 * The local variables are allocated on the stack together with the arguments.
 * the register 'frame_pointer' points to the first argument.
 *
 * The global variables must keep their values between executions, and
 * have space allocated at the creation of the object.
 *
 * Elements in vectors are similar to global variables. There is a reference
 * count to the whole vector, that states when to deallocate the vector.
 * The elements consists of 'struct svalue's, and will thus have to be freed
 * immediately when over written.
 */

/*
 * Push an object pointer on the stack. Note that the reference count is
 * incremented.
 * A destructed object must never be pushed onto the stack.
 */
INLINE
void push_object P1(struct object *, ob)
{
    STACK_CHECK;
    sp->type = T_OBJECT;
    sp->u.ob = ob;
    add_ref(ob, "push_object");
}

char * type_name P1(int, c) { 
    int j = 0; 
    int limit = TYPE_CODES_START;

    do {
	if (c & limit) return type_names[j];
	j++;
    } while (!((limit <<= 1) & TYPE_CODES_END));
    /* Oh crap.  Take some time and figure out what we have. */
    if (c == T_INVALID) return "*invalid*";
    if (c == T_LVALUE) return "*lvalue*";
    if (c == T_LVALUE_BYTE) return "*lvalue_byte*";
    if (c == T_LVALUE_RANGE) return "*lvalue_range*";
    if (c == T_ERROR_HANDLER) return "*error_handler*";
    IF_DEBUG(if (c == T_FREED) return "*freed*");
    return "*unknown*";
}

/*
 * May current_object shadow object 'ob' ? We rely heavily on the fact that
 * function names are pointers to shared strings, which means that equality
 * can be tested simply through pointer comparison.
 */
#ifndef NO_SHADOWS
int validate_shadowing P1(struct object *, ob)
{
    int i, j;
    struct program *shadow = current_object->prog, *victim = ob->prog;
    struct svalue *ret;

    if (current_object->shadowing)
	error("shadow: Already shadowing.\n");
    if (current_object->shadowed)
	error("shadow: Can't shadow when shadowed.\n");
    if (current_object->super)
	error("shadow: The shadow must not reside inside another object.\n");
    if (ob == master_ob)
	error("shadow: cannot shadow the master object.\n");
    if (ob->shadowing)
	error("shadow: Can't shadow a shadow.\n");
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    /*
     * Want to iterate over the smaller function table, and use binary search
     * on the larger one for faster operation, ie O(m lg n), where m < n,
     * versus O(m n)
     */
    if ((int) shadow->p.i.num_functions < (int) victim->p.i.num_functions) {
	for (i = 0; i < (int) shadow->p.i.num_functions; i++) {
	    j = lookup_function(victim->p.i.functions, victim->p.i.tree_r,
				shadow->p.i.functions[i].name);
	    if (j != -1 && victim->p.i.functions[j].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->p.i.functions[j].name);
	}
    } else {
	for (i = 0; i < (int) victim->p.i.num_functions; i++) {
	    j = lookup_function(shadow->p.i.functions, shadow->p.i.tree_r,
				victim->p.i.functions[i].name);
	    if (j != -1 && victim->p.i.functions[i].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->p.i.functions[i].name);
	}
    }
#else
    for (i = 0; i < (int) shadow->p.i.num_functions; i++) {
	for (j = 0; j < (int) victim->p.i.num_functions; j++) {
	    if (shadow->p.i.functions[i].name != victim->p.i.functions[j].name)
		continue;
	    if (victim->p.i.functions[j].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->p.i.functions[j].name);
	}
    }
#endif
    push_object(ob);
    ret = apply_master_ob(APPLY_VALID_SHADOW, 1);
    if (!(ob->flags & O_DESTRUCTED) && MASTER_APPROVED(ret)) {
	return 1;
    }
    return 0;
}
#endif

/*
 * Push a number on the value stack.
 */
INLINE void
push_number P1(int, n)
{
    STACK_CHECK;
    sp->type = T_NUMBER;
    sp->subtype = 0;
    sp->u.number = n;
}

INLINE void
push_real P1(double, n)
{
    STACK_CHECK;
    sp->type = T_REAL;
    sp->u.real = n;
}

/*
 * Push undefined (const0u) onto the value stack.
 */
INLINE
void push_undefined()
{
    STACK_CHECK;
    *sp = const0u;
}

/*
 * Push null (const0n) onto the value stack.
 */
INLINE
void push_null()
{
    STACK_CHECK;
    *sp = const0n;
}

INLINE void push_nulls(int num)
{
    if (sp + num >= end_of_stack) { too_deep_error = 1; error("stack overflow\n"); }
    while (num--) *++sp = const0n;
}

/*
 * Push a string on the value stack.
 */
INLINE
void push_string P2(char *, p, int, type)
{
    STACK_CHECK;
    sp->type = T_STRING;
    sp->subtype = type;
    switch (type) {
    case STRING_MALLOC:
	sp->u.string = string_copy(p, "push_string");
	break;
    case STRING_SHARED:
	sp->u.string = make_shared_string(p);
	break;
    case STRING_CONSTANT:
	sp->u.string = p;
	break;
    }
}

/*
 * Get address to a valid global variable.
 */
#ifdef DEBUG
static INLINE struct svalue *find_value P1(int, num)
{
    DEBUG_CHECK2(num >= (int) current_object->prog->p.i.num_variables,
		 "Illegal variable access %d(%d).\n",
		 num, current_object->prog->p.i.num_variables);
    return &current_object->variables[num];
}
#else
#define find_value(num) (&current_object->variables[num])
#endif

INLINE void
free_string_svalue P1(struct svalue *, v)
{
    switch (v->subtype) {
	case STRING_MALLOC:
	FREE(v->u.string);
	break;
    case STRING_SHARED:
	free_string(v->u.string);
	break;
    }
}

/*
 * Free the data that an svalue is pointing to. Not the svalue
 * itself.
 * Use the free_svalue() define to call this
 */
#ifdef DEBUG
INLINE void int_free_svalue P2(struct svalue *, v, char *, tag)
#else
INLINE void int_free_svalue P1(struct svalue *, v)
#endif
{
    switch (v->type) {
	case T_STRING:
	switch (v->subtype) {
	    case STRING_MALLOC:
	    FREE(v->u.string);
	    break;
	case STRING_SHARED:
	    free_string(v->u.string);
	    break;
	}
	break;
    case T_OBJECT:
	free_object(v->u.ob, "free_svalue");
	break;
    case T_POINTER:
	free_vector(v->u.vec);
	break;
    case T_BUFFER:
	free_buffer(v->u.buf);
	break;
    case T_MAPPING:
	free_mapping(v->u.map);
	break;
    case T_FUNCTION:
	free_funp(v->u.fp);
	break;
    case T_ERROR_HANDLER:
        (*v->u.error_handler)();
	break;
#ifdef DEBUG
    case T_FREED:
	fatal("T_FREED svalue freed.  Previously freed by %s.\n", v->u.string);
	break;
#endif
    }
    IF_DEBUG(v->type = T_FREED);
    IF_DEBUG(v->u.string = tag);
}

/*
 * Free several svalues, and free up the space used by the svalues.
 * The svalues must be sequentially located.
 */
INLINE void free_some_svalues P2(struct svalue *, v, int, num)
{
    while (num--)
	free_svalue(v + num, "free_some_svalues");
    FREE(v);
}

/*
 * Prepend a slash in front of a string.
 */
char *add_slash P1(char *, str)
{
    char *tmp;

    tmp = DXALLOC(strlen(str) + 2, TAG_STRING, "add_slash");
    *tmp = '/';
    strcpy(tmp + 1, str);
    return tmp;
}

/*
 * Assign to a svalue.
 * This is done either when element in vector, or when to an identifier
 * (as all identifiers are kept in a vector pointed to by the object).
 */

INLINE void assign_svalue_no_free P2(struct svalue *, to, struct svalue *, from)
{
    DEBUG_CHECK(from == 0, "Attempt to assign_svalue() from a null ptr.\n");
    DEBUG_CHECK(to == 0, "Attempt to assign_svalue() to a null ptr.\n");
    *to = *from;

    switch (from->type) {
    case T_STRING:
	switch (from->subtype) {
	case STRING_MALLOC:	/* No idea to make the string shared */
	    to->u.string = string_copy(from->u.string, "assign_svalue_no_free");
	    break;
	case STRING_CONSTANT:	/* Good idea to make it shared */
	    to->subtype = STRING_SHARED;
	    to->u.string = make_shared_string(from->u.string);
	    break;
	case STRING_SHARED:	/* It already is shared */
	    to->u.string = ref_string(from->u.string);
	    break;
#ifdef DEBUG
	default:
	    fatal("Bad string type %d\n", from->subtype);
#endif
	}
	break;
    case T_OBJECT:
	add_ref(to->u.ob, "asgn to var");
	break;
    case T_POINTER:
	to->u.vec->ref++;
	break;
    case T_MAPPING:
	to->u.map->ref++;
	break;
    case T_FUNCTION:
	to->u.fp->ref++;
	break;
    case T_BUFFER:
	to->u.buf->ref++;
	break;
    }
}

INLINE void assign_svalue P2(struct svalue *, dest, struct svalue *, v)
{
    /* First deallocate the previous value. */
    free_svalue(dest, "assign_svalue");
    assign_svalue_no_free(dest, v);
}

INLINE void push_some_svalues P2(struct svalue *, v, int, num)
{
    if (sp + num >= end_of_stack) { too_deep_error = 1; error("stack overflow\n"); }
    while (num--) push_svalue(v++);
}

/*
 * Copies an array of svalues to another location, which should be
 * free space.
 */
INLINE void copy_some_svalues P3(struct svalue *, dest, struct svalue *, v, int, num)
{
    while (num--)
	assign_svalue_no_free(dest+num, v+num);
}

INLINE void transfer_push_some_svalues P2(struct svalue *, v, int, num)
{
    if (sp + num >= end_of_stack) { too_deep_error = 1; error("stack overflow\n"); }
    memcpy(sp + 1, v, num * sizeof(struct svalue));
    sp += num;
}

/*
 * Pop the top-most value of the stack.
 * Don't do this if it is a value that will be used afterwards, as the
 * data may be sent to FREE(), and destroyed.
 */
INLINE void pop_stack()
{
    DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
    free_svalue(sp--, "pop_stack");
}

struct svalue glb = { T_LVALUE_BYTE };

/*
 * Compute the address of an array element.
 */
static INLINE void push_indexed_lvalue P1(int, code)
{
    int ind;
    struct svalue *lv;

    if (sp->type == T_LVALUE){
        lv = sp->u.lvalue;
        if (!code && lv->type & T_MAPPING){
            sp--;
            if (!(lv = find_for_insert(lv->u.map, sp, 0)))
                mapping_too_large();
            free_svalue(sp, "push_indexed_lvalue: 1");
            sp->type = T_LVALUE;
            sp->u.lvalue = lv;
            return;
	}
	
        if (!((--sp)->type & T_NUMBER))
            error("Illegal type of index\n");

        ind = sp->u.number;

        switch(lv->type){
#ifndef NEW_FUNCTIONS
	    case T_FUNCTION:
	    {
                if (code) ind = 2  - ind;
                if (ind > 1 || ind < 0)
                    error("Function variables may only be indexed with 0 or 1.\n");
                sp->type = T_LVALUE;
                sp->u.lvalue = ind ? &lv->u.fp->fun : &lv->u.fp->obj;
                break;
	    }
#endif
	    case T_STRING:
	    {
                char *tmp = lv->u.string;
                int len = SVALUE_STRLEN(lv);

                if (code) ind = len - ind;
                if (ind >= len || ind < 0)
                    error("Index out of bounds in string index lvalue.\n");
                if (lv->subtype != STRING_MALLOC){
                    tmp = string_copy(tmp, "push_indexed_lvalue: 1");
                    if (lv->subtype & STRING_SHARED) free_string(lv->u.string);
                    lv->u.string = tmp;
                    lv->subtype = STRING_MALLOC;
		  }
                sp->type = T_LVALUE;
                sp->u.lvalue = &glb;
                glb.u.lvalue_byte = (unsigned char *)&tmp[ind];
                break;
	    }

	    case T_BUFFER:
	    {
                if (code) ind = lv->u.buf->size - ind;
                if (ind >= lv->u.buf->size || ind < 0)
                    error("Buffer index out of bounds.\n");
                sp->type = T_LVALUE;
                sp->u.lvalue = &glb;
                glb.u.lvalue_byte = &lv->u.buf->item[ind];
                break;
	    }
	    
	    case T_POINTER:
	    {
                if (code) ind = lv->u.vec->size - ind;
                if (ind >= lv->u.vec->size || ind < 0)
                    error("Array index out of bounds\n");
                sp->type = T_LVALUE;
                sp->u.lvalue = lv->u.vec->item + ind;
                break;
	    }

	    default:
                error("Indexing on illegal type.\n");
        }
    } else {
        /* It is now coming from (x <assign_type> y)[index]... = rhs */
        /* Where x is a _valid_ lvalue */
        /* Hence the reference to sp is at least 2 :) */

        if (!code && (sp->type & T_MAPPING)){
            if (!(lv = find_for_insert(sp->u.map, sp-1, 0)))
                mapping_too_large();
            sp->u.map->ref--;
            free_svalue(--sp, "push_indexed_lvalue: 2");
            sp->type = T_LVALUE;
            sp->u.lvalue = lv;
            return;
	}
	
        if (!((sp-1)->type & T_NUMBER))
            error("Illegal type of index\n");
	
        ind = (sp-1)->u.number;

        switch (sp->type){
#ifndef NEW_FUNCTIONS
	    case T_FUNCTION:
	    {
                if (code) ind = 2 - ind;
                if (ind > 1 || ind < 0)
                    error("Function variables may only be indexed with 0 or 1.\n");
                lv = ind ? &sp->u.fp->fun : &sp->u.fp->obj;
                sp->u.fp->ref--;
                (--sp)->type = T_LVALUE;
                sp->u.lvalue = lv;
                break;
	    }
#endif
	    case T_STRING:
	    {
                error("Illegal to make char lvalue from assigned string\n");
                break;
	    }

	    case T_BUFFER:
	    {
                if (code) ind = sp->u.buf->size - ind;
                if (ind >= sp->u.buf->size || ind < 0)
                    error("Buffer index out of bounds.\n");
                sp->u.buf->ref--;
                (--sp)->type = T_LVALUE;
                sp->u.lvalue = &glb;
                glb.u.lvalue_byte = (sp+1)->u.buf->item + ind;
                break;
	    }

	    case T_POINTER:
	    {
		if (code) ind = sp->u.vec->size - ind;
		if (ind >= sp->u.vec->size || ind < 0)
                    error("Array index out of bounds.\n");
                sp->u.vec->ref--;
                (--sp)->type = T_LVALUE;
                sp->u.lvalue = (sp+1)->u.vec->item + ind;
                break;
	    }

	    default:
	        error("Indexing on illegal type.\n");
        }
    }
}

static struct lvalue_range {
    int ind1, ind2, size;
    struct svalue *owner;
} glr;

static struct svalue glr_sv = { T_LVALUE_RANGE };

static INLINE void push_lvalue_range(int code)
{
    int ind1, ind2, size;
    struct svalue *lv;

    if (sp->type == T_LVALUE){
        switch((lv = glr.owner = sp->u.lvalue)->type){
            case T_POINTER:
                size = lv->u.vec->size;
                break;
		case T_STRING: {
                char *tmp = lv->u.string;
                size = SVALUE_STRLEN(lv);
                if (lv->subtype != STRING_MALLOC){
                    tmp = string_copy(tmp, "push_lvalue_range");
                    if (lv->subtype & STRING_SHARED) free_string(lv->u.string);
                    lv->u.string = tmp;
                    lv->subtype = STRING_MALLOC;
                }
                break;
            }
	    case T_BUFFER:
                size = lv->u.buf->size;
                break;
	    default: error("Range lvalue on illegal type\n");
        }
    } else error("Range lvalue on illegal type\n");

    if (!((--sp)->type & T_NUMBER)) error("Illegal 2nd index type to range lvalue\n");

    ind2 = (code & 0x01) ? (size - sp->u.number) : sp->u.number;
    if (++ind2 < 0 || (ind2 > size))
        error("The 2nd index to range lvalue must be >= -1 and < sizeof(indexed value)\n");


    if (!((--sp)->type & T_NUMBER)) error("Illegal 1st index type to range lvalue\n");
    ind1 = (code & 0x10) ? (size - sp->u.number) : sp->u.number;

    if (ind1 < 0 || ind1 > size)
        error("The 1st index to range lvalue must be >= 0 and <= sizeof(indexed value)\n");

    glr.ind1 = ind1;
    glr.ind2 = ind2;
    glr.size = size;
    sp->type = T_LVALUE;
    sp->u.lvalue = &glr_sv;
}

INLINE void copy_lvalue_range(struct svalue *from)
{
    int ind1, ind2, size, fsize;
    struct svalue *owner;

    ind1 = glr.ind1;
    ind2 = glr.ind2;
    size = glr.size;
    owner = glr.owner;

    switch(owner->type){
        case T_POINTER:
        {
            struct vector *fv, *dv;
            struct svalue *fptr, *dptr;
            if (from->type != T_POINTER) error("Illegal rhs to array range lvalue\n");

            fv = from->u.vec;
            fptr = fv->item;

            if ((fsize = fv->size) == ind2 - ind1){
                dptr = (owner->u.vec)->item + ind1;

                if (fv->ref == 1){
                    /* Transfer the svalues */
                    while (fsize--){
                        free_svalue(dptr, "copy_lvalue_range : 1");
                        *dptr++ = *fptr++;
                    }
                    free_empty_vector(fv);
                } else {
                    while (fsize--) assign_svalue(dptr++, fptr++);
                    fv->ref--;
                }
            } else {
                struct vector *old_dv = owner->u.vec;
                struct svalue *old_dptr = old_dv->item;

                /* Need to reallocate the array */
                dv = allocate_empty_array(size - ind2 + ind1 + fsize);
                dptr = dv->item;

                /* ind1 can range from 0 to sizeof(old_dv) */
                while (ind1--) assign_svalue_no_free(dptr++, old_dptr++);

                if (fv->ref == 1){
                    while (fsize--) *dptr++ = *fptr++;
                    free_empty_vector(fv);
                } else {
                    while (fsize--) assign_svalue_no_free(dptr++, fptr++);
                    fv->ref--;
                }

                /* ind2 can range from 0 to sizeof(old_dv) */
                old_dptr = old_dv->item + ind2;
                size -= ind2;

                while (size--) assign_svalue_no_free(dptr++, old_dptr++);
                free_vector(old_dv);

                owner->u.vec = dv;
            }
            break;
        }

        case T_STRING:
        {
            if (from->type != T_STRING) error("Illegal rhs to string range lvalue.\n");

            if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1){
                /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
                /* because both of them can only range from 0 to len */

                strncpy(owner->u.string + ind1, from->u.string, fsize);
            } else {
                char *tmp, *dstr = owner->u.string;

                owner->u.string = tmp = DXALLOC(size - ind2 + ind1 + fsize + 1, TAG_STRING, "copy_lvalue_range");
                if (ind1 >= 1){
                    strncpy(tmp, dstr, ind1);
                    tmp += ind1;
                }
                strcpy(tmp, from->u.string);
                tmp += fsize;

                size -= ind2;
                if (size >= 1){
                    strncpy(tmp, dstr + ind2, size);
                    *(tmp + size) = 0;
                }
                FREE(dstr);
            }
            free_string_svalue(from);
            break;
        }

        case T_BUFFER:
        {
            if (from->type != T_BUFFER) error("Illegal rhs to buffer range lvalue.\n");

            if ((fsize = from->u.buf->size) == ind2 - ind1){
                memcpy((owner->u.buf)->item + ind1, from->u.buf->item, fsize);
            } else {
                struct buffer *b;
                unsigned char *old_item = (owner->u.buf)->item;
                unsigned char *new_item;

                b = allocate_buffer(size - ind2 + ind1 + fsize);
                new_item = b->item;
                if (ind1 >= 1){
                    memcpy(b->item, old_item, ind1);
                    new_item += ind1;
                }
                memcpy(new_item, from->u.buf, fsize);
                new_item += fsize;

                if ((size -= ind2) >= 1)
                    memcpy(new_item, old_item + ind2, size);
                free_buffer(owner->u.buf);
                owner->u.buf = b;
            }
            free_buffer(from->u.buf);
            break;
        }
    }
}
INLINE void assign_lvalue_range(struct svalue *from)
{
    int ind1, ind2, size, fsize;
    struct svalue *owner;

    ind1 = glr.ind1;
    ind2 = glr.ind2;
    size = glr.size;
    owner = glr.owner;

    switch(owner->type){
        case T_POINTER:
        {
            struct vector *fv, *dv;
            struct svalue *fptr, *dptr;
            if (from->type != T_POINTER) error("Illegal rhs to array range lvalue\n");

            fv = from->u.vec;
            fptr = fv->item;

            if ((fsize = fv->size) == ind2 - ind1){
                dptr = (owner->u.vec)->item + ind1;
                while (fsize--) assign_svalue(dptr++, fptr++);
            } else {
                struct vector *old_dv = owner->u.vec;
                struct svalue *old_dptr = old_dv->item;

                /* Need to reallocate the array */
                dv = allocate_empty_array(size - ind2 + ind1 + fsize);
                dptr = dv->item;

                /* ind1 can range from 0 to sizeof(old_dv) */
                while (ind1--) assign_svalue_no_free(dptr++, old_dptr++);

                while (fsize--) assign_svalue_no_free(dptr++, fptr++);

                /* ind2 can range from 0 to sizeof(old_dv) */
                old_dptr = old_dv->item + ind2;
                size -= ind2;

                while (size--) assign_svalue_no_free(dptr++, old_dptr++);
                free_vector(old_dv);

                owner->u.vec = dv;
            }
            break;
        }

        case T_STRING:
        {
            if (from->type != T_STRING) error("Illegal rhs to string range lvalue.\n");

            if ((fsize = SVALUE_STRLEN(from)) == ind2 - ind1){
                /* since fsize >= 0, ind2 - ind1 <= strlen(orig string) */
                /* because both of them can only range from 0 to len */

                strncpy(owner->u.string + ind1, from->u.string, fsize);
            } else {
                char *tmp, *dstr = owner->u.string;

                owner->u.string = tmp = DXALLOC(size - ind2 + ind1 + fsize + 1, TAG_STRING, "assign_lvalue_range");
                if (ind1 >= 1){
                    strncpy(tmp, dstr, ind1);
                    tmp += ind1;
                }
                strcpy(tmp, from->u.string);
                tmp += fsize;

                size -= ind2;
                if (size >= 1){
                    strncpy(tmp, dstr + ind2, size);
                    *(tmp + size) = 0;
                }
                FREE((char *) dstr);
            }
            break;
        }

        case T_BUFFER:
        {
            if (from->type != T_BUFFER) error("Illegal rhs to buffer range lvalue.\n");

            if ((fsize = from->u.buf->size) == ind2 - ind1){
                memcpy((owner->u.buf)->item + ind1, from->u.buf->item, fsize);
            } else {
                struct buffer *b;
                unsigned char *old_item = (owner->u.buf)->item;
                unsigned char *new_item;

                b = allocate_buffer(size - ind2 + ind1 + fsize);
                new_item = b->item;
                if (ind1 >= 1){
                    memcpy(b->item, old_item, ind1);
                    new_item += ind1;
                }
                memcpy(new_item, from->u.buf, fsize);
                new_item += fsize;

                if ((size -= ind2) >= 1)
                    memcpy(new_item, old_item + ind2, size);
                free_buffer(owner->u.buf);
                owner->u.buf = b;
            }
            break;
        }
    }
}

/*
 * Deallocate 'n' values from the stack.
 */
INLINE void
pop_n_elems P1(int, n)
{
    DEBUG_CHECK1(n < 0, "pop_n_elems: %d elements.\n", n);
    while (n--) {
	pop_stack();
    }
}

/*
 * Deallocate 2 values from the stack.
 */
INLINE void
pop_2_elems()
{
    free_svalue(sp--, "pop_2_elems");
    DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
    free_svalue(sp--, "pop_2_elems");
}

/*
 * Deallocate 3 values from the stack.
 */
INLINE void
pop_3_elems()
{
    free_svalue(sp--, "pop_3_elems");
    free_svalue(sp--, "pop_3_elems");
    DEBUG_CHECK(sp < start_of_stack, "Stack underflow.\n");
    free_svalue(sp--, "pop_3_elems");
}

void bad_arg P2(int, arg, int, instr)
{
    error("Bad argument %d to %s()\n", arg, get_f_name(instr));
}

void bad_argument P4(struct svalue *, val, int, type, int, arg, int, instr)
{
    char *buf;
    int flag = 0;
    int j = TYPE_CODES_START;
    int k = 0;

    buf = (char *) DMALLOC(300, TAG_TEMPORARY, "bad_argument");
    sprintf(buf, "Bad argument %d to %s%s\nExpected: ", arg, 
	    get_f_name(instr), (instr < BASE ? "" : "()"));

    do {
	if (type & j) {
	    if (flag) strcat(buf, " or ");
	    else flag = 1;
	    strcat(buf, type_names[k]);
	}
	k++;
    } while (!((j <<= 1) & TYPE_CODES_END));

    strcat(buf, " Got: ");
    svalue_to_string(val, &buf, 300, 0, 0, 0);
    strcat(buf, ".\n");
    error_needs_free(buf);
}

INLINE void
push_control_stack P2(int, frkind, void *, funp)
{
    if (csp == &control_stack[MAX_TRACE - 1]) {
	too_deep_error = 1;
	error("Too deep recursion.\n");
    }
    csp++;
    csp->caller_type = caller_type;
    csp->ob = current_object;
    csp->framekind = frkind;          /* Only used for tracebacks */
    csp->fr.func = (struct function *)funp;
    csp->prev_ob = previous_ob;
    csp->fp = fp;
    csp->prog = current_prog;
    /* csp->extern_call = 0; It is set by eval_instruction() */
    csp->pc = pc;
    csp->function_index_offset = function_index_offset;
    csp->variable_index_offset = variable_index_offset;
    csp->break_sp = break_sp;
#ifdef PROFILE_FUNCTIONS
    if (frkind == FRAME_FUNCTION) {
	get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
	((struct function *)funp)->calls++;
    }
#endif
}

/*
 * Pop the control stack one element, and restore registers.
 * extern_call must not be modified here, as it is used imediately after pop.
 */
void pop_control_stack()
{
    DEBUG_CHECK(csp == (control_stack - 1),
		"Popped out of the control stack\n");
#ifdef PROFILE_FUNCTIONS
    if (csp->framekind == FRAME_FUNCTION) {
	long secs, usecs, dsecs;

	get_cpu_times((unsigned long *) &secs, (unsigned long *) &usecs);
	dsecs = (((secs - csp->entry_secs) * 1000000)
		 + (usecs - csp->entry_usecs));
	csp->fr.func->self += dsecs;
	if (csp != control_stack) {
	    struct function *f;

	    if ( (csp - 1)->framekind == FRAME_FUNCTION) {
		(csp - 1)->fr.func->children += dsecs;
	    }
	}
    }
#endif
    current_object = csp->ob;
    current_prog = csp->prog;
    previous_ob = csp->prev_ob;
    caller_type = csp->caller_type;
    pc = csp->pc;
    fp = csp->fp;
    function_index_offset = csp->function_index_offset;
    variable_index_offset = csp->variable_index_offset;
    break_sp = csp->break_sp;
    csp--;
}

/*
 * Push a pointer to a vector on the stack. Note that the reference count
 * is incremented. Newly created vectors normally have a reference count
 * initialized to 1.
 */
INLINE void push_vector P1(struct vector *, v)
{
    v->ref++;
    sp++;
    sp->type = T_POINTER;
    sp->u.vec = v;
}

INLINE void push_refed_vector P1(struct vector *, v)
{
    sp++;
    sp->type = T_POINTER;
    sp->u.vec = v;
}

INLINE void
push_buffer P1(struct buffer *, b)
{
    b->ref++;
    sp++;
    sp->type = T_BUFFER;
    sp->u.buf = b;
}

INLINE void
push_refed_buffer P1(struct buffer *, b)
{
    sp++;
    sp->type = T_BUFFER;
    sp->u.buf = b;
}

/*
 * Push a mapping on the stack.  See push_vector(), above.
 */
INLINE void
push_mapping P1(struct mapping *, m)
{
    m->ref++;
    sp++;
    sp->type = T_MAPPING;
    sp->u.map = m;
}

INLINE void
push_refed_mapping P1(struct mapping *, m)
{
    sp++;
    sp->type = T_MAPPING;
    sp->u.map = m;
}

/*
 * Push a string on the stack that is already malloced.
 */
INLINE void push_malloced_string P1(char *, p)
{
    sp++;
    sp->type = T_STRING;
    sp->u.string = p;
    sp->subtype = STRING_MALLOC;
}

/*
 * Push a string on the stack that is already constant.
 */
INLINE
void push_constant_string P1(char *, p)
{
    sp++;
    sp->type = T_STRING;
    sp->u.string = p;
    sp->subtype = STRING_CONSTANT;
}

#ifdef TRACE
static void do_trace_call P1(struct function *, funp)
{
    do_trace("Call direct ", funp->name, " ");
    if (TRACEHB) {
	if (TRACETST(TRACE_ARGS)) {
	    int i;

	    add_vmessage(" with %d arguments: ", funp->num_arg);
	    for (i = funp->num_arg - 1; i >= 0; i--) {
		print_svalue(&sp[-i]);
		add_message(" ");
	    }
	}
	add_message("\n");
    }
}
#endif

/*
 * Argument is the function to execute. If it is defined by inheritance,
 * then search for the real definition, and return it.
 * There is a number of arguments on the stack. Normalize them and initialize
 * local variables, so that the called function is pleased.
 */
INLINE void setup_variables P3(int, actual, int, local, int, num_arg) {
    if (actual > num_arg) {
      /* Remove excessive arguments */
      pop_n_elems(actual - num_arg);
      push_nulls(local);
      fp = sp - (csp->num_local_variables = local + num_arg) + 1;
    } else {
      /* Correct number of arguments and local variables */
      push_nulls(local + num_arg - actual);
      fp = sp - (csp->num_local_variables = local + num_arg) + 1;
    }
}

INLINE struct function *
         setup_new_frame P1(struct function *, funp)
{
    function_index_offset = variable_index_offset = 0;
    while (funp->flags & NAME_INHERITED) {
	function_index_offset +=
	    current_prog->p.i.inherit[funp->offset].function_index_offset;
	variable_index_offset +=
	    current_prog->p.i.inherit[funp->offset].variable_index_offset;
	current_prog = current_prog->p.i.inherit[funp->offset].prog;
	funp = &current_prog->p.i.functions[funp->function_index_offset];
    }
    /* Remove excessive arguments */
    setup_variables(csp->num_local_variables, funp->num_local, funp->num_arg);
#ifdef TRACE
    tracedepth++;
    if (TRACEP(TRACE_CALL)) {
	do_trace_call(funp);
    }
#endif
    return funp;
}

INLINE struct function *setup_inherited_frame P1(struct function *, funp)
{
    while (funp->flags & NAME_INHERITED) {
        function_index_offset +=
            current_prog->p.i.inherit[funp->offset].function_index_offset;
        variable_index_offset +=
            current_prog->p.i.inherit[funp->offset].variable_index_offset;
        current_prog = current_prog->p.i.inherit[funp->offset].prog;
        funp = &current_prog->p.i.functions[funp->function_index_offset];
    }
    /* Remove excessive arguments */
    setup_variables(csp->num_local_variables, funp->num_local, funp->num_arg);
#ifdef TRACE
    tracedepth++;
    if (TRACEP(TRACE_CALL)) {
        do_trace_call(funp);
    }
#endif
    return funp;
}


static void break_point()
{
    if (sp - fp - csp->num_local_variables + 1 != 0)
	fatal("Bad stack pointer.\n");
}

#ifdef NEW_FUNCTIONS
struct program fake_prog = { "<function>" };
unsigned char fake_program = F_RETURN;

void setup_fake_frame P1(struct funp *, fun) {
  if (csp == &control_stack[MAX_TRACE-1]) {
    too_deep_error = 1;
    error("Too deep recursion.\n");
  }
  if (fun->owner->flags & O_DESTRUCTED)
      error("Owner of function pointer has been destructed.\n");
  csp++;
  csp->caller_type = caller_type;
  csp->framekind = FRAME_FAKE;
  csp->fr.funp = fun;
  csp->ob = current_object;
  csp->extern_call = 1;
  csp->prev_ob = previous_ob;
  csp->fp = fp;
  csp->prog = current_prog;
  csp->pc = pc;
  pc = (char *)&fake_program;
  csp->function_index_offset = function_index_offset;
  csp->variable_index_offset = variable_index_offset;
  csp->break_sp = break_sp;
  caller_type = ORIGIN_FUNCTION_POINTER;
  csp->num_local_variables = 0;
  current_prog = &fake_prog;
  previous_ob = current_object;
  current_object = fun->owner;
}

void remove_fake_frame() {
    DEBUG_CHECK(csp == (control_stack - 1),
		"Popped out of the control stack\n");
  current_object = csp->ob;
  current_prog = csp->prog;
  previous_ob = csp->prev_ob;
  caller_type = csp->caller_type;
  pc = csp->pc;
  fp = csp->fp;
  function_index_offset = csp->function_index_offset;
  variable_index_offset = csp->variable_index_offset;
  break_sp = csp->break_sp;
  csp--;
}
#endif

#ifdef NEW_FUNCTIONS
/* num_arg args are on the stack, and the args from the vector vec should be
 * put in front of them.  This is so that the order of arguments is logical.
 * 
 * evaluate( (: f, a :), b) -> f(a,b) and not f(b, a) which would happen
 * if we simply pushed the args from vec at this point.  (Note that the
 * old function pointers are broken in this regard)
 */
int merge_arg_lists P3(int, num_arg, struct vector *, vec, int, start) {
    int num_vec_arg = vec->size - start;
    struct svalue *sptr;
    int i;

    if (num_arg && num_vec_arg) {
	/* We need to do some stack movement so that the order
	   of arguments is logical */
	for (i=0; i<num_arg; i++)
	    *(sp-i+num_vec_arg) = *(sp-i);
	sptr = sp - num_arg + 1;
	for (i=start; i < vec->size; i++)
	    assign_svalue_no_free(sptr++, &vec->item[i]);
	sp += num_vec_arg;
    } else {
	for (i = start; i < vec->size; i++)
	    push_svalue(&vec->item[i]);
    }
    return num_arg + num_vec_arg;
}

void cfp_error P1(char *, s) {
    remove_fake_frame();
    error(s);
}

struct svalue *
call_function_pointer P2(struct funp *, funp, int, num_arg)
{
    struct object *ob;
    struct function *func;
    char *funcname;
    int i, def;

    setup_fake_frame(funp);
    
    switch (funp->type) {
    case FP_CALL_OTHER:
	if (funp->args.type == T_STRING)
	    funcname = funp->args.u.string;
	else if (funp->args.type == T_POINTER) {
	    check_for_destr(funp->args.u.vec);
	    if ( (funp->args.u.vec->size <1)
		|| (funp->args.u.vec->item[0].type != T_STRING) )
		cfp_error("First argument of call_other function pointer must be a string.\n");
	    funcname = funp->args.u.vec->item[0].u.string;
	    num_arg = merge_arg_lists(num_arg, funp->args.u.vec, 1);
	} else cfp_error("Illegal type for function name in function pointer.\n");
	if (funp->f.obj.type == T_OBJECT)
	    ob = funp->f.obj.u.ob;
	else if (funp->f.obj.type == T_POINTER) {
	    struct vector *vec;

	    vec = call_all_other(funp->f.obj.u.vec, funcname, num_arg);
	    remove_fake_frame();
	    free_svalue(&apply_ret_value, "call_function_pointer");
	    apply_ret_value.type = T_POINTER;
	    apply_ret_value.u.vec = vec;
	    return &apply_ret_value;
	} else if (funp->f.obj.type == T_STRING) {
	    ob = (struct object *)find_object(funp->f.obj.u.string);
	    if (!ob || !object_visible(ob))
		cfp_error("Function pointer couldn't find object\n");
	} else cfp_error("Function pointer object must be an object, array or string.\n");
	
#ifdef TRACE
	if (TRACEP(TRACE_CALL_OTHER)) {
	    do_trace("Call other (function pointer)", funcname, "\n");
	}
#endif
	call_origin = ORIGIN_CALL_OTHER;
	if (apply_low(funcname, ob, num_arg) == 0) {
	    remove_fake_frame();
	    return &const0u;
	}
	break;
    case FP_SIMUL:
	if (funp->args.type == T_POINTER) {
	    check_for_destr(funp->args.u.vec);
	    num_arg = merge_arg_lists(num_arg, funp->args.u.vec, 0);
	}
	call_simul_efun(funp->f.index, num_arg);
	break;
    case FP_EFUN:
	fp = sp - num_arg + 1;
	if (funp->args.type == T_POINTER) {
	    check_for_destr(funp->args.u.vec);
	    num_arg = merge_arg_lists(num_arg, funp->args.u.vec, 0);
	}
	i = funp->f.opcodes[0];
#ifdef NEEDS_CALL_EXTRA
	if (i == F_CALL_EXTRA) i = 0xff + funp->f.opcodes[1];
#endif
	if (num_arg == instrs[i].min_arg - 1 && (def = instrs[i].Default)) {
	    switch (def) {
	    case F_CONST0:
		*(++sp)=const0;
		break;
	    case F_CONST1:
		*(++sp)=const1;
		break;
	    case -((F_NBYTE << 8) + 1):
		sp++;
		sp->type = T_NUMBER;
		sp->subtype = 0;
		sp->u.number = -1;
		break;
	    case F_THIS_OBJECT:
		if (current_object && !(current_object->flags & O_DESTRUCTED))
		    push_object(current_object);
		else
		    *(++sp)=const0;
		break;
	    default:
		fatal("Unsupported type of default argument in efun function pointer.\n");
	    }
	    num_arg++;
	} else
	if (num_arg < instrs[i].min_arg) {
	    error("Too few arguments to efun %s in efun pointer.\n", instrs[i].name);
	} else
	if (num_arg > instrs[i].max_arg && instrs[i].max_arg != -1) {
		error("Too many arguments to efun %s in efun pointer.\n", instrs[i].name);
	    }
	if (instrs[i].min_arg != instrs[i].max_arg) {
	    /* need to update the argument count */
	    funp->f.opcodes[(i > 255 ? 2 : 1)] = num_arg;
	}
	eval_instruction((char*)&funp->f.opcodes[0]);
	free_svalue(&apply_ret_value, "call_function_pointer");
	apply_ret_value = *sp--;
	return &apply_ret_value;
    case FP_LOCAL | FP_NOT_BINDABLE: {
	fp = sp - num_arg + 1;
	
	func = &funp->owner->prog->p.i.functions[funp->f.index];
	
	if (func->flags & NAME_UNDEFINED)
	    error("Undefined function: %s\n", func->name);
	
        push_control_stack(FRAME_FUNCTION, func);
	
	caller_type = ORIGIN_LOCAL;
	current_prog = funp->owner->prog;
	
	if (funp->args.type == T_POINTER) {
	    struct vector *v = funp->args.u.vec;
	    
	    check_for_destr(v);
	    num_arg = merge_arg_lists(num_arg, v, 0);
	}
	
	csp->num_local_variables = num_arg;
	func = setup_new_frame(func);

	csp->extern_call = 1;
	call_program(current_prog, func->offset);
	break;
    }
    case FP_FUNCTIONAL: 
    case FP_FUNCTIONAL | FP_NOT_BINDABLE: {
	fp = sp - num_arg + 1;

        push_control_stack(FRAME_FUNP, funp);
	caller_type = ORIGIN_FUNCTIONAL;
	current_prog = funp->f.a.prog;
	
	if (funp->args.type == T_POINTER) {
	    struct vector *v = funp->args.u.vec;
	    
	    check_for_destr(v);
	    num_arg = merge_arg_lists(num_arg, v, 0);
	}

	setup_variables(num_arg, funp->f.a.num_locals, funp->f.a.num_args);

	csp->extern_call = 1;
	function_index_offset = funp->f.a.fio;
	variable_index_offset = funp->f.a.vio;
	call_program(funp->f.a.prog, funp->f.a.offset);
	break;
    }
    default:
	error("Unsupported function pointer type.\n");
    }
    free_svalue(&apply_ret_value, "call_function_pointer");
    apply_ret_value = *sp--;
    remove_fake_frame();
    return &apply_ret_value;
}
#else
INLINE
struct svalue *
call_function_pointer P2(struct funp *, funp, int, num_arg) {
    struct svalue *arg;
    char *funcname;
    int i;
    struct object *ob;

    if (current_object->flags & O_DESTRUCTED) {	/* No external calls allowed */
	pop_n_elems(num_arg);
	return &const0u;
    }
    if (funp->fun.type == T_STRING)
	funcname = funp->fun.u.string;
    else {			/* must be T_POINTER then */
	check_for_destr(funp->fun.u.vec);
	if ((funp->fun.u.vec->size < 1)
	    || (funp->fun.u.vec->item[0].type != T_STRING))
	    error("call_other: 1st elem of array for arg 2 must be a string\n");
	funcname = funp->fun.u.vec->item[0].u.string;	/* complicated huh? */
	for (i = 1; i < funp->fun.u.vec->size; i++)
	    push_svalue(&funp->fun.u.vec->item[i]);
	num_arg += i - 1;
    }
    if (funp->obj.type == T_OBJECT)
	ob = funp->obj.u.ob;
    else if (funp->obj.type == T_POINTER) {
	struct vector *v;

	v = call_all_other(funp->obj.u.vec, funcname, num_arg - 2);
	free_svalue(&apply_ret_value, "call_function_pointer");
	apply_ret_value.type = T_POINTER;
	apply_ret_value.u.vec = v;
	return &apply_ret_value;
    } else {
	ob = find_object(arg[0].u.string);
	if (!ob || !object_visible(ob))
	    error("call_other() couldn't find object\n");
    }
    /* Send the remaining arguments to the function. */
#ifdef TRACE
    if (TRACEP(TRACE_CALL_OTHER)) {
	do_trace("Call other ", funcname, "\n");
    }
#endif
    call_origin = ORIGIN_CALL_OTHER;
    if (apply_low(funcname, ob, num_arg) == 0) {	/* Function not found */
	return &const0u;
    }
    free_svalue(&apply_ret_value, "call_function_pointer");
    apply_ret_value = *sp--;
    return &apply_ret_value;
}
#endif

/* marion
 * maintain a small and inefficient stack of error recovery context
 * data structures.
 * This routine is called in three different ways:
 * push=-1    Pop the stack.
 * push=1 push the stack.
 * push=0 No error occured, so the pushed value does not have to be
 *        restored. The pushed value can simply be popped into the void.
 *
 * The stack is implemented as a linked list of stack-objects, allocated
 * from the heap, and deallocated when popped.
 */

/* push_pop_error_context: Copied directly from Lars version 3.1.1 */
struct error_context_stack *ecsp = 0;

void push_pop_error_context P1(int, push)
{
    struct error_context_stack *p;

    if (push == 1) {
	/*
	 * Save some global variables that must be restored separately after
	 * a longjmp. The stack will have to be manually popped all the way.
	 */
	p = (struct error_context_stack *)
	    DXALLOC(sizeof(struct error_context_stack), TAG_ERROR_CONTEXT, "push_pop_error_context");
	p->save_sp = sp;
	p->save_csp = csp;
	p->save_command_giver = command_giver;
	memcpy(
		  (char *) p->old_error_context,
		  (char *) error_recovery_context,
		  sizeof error_recovery_context);
	p->old_exists_flag = error_recovery_context_exists;
	p->next = ecsp;
	ecsp = p;
    } else {
	p = ecsp;
	if (p == 0) {
	    fatal("Catch: error context stack underflow.");
	}
	if (push == 0) {
	    DEBUG_CHECK(csp != (p->save_csp - 1), 
			"Catch: Lost track of csp\n");
	} else {
	    /*
	     * push == -1 ! They did a throw() or error. That means that the
	     * control stack must be restored manually here.
	     */
	    csp = p->save_csp;
	    pop_n_elems(sp - p->save_sp);
	    command_giver = p->save_command_giver;
	}
	memcpy((char *) error_recovery_context,
	       (char *) p->old_error_context,
	       sizeof error_recovery_context);
	error_recovery_context_exists = p->old_exists_flag;
	ecsp = p->next;
	FREE((char *) p);
    }
}

/*
 * When a vector is given as argument to an efun, all items have to be
 * checked if there would be a destructed object.
 * A bad problem currently is that a vector can contain another vector, so this
 * should be tested too. But, there is currently no prevention against
 * recursive vectors, which means that this can not be tested. Thus, MudOS
 * may crash if a vector contains a vector that contains a destructed object
 * and this top-most vector is used as an argument to an efun.
 */
/* MudOS won't crash when doing simple operations like assign_svalue
 * on a destructed object. You have to watch out, of course, that you don't
 * apply a function to it.
 * to save space it is preferable that destructed objects are freed soon.
 *   amylaar
 */
void check_for_destr P1(struct vector *, v)
{
    int i = v->size;

    while (i--) {
	if ((v->item[i].type & T_OBJECT) && (v->item[i].u.ob->flags & O_DESTRUCTED)) {
	    free_svalue(&v->item[i], "check_for_destr");
	    v->item[i] = const0;
	}
    }
}

 /* do_loop_cond() coded by John Garnett, 1993/06/01

    Optimizes these four cases (with 'int i'):

    1) for (expr0; i < integer_variable; expr2) statement;
    2) for (expr0; i < integer_constant; expr2) statement;
    3) while (i < integer_variable) statement;
    4) while (i < integer_constant) statement;
 */

static INLINE void do_loop_cond()
{
    struct svalue *s1, *s2;
    int i;
    
    s1 = fp + EXTRACT_UCHAR(pc++);    /* a from (a < b) */
    if (*pc++ == F_LOCAL){
	s2 = fp + EXTRACT_UCHAR(pc++);
	switch(s1->type | s2->type){
	case T_NUMBER: 
	    i = s1->u.number < s2->u.number;
	    break;
	case T_REAL:
	    i = s1->u.real < s2->u.real;
	    break;
	case T_STRING:
	    i = (strcmp(s1->u.string, s2->u.string) < 0);
	    break;
	case T_NUMBER|T_REAL:
	    if (s1->type == T_NUMBER) i = s1->u.number < s2->u.real;
	    else i = s1->u.real < s2->u.number;
	    break;
	default:
	    if (s1->type == T_OBJECT && (s1->u.ob->flags & O_DESTRUCTED)){
		free_object(s1->u.ob, "do_loop_cond:1");
		*s1 = const0;
	    }
	    if (s2->type == T_OBJECT && (s2->u.ob->flags & O_DESTRUCTED)){
		free_object(s2->u.ob, "do_loop_cond:2");
		*s2 = const0;
	    }
	    if (s1->type == T_NUMBER && s2->type == T_NUMBER){
		i = 0;
		break;
	    }
	    switch(s1->type){
	    case T_NUMBER:
	    case T_REAL:
		error("2nd argument to < is not numeric when the 1st is.\n");
	    case T_STRING:
		error("2nd argument to < is not string when the 1st is.\n");
	    default:
		error("Bad 1st argument to <.\n");
	    }
        }
	if (i) {
	    unsigned short offset;
	    
	    COPY_SHORT(&offset, pc);
	    pc -= offset;
	} else pc += 2;
	
    } else {
	LOAD_INT(i, pc);
	if (s1->type == T_NUMBER) {
	    if (s1->u.number < i){
		unsigned short offset;
		
		COPY_SHORT(&offset, pc);
		pc -= offset;
	    } else pc += 2;
	} else if (s1->type == T_REAL) {
	    if (s1->u.real < i) {
		unsigned short offset;
		
		COPY_SHORT(&offset, pc);
		pc -= offset;
	    } else pc += 2;
	} else error("Right side of < is a number, left side is not.\n");
    }
}

#ifdef LPC_TO_C
void
call_program P2(struct program *, prog, int, offset) {
    struct svalue ret;
    
    if (prog->p.i.program_size)
	eval_instruction(prog->p.i.program + offset);
    else {
	DEBUG_CHECK(!offset, "Null function pointer in jump_table.\n");
	ret.type = T_NUMBER;
	(*
	 ( void (*)() ) offset     /* cast to a function pointer */
	 )(&ret);
	*sp++ = ret;
    }
}
#endif

 /*
  * Evaluate instructions at address 'p'. All program offsets are
  * to current_prog->p.i.program. 'current_prog' must be setup before
  * call of this function.
  *
  * There must not be destructed objects on the stack. The destruct_object()
  * function will automatically remove all occurences. The effect is that
  * all called efuns knows that they won't have destructed objects as
  * arguments.
  */
#ifdef TRACE_CODE
 static int previous_instruction[60];
 static int stack_size[60];
 static char *previous_pc[60];
 static int last;
#endif

 void
 eval_instruction P1(char *, p)
 {
#ifdef DEBUG
     int num_arg;
#endif
     int i;
     float real;
     struct svalue *lval;
     int instruction;
     unsigned short offset;
     static func_t *oefun_table = efun_table - BASE;
     IF_DEBUG(struct svalue *expected_stack);

     /* Next F_RETURN at this level will return out of eval_instruction() */
     csp->extern_call = 1;
     pc = p;
     while (1) {
	 instruction = EXTRACT_UCHAR(pc++);
	 /* These defines can't handle the F_CALL_EXTRA optimization */
#if defined(TRACE_CODE) || defined(TRACE) || defined(OPCPROF)
#  ifdef NEEDS_CALL_EXTRA
	 if (instruction == F_CALL_EXTRA)
	     instruction = EXTRACT_UCHAR(pc++) + 0xff;
#  endif
#  ifdef TRACE_CODE
	 previous_instruction[last] = instruction;
	 previous_pc[last] = pc - 1;
	 stack_size[last] = sp - fp - csp->num_local_variables;
	 last = (last + 1) % (sizeof previous_instruction / sizeof(int));
#  endif
#  ifdef TRACE
	 if (TRACEP(TRACE_EXEC)) {
	     do_trace("Exec ", get_f_name(instruction), "\n");
	 }
#  endif
#  ifdef OPCPROF
	 if (instruction < BASE)
	     opc_eoper[instruction]++;
	 else
	     opc_efun[instruction-BASE].count++;
#  endif
#endif
	 if (!--eval_cost) {
	     fprintf(stderr, "eval_cost too big %d\n", max_cost);
	     eval_cost = max_cost;
	     max_eval_error = 1;
	     error("Too long evaluation. Execution aborted.\n");
	 }
	 /*
	  * Execute current instruction. Note that all functions callable from
	  * LPC must return a value. This does not apply to control
	  * instructions, like F_JUMP.
	  */
	 switch (instruction) {
	 case F_INC:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to ++\n");
	     lval = (sp--)->u.lvalue;
	     switch (lval->type) {
	     case T_NUMBER:
		 lval->u.number++;
		 break;
	     case T_REAL:
		 lval->u.real++;
		 break;
	     case T_LVALUE_BYTE:
		 ++*glb.u.lvalue_byte;
		 break;
	     default:
		 error("++ of non-numeric argument\n");
	     }
	     break;
	 case F_WHILE_DEC:
	     {
		 struct svalue *s;

		 s = fp + EXTRACT_UCHAR(pc);
		 pc += 2;	/* skip the BBRANCH */
		 if (s->type & T_NUMBER) {
		     i = s->u.number--;
		 } else if (s->type & T_REAL) {
		     i = s->u.real--;
		 } else {
		     error("-- of non-numeric argument\n");
		 }
		 if (i) {
		     COPY_SHORT(&offset, pc);
		     pc -= offset;
		 } else {
		     pc += 2;
		 }
	     }
	     break;
	 case F_LOCAL_LVALUE:
	     (++sp)->type = T_LVALUE;
	     sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
	     break;
	 case F_NUMBER:
	     LOAD_INT(i, pc);
	     push_number(i);
	     break;
	 case F_REAL:
	     LOAD_FLOAT(real, pc);
	     push_real(real);
	     break;
	 case F_BYTE:
	     push_number(EXTRACT_UCHAR(pc++));
	     break;
	 case F_NBYTE:
	     push_number(-((int)EXTRACT_UCHAR(pc++)));
	     break;
#ifdef F_JUMP_WHEN_NON_ZERO
	 case F_JUMP_WHEN_NON_ZERO:
	     if ((i = (sp->type == T_NUMBER)) && (sp->u.number == 0))
		 pc += 2;
	     else {
		 COPY_SHORT(&offset, pc);
		 pc = current_prog->p.i.program + offset;
	     }
	     if (i) {
		 sp--;	/* when sp is an integer svalue, its cheaper
			  * to do this */
	     } else {
		 pop_stack();
	     }
	     break;
#endif
	 case F_BRANCH:	/* relative offset */
	     COPY_SHORT(&offset, pc);
	     pc += offset;
	     break;
	 case F_BBRANCH:	/* relative offset */
	     COPY_SHORT(&offset, pc);
	     pc -= offset;
	     break;
	 case F_BRANCH_WHEN_ZERO:	/* relative offset */
	     if (sp->type & T_NUMBER) {
		 if (!((sp--)->u.number)) {
		     COPY_SHORT(&offset, pc);
		     pc += offset;
		     break;
		 }
	     } else pop_stack();
	     pc += 2;	/* skip over the offset */
	     break;
	 case F_BRANCH_WHEN_NON_ZERO:	/* relative offset */
	     if (sp->type & T_NUMBER) {
		 if (!((sp--)->u.number)) {
		     pc += 2;
		     break;
		 }
	     } else pop_stack();
	     COPY_SHORT(&offset, pc);
	     pc += offset;
	     break;
	 case F_BBRANCH_WHEN_ZERO:	/* relative backwards offset */
	     if (sp->type & T_NUMBER) {
		 if (!((sp--)->u.number)) {
		     COPY_SHORT(&offset, pc);
		     pc -= offset;
		     break;
		 }
	     } else pop_stack();
	     pc += 2;
	     break;
	 case F_BBRANCH_WHEN_NON_ZERO:	/* relative backwards offset */
	     if (sp->type & T_NUMBER) {
		 if (!((sp--)->u.number)) {
		     pc += 2;
		     break;
		 }
	     } else pop_stack();
	     COPY_SHORT(&offset, pc);
	     pc -= offset;
	     break;
	 case F_LOR:
	     /* replaces F_DUP; F_BRANCH_WHEN_NON_ZERO; F_POP */
	     if (sp->type & T_NUMBER) {
		 if (!sp->u.number) {
		     pc += 2;
		     sp--;
		     break;
		 }
	     }
	     COPY_SHORT(&offset, pc);
	     pc += offset;
	     break;
	case F_LAND:
	     /* replaces F_DUP; F_BRANCH_WHEN_ZERO; F_POP */
	     if (sp->type & T_NUMBER) {
		 if (!sp->u.number) {
		     COPY_SHORT(&offset, pc);
		     pc += offset;
		     break;
		 }
		 sp--;
	     } else pop_stack();
	     pc += 2;
	     break;
	case F_LOOP_INCR:	/* this case must be just prior to
				 * F_LOOP_COND */
	    {
		struct svalue *s;
		
		s = fp + EXTRACT_UCHAR(pc++);
		if (s->type & T_NUMBER) {
		    s->u.number++;
		} else if (s->type & T_REAL) {
		    s->u.real++;
		} else {
		    error("++ of non-numeric argument\n");
		}
	    }
	    if (*pc == F_LOOP_COND) 
		pc++;
	    else
		break;
	 case F_LOOP_COND:
	     do_loop_cond();
	     break;
	case F_LOCAL:
	    {
		struct svalue *s;
		
		s = fp + EXTRACT_UCHAR(pc++);
		DEBUG_CHECK((fp-s) >= csp->num_local_variables,
			    "Tried to push non-existent local\n");
		
		/*
		 * If variable points to a destructed object, replace it
		 * with 0, otherwise, fetch value of variable.
		 */
		if ((s->type & T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
		    *++sp = const0;
		    assign_svalue(s, &const0);
		} else {
		    assign_svalue_no_free(++sp, s);
		}
		break;
	    }
	 case F_LT:
	     {
		 i = sp->type;
		 switch (i | (--sp)->type) {
		 case T_NUMBER:
		     sp->u.number = sp->u.number < (sp+1)->u.number;
		     break;
		 case T_REAL:
		     sp->u.number = sp->u.real < (sp+1)->u.real;
		     sp->type = T_NUMBER;
		     break;
		 case T_NUMBER|T_REAL:
		     if (i & T_NUMBER) {
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
			 bad_argument(sp, T_NUMBER | T_REAL, 2, instruction);
		     case T_STRING:
			 bad_argument(sp, T_STRING, 2, instruction);
		     default:
			 bad_argument(sp-1, T_NUMBER | T_STRING | T_REAL, 1, instruction);
		     }
		 }
	     }
	     break;
	 case F_ADD:
	     {
		 switch (sp->type) {
		 case T_BUFFER:
		     {
			 if (!((sp-1)->type & T_BUFFER)) {
			     error("Bad type argument to +. Had %s and %s.\n",
				   type_name((sp - 1)->type), type_name(sp->type));
			 } else {
			     struct buffer *b;
			     
			     b = allocate_buffer(sp->u.buf->size + (sp - 1)->u.buf->size);
			     memcpy(b->item, (sp - 1)->u.buf->item, (sp - 1)->u.buf->size);
			     memcpy(b->item + (sp - 1)->u.buf->size, sp->u.buf->item,
				    sp->u.buf->size);
			     free_buffer((sp--)->u.buf);
			     free_buffer(sp->u.buf);
			     sp->u.buf = b;
			 }
			 break;
		     } /* end of x + T_BUFFER */
		 case T_NUMBER:
		     {
			 switch ((--sp)->type) {
			 case T_NUMBER:
			     sp->u.number += (sp+1)->u.number;
			     break;
			 case T_REAL:
			     sp->u.real += (sp+1)->u.number;
			     break;
			 case T_STRING:
			     {
				 char buff[20];

				 sprintf(buff, "%d", (sp+1)->u.number);
				 EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
				 break;
			     }
			 default:
			     error("Bad type argument to +.  Had %s and %s.\n",
				   type_name(sp->type), type_name((sp+1)->type));
			 }
			 break;
		     } /* end of x + NUMBER */
		 case T_REAL:
		     {
			 switch ((--sp)->type) {
			 case T_NUMBER:
			     sp->type = T_REAL;
			     sp->u.real = sp->u.number + (sp+1)->u.real;
			     break;
			 case T_REAL:
			     sp->u.real += (sp+1)->u.real;
			     break;
			 case T_STRING:
			     {
				 char buff[40];

				 sprintf(buff, "%f", (sp+1)->u.real);
				 EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
				 break;
			     }
			 default:
			     error("Bad type argument to +. Had %s and %s\n",
				  type_name(sp->type), type_name((sp+1)->type));
			 }
			 break;
		     } /* end of x + T_REAL */
		 case T_POINTER:
		     {
			 if (!((sp-1)->type & T_POINTER)) {
			     error("Bad type argument to +. Had %s and %s\n",
			      type_name((sp - 1)->type), type_name(sp->type));
			 } else {
			     /* add_array now free's the vectors */
			     (sp-1)->u.vec = add_array((sp - 1)->u.vec, sp->u.vec);
			     sp--;
			     break;
			 }
		     } /* end of x + T_POINTER */
		 case T_MAPPING:
		     {
			 if ((sp-1)->type & T_MAPPING) {
			     struct mapping *map;
			
			     map = add_mapping((sp - 1)->u.map, sp->u.map);
			     free_mapping((sp--)->u.map);
			     free_mapping(sp->u.map);
			     sp->u.map = map;
			     break;
			 } else
			     error("Bad type argument to +. Had %s and %s\n",
			      type_name((sp - 1)->type), type_name(sp->type));
		     } /* end of x + T_MAPPING */
		 case T_STRING:
		     {
			 switch ((sp-1)->type) {
			 case T_NUMBER:
			     {
				 char buff[20];

				 sprintf(buff, "%d", (sp-1)->u.number);
				 SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
				 break;
			     } /* end of T_NUMBER + T_STRING */
			 case T_REAL:
			     {
				 char buff[40];
				 
				 sprintf(buff, "%f", (sp - 1)->u.real);
				 SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
				 break;
			     } /* end of T_REAL + T_STRING */
			 case T_STRING:
			     {
				 SVALUE_STRING_JOIN(sp-1, sp, "f_add: 1");
				 sp--;
				 break;
			     } /* end of T_STRING + T_STRING */
			 default:
			     error("Bad type argument to +. Had %s and %s\n",
				   type_name((sp - 1)->type), type_name(sp->type));
			 }
			 break;
		     } /* end of x + T_STRING */
		     
		 default:
		     error("Bad type argument to +.  Had %s and %s.\n",
			   type_name((sp-1)->type), type_name(sp->type));
		 }
		 break;
	     }
	case F_VOID_ADD_EQ:
	case F_ADD_EQ:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to +=\n");
	     lval = sp->u.lvalue;
	     sp--; /* points to the RHS */
	     switch (lval->type) {
	     case T_STRING:
		 if (sp->type == T_STRING) {
		     SVALUE_STRING_JOIN(lval, sp, "f_add_eq: 1");
		 } else if (sp->type & T_NUMBER) {
		     char buff[20];
		     
		     sprintf(buff, "%d", sp->u.number);
		     EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
		 } else if (sp->type & T_REAL) {
		     char buff[40];
		     
		     sprintf(buff, "%f", sp->u.real);
		     EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
		 } else {
		     bad_argument(sp, T_STRING | T_NUMBER | T_REAL, 2, instruction);
		 }
		 break;
	     case T_NUMBER:
		 if (sp->type & T_NUMBER) {
		     lval->u.number += sp->u.number;
		     /* both sides are numbers, no freeing required */
		 } else if (sp->type & T_REAL) {
		     lval->u.number += sp->u.real;
		     /* both sides are numbers, no freeing required */
		 } else {
		     error("Left hand side of += is a number (or zero); right side is not a number.\n");
		 }
		 break;
	     case T_REAL:
		 if (sp->type & T_NUMBER) {
		     lval->u.real += sp->u.number;
		     /* both sides are numerics, no freeing required */
		 }
		 if (sp->type & T_REAL) {
		     lval->u.real += sp->u.real;
		     /* both sides are numerics, no freeing required */
		 } else {
		     error("Left hand side of += is a number (or zero); right side is not a number.\n");
		 }
		 break;
	     case T_BUFFER:
		 if (sp->type != T_BUFFER) {
		     bad_argument(sp, T_BUFFER, 2, instruction);
		 } else {
		     struct buffer *b;
		     
		     b = allocate_buffer(lval->u.buf->size + sp->u.buf->size);
		     memcpy(b->item, lval->u.buf->item, lval->u.buf->size);
		     memcpy(b->item + lval->u.buf->size, sp->u.buf->item,
			    sp->u.buf->size);
		     free_buffer(sp->u.buf);
		     free_buffer(lval->u.buf);
		     lval->u.buf = b;
		 }
		 break;
	     case T_POINTER:
		 if (sp->type != T_POINTER)
		     bad_argument(sp, T_POINTER, 2, instruction);
		 else {
		     /* add_array now frees the vectors */
		     lval->u.vec = add_array(lval->u.vec, sp->u.vec);
		 }
		 break;
	     case T_MAPPING:
		 if (sp->type != T_MAPPING)
		     bad_argument(sp, T_MAPPING, 2, instruction);
		 else {
		     absorb_mapping(lval->u.map, sp->u.map);
		     free_mapping(sp->u.map);	/* free RHS */
		     /* LHS not freed because its being reused */
		 }
		 break;
	     case T_LVALUE_BYTE:
                 if (sp->type != T_NUMBER)
                     error("Bad right type to += of char lvalue.\n");
                 else *glb.u.lvalue_byte += sp->u.number;
                 break;
	     default:
		 bad_arg(1, instruction);
	     }
	     
	     if (instruction == F_ADD_EQ) {	/* not void add_eq */
		 assign_svalue_no_free(sp, lval);
	     } else {
		 /*
		  * but if (void)add_eq then no need to produce an
		  * rvalue
		  */
		 sp--;
	     }
	     break;
	 case F_AND:
	     f_and();
	     break;
	 case F_AND_EQ:
	     f_and_eq();
	     break;
	case F_FUNCTION_CONSTRUCTOR:
	    f_function_constructor();
	    break;
#ifndef NEW_FUNCTIONS
	case F_EVALUATE:
	    f_evaluate();
	    break;
#endif
	case F_AGGREGATE:
	    {
		struct vector *v;
		
		LOAD_SHORT(offset, pc);
		v = allocate_empty_array((int) offset);
		/*
		 * transfer svalues in reverse...popping stack as we go
		 */
		while (offset--)
		    v->item[offset] = *sp--;
		(++sp)->type = T_POINTER;
		sp->u.vec = v;
	    }
	    break;
	case F_AGGREGATE_ASSOC:
            {
		struct mapping *m;
		
		LOAD_SHORT(offset, pc);
		
		m = load_mapping_from_aggregate(sp -= offset, offset);
		(++sp)->type = T_MAPPING;
		sp->u.map = m;
		break;
	    }
	case F_ASSIGN:
#ifdef DEBUG
             if (sp->type != T_LVALUE) fatal("Bad argument to F_ASSIGN\n");
#endif
             switch(sp->u.lvalue->type){
                 case T_LVALUE_BYTE:
                     if ((sp - 1)->type != T_NUMBER) {
                         error("Illegal rhs to char lvalue\n");
                     } else {
                         *glb.u.lvalue_byte = ((sp - 1)->u.number & 0xff);
                     }
                     break;
		 default:
                     assign_svalue(sp->u.lvalue, sp - 1);
                     break;
		 case T_LVALUE_RANGE:
                     assign_lvalue_range(sp - 1);
                     break;
	     }
             sp--;              /* ignore lvalue */
             /* rvalue is already in the correct place */
             break;
	 case F_VOID_ASSIGN:
#ifdef DEBUG
             if (sp->type != T_LVALUE) fatal("Bad argument to F_VOID_ASSIGN\n");
#endif
             lval = (sp--)->u.lvalue;
             if (sp->type != T_INVALID){
                 switch(lval->type){
                     case T_LVALUE_BYTE:
                     {
                         if (sp->type != T_NUMBER){
                             error("Illegal rhs to char lvalue\n");
                         } else {
                             *glb.u.lvalue_byte = (sp--)->u.number & 0xff;
                         }
                         break;
                     }

                     case T_STRING:
                     {
                         free_svalue(lval, "F_VOID_ASSIGN : 2");
                         if (sp->subtype != STRING_CONSTANT) {
                             /*
                              * avoid unnecessary (and costly)
                              * string_copy()...FREE() or
                              * ref_string()...free_string()
                              */
                             *lval = *sp--;     /* copy string directly */
                         } else {
                             lval->type = T_STRING;
                             lval->subtype = STRING_SHARED;
                             lval->u.string = make_shared_string((sp--)->u.string);
                         }
                         break;
                     }

                     case T_LVALUE_RANGE:
                     {
                         copy_lvalue_range(sp--);
                         break;
                     }

                     default:
                     {
                         free_svalue(lval, "F_VOID_ASSIGN : 3");
                         *lval = *sp--;
                     }
                 }
             } else sp--;
             break;
#ifdef DEBUG
	case F_BREAK_POINT:
	    break_point();
	    break;
#endif
	case F_BREAK:
	    pc = current_prog->p.i.program + *break_sp++;
	    break;
	case F_CALL_FUNCTION_BY_ADDRESS:
	    {
		struct function *funp;
		
		LOAD_SHORT(offset, pc);
		offset += function_index_offset;
		/*
		 * Find the function in the function table. As the
		 * function may have been redefined by inheritance, we
		 * must look in the last table, which is pointed to by
		 * current_object.
		 */
		DEBUG_CHECK(offset >= current_object->prog->p.i.num_functions,
			    "Illegal function index\n");
		
		funp = &current_object->prog->p.i.functions[offset];

		if (funp->flags & NAME_UNDEFINED)
		    error("Undefined function: %s\n", funp->name);
		/* Save all important global stack machine registers */
		push_control_stack(FRAME_FUNCTION, funp);
		
		caller_type = ORIGIN_LOCAL;
		/* This assigment must be done after push_control_stack() */
		current_prog = current_object->prog;
		/*
		 * If it is an inherited function, search for the real
		 * definition.
		 */
		csp->num_local_variables = EXTRACT_UCHAR(pc++);
		function_index_offset = variable_index_offset = 0;
		funp = setup_new_frame(funp);
		csp->pc = pc;	/* The corrected return address */
#ifdef LPC_TO_C
		if (current_prog->p.i.program_size) {
#endif
		    pc = current_prog->p.i.program + funp->offset;
		    csp->extern_call = 0;
#ifdef LPC_TO_C
		} else {
/* TODO
		    struct svalue ret =
			{T_NUMBER};
		    DEBUG_CHECK(!(funp->offset),
				"Null function pointer in jump_table.\n");
		    (*
		     (void (*) ()) (funp->offset)
		     ) (&ret);
		    *sp++ = ret;
*/
		}
#endif
	    }
	    break;
       case F_CALL_INHERITED:
	    {
		struct inherit *ip = current_prog->p.i.inherit + *pc++;
		struct program *temp_prog = ip->prog;
		struct function *funp;
		
		LOAD_SHORT(offset, pc);

		funp = &temp_prog->p.i.functions[offset];
		
		push_control_stack(FRAME_FUNCTION, funp);

		caller_type = ORIGIN_LOCAL;
		current_prog = temp_prog;
		
		csp->num_local_variables = EXTRACT_UCHAR(pc++);
		
		function_index_offset += ip->function_index_offset;
		variable_index_offset += ip->variable_index_offset;
		
		funp = setup_inherited_frame(funp);
		csp->pc = pc;
		pc = current_prog->p.i.program + funp->offset;
		csp->extern_call = 0;
	    }
	    break;
	case F_COMPL:
	    if (sp->type != T_NUMBER)
		error("Bad argument to ~\n");
	    sp->u.number = ~sp->u.number;
	    break;
	case F_CONST0:
	    push_number(0);
	    break;
	case F_CONST1:
	    push_number(1);
	    break;
	case F_PRE_DEC:
	    DEBUG_CHECK(sp->type != T_LVALUE, 
			"non-lvalue argument to --\n");
	     lval = sp->u.lvalue;
	     switch (lval->type) {
	     case T_NUMBER:
		 sp->type = T_NUMBER;
		 sp->u.number = --(lval->u.number);
		 break;
	     case T_REAL:
		 sp->type = T_REAL;
		 sp->u.real = --(lval->u.real);
		 break;
	     case T_LVALUE_BYTE:
		 sp->type = T_NUMBER;
		 sp->u.number = --(*glb.u.lvalue_byte);
		 break;
	     default:
		error("-- of non-numeric argument\n");
	     }
	     break;
	 case F_DEC:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to --\n");
	     lval = (sp--)->u.lvalue;
	     switch (lval->type) {
	     case T_NUMBER:
		 lval->u.number--;
		 break;
	     case T_REAL:
		 lval->u.real--;
		 break;
	     case T_LVALUE_BYTE:
		 --(*glb.u.lvalue_byte);
		 break;
	     default:
		 error("-- of non-numeric argument\n");
	     }
	     break;
	case F_DIVIDE:
	    { 
		switch((sp-1)->type|sp->type){
		    
		case T_NUMBER:
		    {
			if (!(sp--)->u.number) error("Division by zero\n");
			sp->u.number /= (sp+1)->u.number;
			break;
		    }
		    
		case T_REAL:
		    {
			if ((sp--)->u.real == 0.0) error("Division by zero\n");
			sp->u.real /= (sp+1)->u.real;
			break;
		    }
		    
		case T_NUMBER|T_REAL:
		    {
			if ((sp--)->type & T_NUMBER){
			    if (!((sp+1)->u.number)) error("Division by zero\n");
			    sp->u.real /= (sp+1)->u.number;
			} else {
			    if ((sp+1)->u.real == 0.0) error("Division by 0.0\n");
			    sp->type = T_REAL;
			    sp->u.real = sp->u.number / (sp+1)->u.real;
			}
			break;
		    }
		    
		default:
		    {
			if (!((sp-1)->type & (T_NUMBER|T_REAL)))
			    bad_argument(sp-1,T_NUMBER|T_REAL,1, instruction);
			if (!(sp->type & (T_NUMBER|T_REAL)))
			    bad_argument(sp, T_NUMBER|T_REAL,2, instruction);
		    }
		}
	    }
	    break;
	case F_DIV_EQ:
	    f_div_eq();
	    break;
	case F_EQ:
	    f_eq();
	    break;
	case F_GE:
	    {
		i = sp->type;
		switch ((--sp)->type | i) {
		case T_NUMBER:
		    sp->u.number = sp->u.number >= (sp+1)->u.number;
		    break;
		case T_REAL:
		    sp->u.number = sp->u.real >= (sp+1)->u.real;
		    sp->type = T_NUMBER;
		    break;
		case T_NUMBER | T_REAL:
		    if (i & T_NUMBER) {
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
			    bad_argument(sp, T_NUMBER | T_REAL, 2, instruction);
			case T_STRING:
			    bad_argument(sp, T_STRING, 2, instruction);
			default:
			    bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, instruction);
			}
		    }
		}
	    }
	    break;
	case F_GT:
	    {
		i = sp->type;
		switch ((--sp)->type | i) {
		case T_NUMBER:
		    sp->u.number = sp->u.number > (sp+1)->u.number;
		    break;
		case T_REAL:
		    sp->u.number = sp->u.real > (sp+1)->u.real;
		    sp->type = T_NUMBER;
		    break;
		case T_NUMBER | T_REAL:
		    if (i & T_NUMBER) {
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
			    bad_argument(sp, T_NUMBER | T_REAL, 2, instruction);
			case T_STRING:
			    bad_argument(sp, T_STRING, 2, instruction);
			default:
			    bad_argument(sp-1, T_NUMBER | T_REAL | T_STRING, 1, instruction);
			}
		    }
		}
		break;
	    }
	case F_GLOBAL:
	    {
		struct svalue *s;
		
		s = find_value((int) (EXTRACT_UCHAR(pc++) + variable_index_offset));
		
		/*
		 * If variable points to a destructed object, replace it
		 * with 0, otherwise, fetch value of variable.
		 */
		if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
		    *++sp = const0;
		    assign_svalue(s, &const0);
		} else {
		    assign_svalue_no_free(++sp, s);
		}
		break;
	    }
	case F_PRE_INC:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to ++\n");
	     lval = sp->u.lvalue;
	     switch (lval->type) {
	     case T_NUMBER:
		 sp->type = T_NUMBER;
		 sp->u.number = ++lval->u.number;
		 break;
	     case T_REAL:
		 sp->type = T_REAL;
		 sp->u.real = ++lval->u.number;
		 break;
	     case T_LVALUE_BYTE:
		 sp->type = T_NUMBER;
		 sp->u.number = ++*glb.u.lvalue_byte;
		 break;
	     default:
		 error("++ of non-numeric argument\n");
	     }
	     break;
	 case F_INDEX:
             switch (sp->type) {
		 case T_MAPPING:
                 {
                     struct svalue *v;
                     struct mapping *m;

                     v = find_in_mapping(m = sp->u.map, sp - 1);
                     assign_svalue(--sp, v);    /* v will always have a
                                                 * value */
                     free_mapping(m);
                     break;
                 }
		 case T_BUFFER:
                 {
                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing a buffer with an illegal type.\n");

                     i = (sp - 1)->u.number;
                     if ((i > sp->u.buf->size) || (i < 0))
                         error("Buffer index out of bounds.\n");
                     i = sp->u.buf->item[i];
                     free_buffer(sp->u.buf);
                     (--sp)->u.number = i;
                     break;
                 }
		 case T_STRING:
                 {
                     if ((sp-1)->type != T_NUMBER) {
                         error("Indexing a string with an illegal type.\n");
                     }
                     i = (sp - 1)->u.number;
                     if ((i > SVALUE_STRLEN(sp)) || (i < 0))
                         error("String index out of bounds.\n");
                     i = (unsigned char) sp->u.string[i];
                     free_string_svalue(sp);
                     (--sp)->u.number = i;
                     break;
                 }
#ifndef NEW_FUNCTIONS
		 case T_FUNCTION:
                 {
                     struct funp *f;

                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing a function with an illegal type\n");
                     i = (sp - 1)->u.number;
                     if (i<0 || i>1)
                         error("Function variables may only be indexed with 0 or 1.\n");
                     f = sp->u.fp;
                     assign_svalue_no_free(--sp, i ? &f->fun : &f->obj);
                     free_funp(f);
                     break;
                 }
#endif
		 case T_POINTER:
                 {
                     struct vector *vec;

                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing an array with an illegal type\n");
                     i = (sp - 1)->u.number;
                     if (i<0) error("Negative index passed to array.\n");
                     vec = sp->u.vec;
                     if (i >= vec->size) error("Array index out of bounds.\n");
                     assign_svalue_no_free(--sp, &vec->item[i]);
                     free_vector(vec);
                     break;
                 }
		 default:
                 error("Indexing on illegal type.\n");
             }

             /*
              * Fetch value of a variable. It is possible that it is a
              * variable that points to a destructed object. In that case,
              * it has to be replaced by 0.
              */
             if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
                 free_object(sp->u.ob, "F_INDEX");
                 sp->type = T_NUMBER;
                 sp->u.number = 0;
             }
             break;
	 case F_RINDEX:
             switch (sp->type) {
		 case T_BUFFER:
                 {
                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing a buffer with an illegal type.\n");

                     i = sp->u.buf->size - (sp - 1)->u.number;
                     if ((i > sp->u.buf->size) || (i < 0))
                         error("Buffer index out of bounds.\n");

                     i = sp->u.buf->item[i];
                     free_buffer(sp->u.buf);
                     (--sp)->u.number = i;
                     break;
                 }
		 case T_STRING:
                 {
                     int len = SVALUE_STRLEN(sp);
                     if ((sp-1)->type != T_NUMBER) {
                         error("Indexing a string with an illegal type.\n");
                     }
                     i = len - (sp - 1)->u.number;
                     if ((i > len) || (i < 0))
                         error("String index out of bounds.\n");
                     i = (unsigned char) sp->u.string[i];
                     free_string_svalue(sp);
                     (--sp)->u.number = i;
                     break;
                 }
#ifndef NEW_FUNCTIONS
		 case T_FUNCTION:
                 {
                     struct funp *f;

                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing a function with an illegal type\n");
                     i = 2 - (sp - 1)->u.number;
                     if (i<0 || i>1)
                         error("Function variables may only be indexed with 0 or 1.\n");
                     f = sp->u.fp;
                     assign_svalue_no_free(--sp, i ? &f->fun : &f->obj);
                     free_funp(f);
                     break;
                 }
#endif
		 case T_POINTER:
                 {
                     struct vector *vec = sp->u.vec;

                     if ((sp-1)->type != T_NUMBER)
                         error("Indexing an array with an illegal type\n");
                     i = vec->size - (sp - 1)->u.number;
                     if (i<0) error("Negative index passed to array.\n");
                     if (i >= vec->size) error("Array index out of bounds.\n");
                     assign_svalue_no_free(--sp, &vec->item[i]);
                     free_vector(vec);
                     break;
                 }
		 default:
                 error("Indexing from the right on illegal type.\n");
             }

             /*
              * Fetch value of a variable. It is possible that it is a
              * variable that points to a destructed object. In that case,
              * it has to be replaced by 0.
              */
             if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
                 free_object(sp->u.ob, "F_RINDEX");
                 sp->type = T_NUMBER;
                 sp->u.number = 0;
             }
             break;
#ifdef F_JUMP_WHEN_ZERO
	case F_JUMP_WHEN_ZERO:
	    if ((i = (sp->type == T_NUMBER)) && sp->u.number == 0) {
		COPY_SHORT(&offset, pc);
		pc = current_prog->p.i.program + offset;
	    } else {
		pc += 2;
	    }
	    if (i) {
		sp--;	/* cheaper to do this when sp is an integer
			 * svalue */
	    } else {
		pop_stack();
	    }
	    break;
#endif
#ifdef F_JUMP
	case F_JUMP:
	     COPY_SHORT(&offset, pc);
	     pc = current_prog->p.i.program + offset;
	     break;
#endif
	 case F_LE:
	     {
                 i = sp->type;
                 switch((--sp)->type|i){
		 case T_NUMBER:
		     sp->u.number = sp->u.number <= (sp+1)->u.number;
		     break;
		     
		 case T_REAL:
		     sp->u.number = sp->u.real <= (sp+1)->u.real;
		     sp->type = T_NUMBER;
		     break;
		     
		 case T_NUMBER|T_REAL:
		     if (i & T_NUMBER){
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
			     bad_argument(sp, T_NUMBER | T_REAL, 2, instruction);
			     
			 case T_STRING:
			     bad_argument(sp, T_STRING, 2, instruction);
			     
			 default:
			     bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, instruction);
			 }
		     }
		 }
		 break;
	     }
	     break;
	 case F_LSH:
	     f_lsh();
	     break;
	 case F_LSH_EQ:
	     f_lsh_eq();
	     break;
	 case F_MOD:
	     {
		 CHECK_TYPES(sp - 1, T_NUMBER, 1, instruction);
		 CHECK_TYPES(sp, T_NUMBER, 2, instruction);
		 if ((sp--)->u.number == 0)
		     error("Modulus by zero.\n");
		 sp->u.number %= (sp+1)->u.number;
	     }
	     break;
	 case F_MOD_EQ:
	     f_mod_eq();
	     break;
	 case F_MULTIPLY:
	     {
		 switch((sp-1)->type|sp->type){
		 case T_NUMBER:
		     {
			 sp--;
			 sp->u.number *= (sp+1)->u.number;
			 break;
		     }
		     
		 case T_REAL:
		     {
			 sp--;
			 sp->u.real *= (sp+1)->u.real;
			 break;
		     }
		     
		 case T_NUMBER|T_REAL:
		     {
			 if ((--sp)->type & T_NUMBER){
			     sp->type = T_REAL;
			     sp->u.real = sp->u.number * (sp+1)->u.real;
			 }
			 else sp->u.real *= (sp+1)->u.number;
			 break;
		     }
		     
		 case T_MAPPING:
		     {
			 struct mapping *m;
			 m = compose_mapping((sp-1)->u.map, sp->u.map, 1);
			 pop_2_elems();
			 (++sp)->type = T_MAPPING;
			 sp->u.map = m;
			 break;
		     }
		     
		 default:
		     {
			 if (!((sp-1)->type & (T_NUMBER|T_REAL|T_MAPPING)))
			     bad_argument(sp-1, T_NUMBER|T_REAL|T_MAPPING,1, instruction);
			 if (!(sp->type & (T_NUMBER|T_REAL|T_MAPPING)))
			     bad_argument(sp, T_NUMBER|T_REAL|T_MAPPING,2, instruction);
			 error("Args to * are not compatible.\n");
		     }
		 }
	     }
	     break;
	 case F_MULT_EQ:
	     f_mult_eq();
	     break;
	 case F_NE:
	     f_ne();
	     break;
	 case F_NEGATE:
	     if (sp->type == T_NUMBER)
		 sp->u.number = -sp->u.number;
	     else if (sp->type == T_REAL)
		 sp->u.real = -sp->u.real;
	     else
		 error("Bad argument to unary minus\n");
	     break;
	case F_NOT:
	    if (sp->type == T_NUMBER)
		sp->u.number = !sp->u.number;
	    else
		assign_svalue(sp, &const0);
	    break;
	case F_OR:
	    f_or();
	    break;
	case F_OR_EQ:
	    f_or_eq();
	    break;
	case F_PARSE_COMMAND:
	    f_parse_command();
	    break;
	case F_POP_VALUE:
	    pop_stack();
	    break;
	case F_POP_BREAK:
	     break_sp += EXTRACT_UCHAR(pc++);
	     break;
	case F_POST_DEC:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to --\n");
	     lval = sp->u.lvalue;
	     switch(lval->type) {
	     case T_NUMBER:
		 sp->type = T_NUMBER;
		 sp->u.number = lval->u.number--;
		 break;
	     case T_REAL:
		 sp->type = T_REAL;
		 sp->u.real = lval->u.real--;
		 break;
	     case T_LVALUE_BYTE:
		 sp->type = T_NUMBER;
		 sp->u.number = (*glb.u.lvalue_byte)--;
		 break;
	     default:
		error("-- of non-numeric argument\n");
	     }
	     break;
	 case F_POST_INC:
	     DEBUG_CHECK(sp->type != T_LVALUE,
			 "non-lvalue argument to ++\n");
	     lval = sp->u.lvalue;
	     switch (lval->type) {
	     case T_NUMBER:
		 sp->type = T_NUMBER;
		 sp->u.number = lval->u.number++;
		 break;
	     case T_REAL:
		 sp->type = T_REAL;
		 sp->u.real = lval->u.real++;
		 break;
	     case T_LVALUE_BYTE:
		 sp->type = T_NUMBER;
		 sp->u.number = (*glb.u.lvalue_byte)++;
		 break;
	     default:
		 error("++ of non-numeric argument\n");
	     }
	     break;
	case F_GLOBAL_LVALUE:
	     (++sp)->type = T_LVALUE;
	     sp->u.lvalue = find_value((int) (EXTRACT_UCHAR(pc++) +
					      variable_index_offset));
	     break;
	case F_INDEX_LVALUE:
             push_indexed_lvalue(0);
             break;
	case F_RINDEX_LVALUE:
             push_indexed_lvalue(1);
             break;
	case F_NN_RANGE_LVALUE:
             push_lvalue_range(0x00);
             break;
	case F_RN_RANGE_LVALUE:
             push_lvalue_range(0x10);
             break;
	case F_RR_RANGE_LVALUE:
             push_lvalue_range(0x11);
             break;
	case F_NR_RANGE_LVALUE:
             push_lvalue_range(0x01);
             break;
	case F_NN_RANGE:
             f_range(0x00);
             break;
	case F_RN_RANGE:
             f_range(0x10);
             break;
	case F_NR_RANGE:
             f_range(0x01);
             break;
	case F_RR_RANGE:
             f_range(0x11);
             break;
	case F_NE_RANGE:
             f_extract_range(0);
             break;
	case F_RE_RANGE:
             f_extract_range(1);
             break;
	case F_RETURN:
	    {
		struct svalue sv;
		
		sv = *sp--;
		/*
		 * Deallocate frame and return.
		 */
		pop_n_elems(csp->num_local_variables);
		sp++;
		DEBUG_CHECK(sp != fp, "Bad stack at F_RETURN\n");
		*sp = sv;	/* This way, the same ref counts are
				 * maintained */
		pop_control_stack();
#ifdef TRACE
		tracedepth--;
		if (TRACEP(TRACE_RETURN)) {
		    do_trace("Return", "", "");
		    if (TRACEHB) {
			if (TRACETST(TRACE_ARGS)) {
			    add_message(" with value: ");
			    print_svalue(sp);
			}
			add_message("\n");
		    }
		}
#endif
		/* The control stack was popped just before */
		if (csp[1].extern_call)
		    return;
		break;
	    }
	case F_RSH:
	    f_rsh();
	    break;
	case F_RSH_EQ:
	    f_rsh_eq();
	    break;
	case F_SSCANF:
	    f_sscanf();
	    break;
	case F_STRING:
	     LOAD_SHORT(offset, pc);
	     DEBUG_CHECK1(offset >= current_prog->p.i.num_strings,
			  "string %d out of range in F_STRING!\n",
			  offset);
	     (++sp)->type = T_STRING;
	     sp->subtype = STRING_SHARED;
	     sp->u.string = ref_string(current_prog->p.i.strings[offset]);
	     break;
	case F_SHORT_STRING:
	     DEBUG_CHECK1(EXTRACT_UCHAR(pc) >= current_prog->p.i.num_strings,
                          "string %d out of range in F_STRING!\n",
			  EXTRACT_UCHAR(pc));
	     (++sp)->type = T_STRING;
	     sp->subtype = STRING_SHARED;
	     sp->u.string = ref_string(current_prog->p.i.strings[EXTRACT_UCHAR(pc++)]);
	     break;
	case F_SUBTRACT:
	    {
		i = (sp--)->type;
		switch (i | sp->type) {
		case T_NUMBER:
		    sp->u.number -= (sp+1)->u.number;
		    break;

		case T_REAL:
		    sp->u.real -= (sp+1)->u.real;
		    break;

		case T_NUMBER | T_REAL:
		    if (sp->type & T_REAL) sp->u.real -= (sp+1)->u.number;
		    else {
			sp->type = T_REAL;
			sp->u.real = sp->u.number - (sp+1)->u.real;
		    }
		    break;

		case T_POINTER:
		    {
			/*
			 * subtract_array already takes care of
			 * destructed objects
			 */
			sp->u.vec = subtract_array(sp->u.vec, (sp+1)->u.vec);
			break;
		    }
		    
		default:
		    if (!((sp++)->type & (T_NUMBER|T_REAL|T_POINTER)))
			error("Bad left type to -.\n");
		    else if (!(sp->type & (T_NUMBER|T_REAL|T_POINTER)))
			error("Bad right type to -.\n");
		    else error("Arguments to - do not have compatible types.\n");
		}
		break;
	    }
	case F_SUB_EQ:
	    f_sub_eq();
	    break;
	case F_SIMUL_EFUN:
	    f_simul_efun();
	    break;
	case F_SWITCH:
	    f_switch();
	    break;
	case F_XOR:
	    f_xor();
	    break;
	case F_XOR_EQ:
	    f_xor_eq();
	    break;
	case F_CATCH:
	    {
		/*
		 * Compute address of next instruction after the CATCH
		 * statement.
		 */
		((char *) &offset)[0] = pc[0];
		((char *) &offset)[1] = pc[1];
		offset = pc + offset - current_prog->p.i.program;
		pc += 2;
		
		do_catch(pc, offset);
		
		pc = current_prog->p.i.program + offset;
		
		break;
	    }
	case F_END_CATCH:
	    {
		pop_stack();/* discard expression value */
		free_svalue(&catch_value, "F_END_CATCH");
		catch_value.type = T_NUMBER;
		catch_value.u.number = 0;
		/* We come here when no longjmp() was executed */
		pop_control_stack();
		push_pop_error_context(0);
		push_number(0);
		return;	/* return to do_catch */
	    }
	case F_TIME_EXPRESSION:
	    {
		long sec, usec;
		
		get_usec_clock(&sec, &usec);
		push_number(sec);
		push_number(usec);
		break;
	    }
	case F_END_TIME_EXPRESSION:
	    {
		long sec, usec;
		
		get_usec_clock(&sec, &usec);
		usec = (sec - (sp - 2)->u.number) * 1000000 + (usec - (sp - 1)->u.number);
		pop_stack();
		sp -= 2;
		push_number(usec);
		break;
	    }
#ifdef NEEDS_CALL_EXTRA
	case F_CALL_EXTRA:
	    instruction = EXTRACT_UCHAR(pc++) + 0xff;
#endif
	default:
	    /* We have an efun.  Execute it
	     * Check the types of the first two args first
	     */
	    if (instrs[instruction].min_arg != instrs[instruction].max_arg)
		st_num_arg = EXTRACT_UCHAR(pc++);
	    else
		st_num_arg = instrs[instruction].min_arg;
#ifdef DEBUG
	    if (instruction > NUM_OPCODES) {
		debug_fatal("Undefined instruction %s (%d)\n",
			    get_f_name(instruction), instruction);
	    }
	    if (instruction < BASE) {
		debug_fatal("No case for eoperator %s (%d)\n",
			    get_f_name(instruction), instruction);
	    }
	    expected_stack = sp - st_num_arg + 1;
#endif
	    if (st_num_arg > 0) {
		CHECK_TYPES(sp - st_num_arg + 1, instrs[instruction].type[0], 1, instruction);
		if (st_num_arg > 1) {
		    CHECK_TYPES(sp - st_num_arg + 2, instrs[instruction].type[1], 2, instruction);
		}
	    }
#ifdef DEBUG
	    num_arg = st_num_arg;
#endif

	    (*oefun_table[instruction]) ();
	    DEBUG_CHECK2(expected_stack != sp,
		 "Bad stack after evaluation. Instruction %d, num arg %d\n",
			 instruction, num_arg);
	}  /* switch (instruction) */
	DEBUG_CHECK2(sp < fp + csp->num_local_variables - 1,
		 "Bad stack after evaluation. Instruction %d, num arg %d\n",
		     instruction, num_arg);
    }   /* while (1) */
}

static void
do_catch P2(char *, pc, unsigned short, new_pc_offset)
{
    push_control_stack(FRAME_CATCH, 0);
    /* next two probably not necessary... */
    csp->pc = current_prog->p.i.program + new_pc_offset;
    csp->num_local_variables = (csp - 1)->num_local_variables;	/* marion */
    /*
     * Save some global variables that must be restored separately after a
     * longjmp. The stack will have to be manually popped all the way.
     */
    push_pop_error_context(1);

    /* signal catch OK - print no err msg */
    error_recovery_context_exists = CATCH_ERROR_CONTEXT;
    if (SETJMP(error_recovery_context)) {
	/*
	 * They did a throw() or error. That means that the control stack
	 * must be restored manually here.
	 */
	push_pop_error_context(-1);
	pop_control_stack();
	sp++;
	*sp = catch_value;
	assign_svalue_no_free(&catch_value, &const1);
	
	/* if it's too deep or max eval, we can't let them catch it */
	if (max_eval_error)
	    error("Can't catch eval cost too big error.\n");
	if (too_deep_error)
	    error("Can't catch too deep recursion error.\n");
    } else {
	assign_svalue(&catch_value, &const1);
	/* note, this will work, since csp->extern_call won't be used */
	eval_instruction(pc);
    }
}

/*
 * Apply a fun 'fun' to the program in object 'ob', with
 * 'num_arg' arguments (already pushed on the stack).
 * If the function is not found, search in the object pointed to by the
 * inherit pointer.
 * If the function name starts with '::', search in the object pointed out
 * through the inherit pointer by the current object. The 'current_object'
 * stores the base object, not the object that has the current function being
 * evaluated. Thus, the variable current_prog will normally be the same as
 * current_object->prog, but not when executing inherited code. Then,
 * it will point to the code of the inherited object. As more than one
 * object can be inherited, the call of function by index number has to
 * be adjusted. The function number 0 in a superclass object must not remain
 * number 0 when it is inherited from a subclass object. The same problem
 * exists for variables. The global variables function_index_offset and
 * variable_index_offset keep track of how much to adjust the index when
 * executing code in the superclass objects.
 *
 * There is a special case when called from the heart beat, as
 * current_prog will be 0. When it is 0, set current_prog
 * to the 'ob->prog' sent as argument.
 *
 * Arguments are always removed from the stack.
 * If the function is not found, return 0 and nothing on the stack.
 * Otherwise, return 1, and a pushed return value on the stack.
 *
 * Note that the object 'ob' can be destructed. This must be handled by
 * the caller of apply().
 *
 * If the function failed to be called, then arguments must be deallocated
 * manually !  (Look towards end of this function.)
 */

#ifdef DEBUG
static char debug_apply_fun[30];/* For debugging */
#endif

#ifdef CACHE_STATS
unsigned int apply_low_call_others = 0;
unsigned int apply_low_cache_hits = 0;
unsigned int apply_low_slots_used = 0;
unsigned int apply_low_collisions = 0;
#endif

typedef struct cache_entry_s {
    int id;
    char *name;
    struct function *pr;
    struct function *pr_inherited;
    struct program *progp;
    struct program *oprogp;
    int function_index_offset;
    int variable_index_offset;
}             cache_entry_t;

static cache_entry_t cache[APPLY_CACHE_SIZE];

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_apply_low_cache() {
    int i;
    for (i = 0; i < APPLY_CACHE_SIZE; i++) {
	if (cache[i].id && !cache[i].progp)
	    EXTRA_REF(BLOCK(cache[i].name))++;
    }
}
#endif

int apply_low P3(char *, fun, struct object *, ob, int, num_arg)
{
    /*
     * static memory is initialized to zero by the system or so Jacques says
     * :)
     */
    cache_entry_t *entry;
    struct function *pr;
    struct program *progp;
    int ix;
    static int cache_mask = APPLY_CACHE_SIZE - 1;
    char *funname;
    int i, j;
    int local_call_origin = call_origin;
    IF_DEBUG(struct control_stack *save_csp);

    if (!local_call_origin)
	local_call_origin = ORIGIN_DRIVER;
    call_origin = 0;
    ob->time_of_ref = current_time;	/* Used by the swapper */
    /*
     * This object will now be used, and is thus a target for reset later on
     * (when time due).
     */
#ifdef LAZY_RESETS
    try_reset(ob);
    if ((ob->flags & O_DESTRUCTED) && (num_error <= 0)) {
	pop_n_elems(num_arg);
	return 0;
    }
#endif
    ob->flags &= ~O_RESET_STATE;
#ifdef DEBUG
    strncpy(debug_apply_fun, fun, sizeof(debug_apply_fun));
    debug_apply_fun[sizeof debug_apply_fun - 1] = '\0';
#endif
    if (num_error <= 0) {	/* !failure */
	/*
	 * If there is a chain of objects shadowing, start with the first of
	 * these.
	 */
#ifndef NO_SHADOWS
	while (ob->shadowed && ob->shadowed != current_object)
	    ob = ob->shadowed;
#endif
      retry_for_shadow:
	if (ob->flags & O_SWAPPED)
	    load_ob_from_swap(ob);
	progp = ob->prog;
	DEBUG_CHECK(ob->flags & O_DESTRUCTED,"apply() on destructed object\n");
#ifdef CACHE_STATS
	apply_low_call_others++;
#endif
	ix = (progp->p.i.id_number ^ (int) fun ^
	      ((int) fun >> APPLY_CACHE_BITS)) & cache_mask;
	entry = &cache[ix];
	if ((entry->id == progp->p.i.id_number)
	    && (!entry->progp || (entry->oprogp == ob->prog))
	    && !strcmp(entry->name, fun)) {
	    /*
	     * We have found a matching entry in the cache. The pointer to
	     * the function name has to match, not only the contents. This is
	     * because hashing the string in order to get a cache index would
	     * be much more costly than hashing it's pointer. If cache access
	     * would be costly, the cache would be useless.
	     */
#ifdef CACHE_STATS
	    apply_low_cache_hits++;
#endif
	    if (entry->progp 
		&& (!(entry->pr->type & (TYPE_MOD_STATIC | TYPE_MOD_PRIVATE))
		    || current_object == ob || (local_call_origin & (ORIGIN_DRIVER | ORIGIN_CALL_OUT)))) {
		/*
		 * the cache will tell us in which program the function is,
		 * and where
		 */
	        push_control_stack(FRAME_FUNCTION, entry->pr);
		caller_type = local_call_origin;
		csp->num_local_variables = num_arg;
		current_prog = entry->progp;
		pr = entry->pr_inherited;
		function_index_offset = entry->function_index_offset;
		variable_index_offset = entry->variable_index_offset;
		/* Remove excessive arguments */
		if ((i = csp->num_local_variables - (int) pr->num_arg) > 0) {
		    pop_n_elems(i);
		    push_nulls(j = pr->num_local);
		} else {
		    /* Correct number of arguments and local variables */
		    j = pr->num_local;
		    push_nulls(j - i);
		}
		fp = sp - (csp->num_local_variables = pr->num_arg + j) + 1;
#ifdef TRACE
		tracedepth++;
		if (TRACEP(TRACE_CALL)) {
		    do_trace_call(pr);
		}
#endif

#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
		/*
		 * Now, previous_object() is always set, even by
		 * call_other(this_object()). It should not break any
		 * compatibility.
		 */
		if (current_object != ob)
#endif
		    previous_ob = current_object;
		current_object = ob;
		IF_DEBUG(save_csp = csp);
		call_program(current_prog, pr->offset);

		DEBUG_CHECK(save_csp - 1 != csp, 
			    "Bad csp after execution in apply_low.\n");
		/*
		 * Arguments and local variables are now removed. One
		 * resulting value is always returned on the stack.
		 */
		return 1;
	    }			/* when we come here, the cache has told us
				 * that the function isn't defined in the
				 * object */
	} else {
	    /* we have to search the function */
	    if (!entry->progp && entry->id) {
		/*
		 * The old cache entry was for an undefined function, so the
		 * name had to be allocated
		 */
		free_string(entry->name);
	    }
#ifdef CACHE_STATS
	    if (!entry->id) {
		apply_low_slots_used++;
	    } else {
		apply_low_collisions++;
	    }
#endif
	    /*
	     * All functions are shared strings.  Searching the hash table
	     * will typically take less than three string compares.  If the
	     * string isn't in the hash table then the object contains no
	     * function by that name. If the string is in the hash table then
	     * we can search for the string in the object by comparing
	     * pointers rather than using strcmp's (since shared strings are
	     * unique).  The idea for this optimization comes from the
	     * lp-strcmpoptim file on alcazar.
	     */
	    if ((funname = findstring(fun))) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
		i = lookup_function(progp->p.i.functions,
				    progp->p.i.tree_r, funname);
		if (i == -1 || 
		    (progp->p.i.functions[i].flags & NAME_UNDEFINED)
		    || ((progp->p.i.functions[i].type & (TYPE_MOD_STATIC | TYPE_MOD_PRIVATE))
			&& current_object != ob && !(local_call_origin & (ORIGIN_DRIVER | ORIGIN_CALL_OUT)))) {
		    ;
		} else {
		    pr = (struct function *) & progp->p.i.functions[i];
#else
		/* comparing pointers okay since both are shared strings */
		for (pr = progp->p.i.functions;
		     pr < progp->p.i.functions + progp->p.i.num_functions;
		     pr++) {
		    if (pr->name == 0 ||
			pr->name != funname) 
			continue;
		    if (pr->flags & NAME_UNDEFINED)
			continue;
		    /* Static functions may not be called from outside. */
		    if ((pr->type & (TYPE_MOD_STATIC | TYPE_MOD_PRIVATE)) &&
			current_object != ob && !(local_call_origin & (ORIGIN_DRIVER | ORIGIN_CALL_OUT))) {
			continue;
		    }
#endif
		    push_control_stack(FRAME_FUNCTION, pr);
		    caller_type = local_call_origin;
		    /* The searched function is found */
		    entry->id = progp->p.i.id_number;
		    entry->pr = pr;
		    entry->name = pr->name;
		    csp->num_local_variables = num_arg;
		    current_prog = progp;
		    entry->oprogp = current_prog;	/* before
							 * setup_new_frame */
		    pr = setup_new_frame(pr);
		    entry->pr_inherited = pr;
		    entry->progp = current_prog;
		    entry->variable_index_offset = variable_index_offset;
		    entry->function_index_offset = function_index_offset;
#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
		    if (current_object != ob)
#endif
			previous_ob = current_object;
		    current_object = ob;
		    IF_DEBUG(save_csp = csp);
		    call_program(current_prog, pr->offset);

		    DEBUG_CHECK(save_csp - 1 != csp,
				"Bad csp after execution in apply_low\n");
		    /*
		     * Arguments and local variables are now removed. One
		     * resulting value is always returned on the stack.
		     */
		    return 1;
		}
	    }
	    /* We have to mark a function not to be in the object */
	    entry->id = progp->p.i.id_number;
	    /* Beek - 99% of the time it's a shared string already */
	    entry->name = make_shared_string(fun);
	    entry->progp = (struct program *) 0;
	}
#ifndef NO_SHADOWS
	if (ob->shadowing) {
	    /*
	     * This is an object shadowing another. The function was not
	     * found, but can maybe be found in the object we are shadowing.
	     */
	    ob = ob->shadowing;
	    goto retry_for_shadow;
	}
#endif
    }				/* !failure */
    /* Failure. Deallocate stack. */
    pop_n_elems(num_arg);
    return 0;
}

/*
 * Arguments are supposed to be
 * pushed (using push_string() etc) before the call. A pointer to a
 * 'struct svalue' will be returned. It will be a null pointer if the called
 * function was not found. Otherwise, it will be a pointer to a static
 * area in apply(), which will be overwritten by the next call to apply.
 * Reference counts will be updated for this value, to ensure that no pointers
 * are deallocated.
 */

static struct svalue *sapply P3(char *, fun, struct object *, ob, int, num_arg)
{
    IF_DEBUG(struct svalue *expected_sp);

#ifdef TRACE
    if (TRACEP(TRACE_APPLY)) {
	do_trace("Apply", "", "\n");
    }
#endif

    IF_DEBUG(expected_sp = sp - num_arg);
    if (apply_low(fun, ob, num_arg) == 0)
	return 0;
    free_svalue(&apply_ret_value, "sapply");
    apply_ret_value = *sp--;
    DEBUG_CHECK(expected_sp != sp,
		"Corrupt stack pointer.\n");
    return &apply_ret_value;
}

struct svalue *apply P4(char *, fun, struct object *, ob, int, num_arg,
			int, where)
{
    tracedepth = 0;
    call_origin = where;
    return sapply(fun, ob, num_arg);
}

/* Reason for the following 1. save cache space 2. speed :) */
/* The following is to be called only from reset_object for */
/* otherwise extra checks are needed - Sym                  */

void call___INIT P1(struct object *, ob)
{
    struct function *pr;
    struct program *progp;
    int num_functions;
    IF_DEBUG(struct svalue *expected_sp);
    IF_DEBUG(struct control_stack *save_csp);

    tracedepth = 0;
    
#ifdef TRACE
    if (TRACEP(TRACE_APPLY)) {
        do_trace("Apply", "", "\n");
    }
#endif

    IF_DEBUG(expected_sp = sp);
    
    /* No try_reset here for obvious reasons :) */

    ob->flags &= ~O_RESET_STATE;

    progp = ob->prog;
    num_functions = progp->p.i.num_functions;
    if (!num_functions) return;

    /* ___INIT turns out to be always the last function */
    pr = progp->p.i.functions + num_functions - 1;
    if (*pr->name != APPLY___INIT_SPECIAL_CHAR) return;
    push_control_stack(FRAME_FUNCTION, pr);
    caller_type = ORIGIN_DRIVER;
    csp->num_local_variables = 0;
    current_prog = progp;

    pr = setup_new_frame(pr);
#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
    if (current_object != ob)
#endif
	previous_ob = current_object;
    
    current_object = ob;
    IF_DEBUG(save_csp = csp);
    call_program(current_prog, pr->offset);

    DEBUG_CHECK(save_csp - 1 != csp,
		"Bad csp after execution in apply_low\n");
    sp--;
    DEBUG_CHECK(expected_sp != sp,
                "Corrupt stack pointer.\n");
}

/*
 * this is a "safe" version of apply
 * this allows you to have dangerous driver mudlib dependencies
 * and not have to worry about causing serious bugs when errors occur in the
 * applied function and the driver depends on being able to do something
 * after the apply. (such as the ed exit function, and the net_dead function).
 * note: this function uses setjmp() and thus is fairly expensive when
 * compared to a normal apply().  Use sparingly.
 */

struct svalue *
       safe_apply P4(char *, fun, struct object *, ob, int, num_arg, int, where)
{
    jmp_buf save_error_recovery_context;
    struct svalue *ret;
    struct object *save_command_giver = command_giver;
    struct svalue *old_sp = sp;
    struct control_stack *old_csp = csp;
    int old_error_context_exists = error_recovery_context_exists;

    debug(32768, ("safe_apply: before sp = %d\n", sp));
    debug(32768, ("safe_apply: before csp = %d\n", csp));
    memcpy((char *) save_error_recovery_context,
	   (char *) error_recovery_context, sizeof(error_recovery_context));
    error_recovery_context_exists = SAFE_APPLY_ERROR_CONTEXT;
    if (!SETJMP(error_recovery_context)) {
	if (!(ob->flags & O_DESTRUCTED)) {
	    ret = apply(fun, ob, num_arg, where);
	} else ret = NULL;
    } else {
#if 0
	/*
	 * this shouldn't be needed here (would likely cause problems).
	 * 0.9.17.5 - tru
	 */
	clear_state();
#else
	/*
	 * Think it shouldn't be done at all
	 * v20.25a4 - Sym
	 */

	csp = old_csp + 1;
	pop_control_stack();
	pop_n_elems(sp - old_sp + num_arg);
#endif
	ret = NULL;
	fprintf(stderr, "Warning: Error in the '%s' function in '%s'\n",
		fun, ob->name);
	fprintf(stderr,
		"The driver may function improperly if this problem is not fixed.\n");
    }
    debug(32768, ("safe_apply: after sp = %d\n", sp));
    debug(32768, ("safe_apply: after csp = %d\n", csp));
    error_recovery_context_exists = old_error_context_exists;
    memcpy((char *) error_recovery_context,
      (char *) save_error_recovery_context, sizeof(error_recovery_context));
    command_giver = save_command_giver;
    return ret;
}

/*
 * Call a function in all objects in a vector.
 */
struct vector *call_all_other P3(struct vector *, v, char *, func, int, numargs)
{
    int idx, size;
    struct svalue *tmp;
    struct vector *ret;
    struct object *ob;
    int i;

    tmp = sp;
    (++sp)->type = T_POINTER;
    sp->u.vec = ret = allocate_array(size = v->size);
    if (size && (sp + numargs >= end_of_stack)) {
	too_deep_error = 1;
	error("stack overflow\n");
    }
    for (idx = 0; idx < size; idx++) {
	if (v->item[idx].type != T_OBJECT)
	    continue;
	ob = v->item[idx].u.ob;
	if (ob->flags & O_DESTRUCTED)
	    continue;
	for (i = numargs; i--;)
	    push_svalue(tmp - i);
	call_origin = ORIGIN_CALL_OTHER;
	if (apply_low(func, ob, numargs)) {
	    assign_svalue_no_free(&ret->item[idx], sp);
	    pop_stack();
	}
    }
    sp--;
    pop_n_elems(numargs);
    return ret;
}

/*
 * This function is similar to apply(), except that it will not
 * call the function, only return object name if the function exists,
 * or 0 otherwise.
 */
char *function_exists P2(char *, fun, struct object *, ob)
{
    struct function *pr;
    char *funname;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    int i;

#endif

    DEBUG_CHECK(ob->flags & O_DESTRUCTED,
		"function_exists() on destructed object\n");

    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    pr = ob->prog->p.i.functions;
    /* all function names are in the shared string table */
    if ((funname = findstring(fun))) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	i = lookup_function(ob->prog->p.i.functions,
			    ob->prog->p.i.tree_r, funname);
	if (i != -1) {
	    struct program *progp;

	    pr = (struct function *) & ob->prog->p.i.functions[i];
	    if ((pr->flags & NAME_UNDEFINED) ||
		((pr->type & TYPE_MOD_STATIC) && current_object != ob))
		return 0;
#else
	for (; pr < ob->prog->p.i.functions + ob->prog->p.i.num_functions; pr++) {
	    struct program *progp;

	    /* okay to compare pointers since both are shared strings */
	    if (funname != pr->name)
		continue;
	    /* Static functions may not be called from outside. */
	    if ((pr->type & TYPE_MOD_STATIC) && current_object != ob)
		continue;
	    if (pr->flags & NAME_UNDEFINED)
		return 0;
#endif
	    for (progp = ob->prog; pr->flags & NAME_INHERITED;) {
		progp = progp->p.i.inherit[pr->offset].prog;
		pr = &progp->p.i.functions[pr->function_index_offset];
	    }
	    return progp->name;
	}
    }
    return 0;
}

#ifndef NO_SHADOWS
/*
  is_static: returns 1 if a function named 'fun' is declared 'static' in 'ob';
  0 otherwise.
*/

int is_static(fun, ob)
    char *fun;
    struct object *ob;
{
    char *funname;

    DEBUG_CHECK(ob->flags & O_DESTRUCTED,
		"is_static() on destructed object\n");
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    /* all function names are in the shared string table */
    if ((funname = findstring(fun))) {
	struct function *pr;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	int i;

	i = lookup_function(ob->prog->p.i.functions,
			    ob->prog->p.i.tree_r, funname);
	if (i != -1) {
	    pr = (struct function *) & ob->prog->p.i.functions[i];
#else
	struct function *limit;

	limit = ob->prog->p.i.functions + ob->prog->p.i.num_functions;
	for (pr = ob->prog->p.i.functions; pr < limit; pr++) {
	    /* okay to compare pointers since both are shared strings */
	    if (funname != pr->name)
		continue;
#endif
	    if (pr->flags & NAME_UNDEFINED)
		return 0;
	    if (pr->type & TYPE_MOD_STATIC)
		return 1;
	}
    }
    return 0;
}
#endif

/*
 * Call a specific function address in an object. This is done with no
 * frame set up. It is expected that there are no arguments. Returned
 * values are removed.
 */

void call_function P2(struct program *, progp, struct function *, pr)
{
    if (pr->flags & NAME_UNDEFINED)
	return;
    push_control_stack(FRAME_FUNCTION, pr);
    caller_type = ORIGIN_DRIVER;
    DEBUG_CHECK(csp != control_stack,
		"call_function with bad csp\n");
    csp->num_local_variables = 0;
    current_prog = progp;
    pr = setup_new_frame(pr);
    previous_ob = current_object;
    tracedepth = 0;
    call_program(current_prog, pr->offset);
    pop_stack();
}

void translate_absolute_line P4(int, abs_line, unsigned short *, file_info,
				int *, ret_file, int *, ret_line) {
    unsigned short *p1, *p2;
    int file;
    int line_tmp = abs_line;

    /* two passes: first, find out what file we're interested in */
    p1 = file_info;
    while (line_tmp > *p1) {
	line_tmp -= *p1;
	p1 += 2;
    }
    file = p1[1];

    /* now correct the line number for that file */
    p2 = file_info;
    while (p2 < p1) {
	if (p2[1] == file)
	    line_tmp += *p2;
	p2 += 2;
    }
    *ret_line = line_tmp;
    *ret_file = file;
}

int find_line P4(char *, p, struct program *, progp,
		 char **, ret_file, int *, ret_line )
{
    int offset;
    unsigned char *lns;
    short abs_line;
    int file_idx;

    *ret_file = "";
    *ret_line = 0;

    if (!progp) return 1;
#ifdef NEW_FUNCTIONS
    if (progp == &fake_prog) return 2;
#endif

#if defined(LPC_TO_C)
    /* currently no line number info for compiled programs */
    if (progp->p.i.program_size == 0) 
	return 3;
#endif
	
    /*
     * Load line numbers from swap if necessary.  Leave them in memory until
     * look_for_objects_to_swap() swaps them back out, since more errors are
     * likely.
     */
    if (!progp->p.i.line_info) {
	load_line_numbers(progp);
	if (!progp->p.i.line_info)
	    return 4;
    }
    offset = p - progp->p.i.program;
    DEBUG_CHECK2(offset > (int) progp->p.i.program_size,
		 "Illegal offset %d in object %s\n", offset, progp->name);

    lns = progp->p.i.line_info;
    while (offset > *lns) {
	offset -= *lns;
	lns += 3;
    }

    COPY_SHORT(&abs_line, lns + 1);

    translate_absolute_line(abs_line, &progp->p.i.file_info[2], 
			    &file_idx, ret_line);

    *ret_file = progp->p.i.strings[file_idx - 1];
    return 0;
}

void get_explicit_line_number_info P4(char *, p, struct program *, prog,
				      char **, ret_file, int *, ret_line) {
    find_line(p, prog, ret_file, ret_line);
    if (!(*ret_file))
	*ret_file = prog->name;
}

void get_line_number_info P2(char **, ret_file, int *, ret_line)
{
    find_line(pc, current_prog, ret_file, ret_line);
    if (!(*ret_file))
	*ret_file = current_prog->name;
}

static char* get_line_number P2(char *, p, struct program *, progp)
{
    static char buf[256];
    int i;
    char *file;
    int line;

    i = find_line(p, progp, &file, &line);
    
    switch (i) {
    case 1:
	strcpy(buf, "(no program)");
	return buf;
    case 2:	
	*buf = 0;
	return buf;
    case 3:
	strcpy(buf, "(compiled program)");
	return buf;
    case 4:
	strcpy(buf, "(no line numbers)");
	return buf;
    case 5:
	strcpy(buf, "(includes too deep)");
	return buf;
    }
    if (!file)
	file = progp->name;
    sprintf(buf, "/%s:%d", file, line);
    return buf;
}

/*
 * Write out a trace. If there is a heart_beat(), then return the
 * object that had that heart beat.
 */
char *dump_trace P1(int, how)
{
    struct control_stack *p;
    char *ret = 0;
    struct function *funp;

#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
    struct svalue *ptr;
    int i;
#endif

    if (current_prog == 0)
	return 0;
    if (csp < &control_stack[0]) {
	debug_message("No trace.\n");
	return 0;
    }
#ifdef TRACE_CODE
    if (how)
	(void) last_instructions();
#endif
    for (p = &control_stack[0]; p < csp; p++) {
      switch (p[0].framekind) {
      case FRAME_FUNCTION:
          debug_message("'%15s' in '%20s' ('%20s') %s\n",
		      p[0].fr.func->name, p[1].prog->name, p[1].ob->name,
		      get_line_number(p[1].pc, p[1].prog));
	  if (strcmp(p[0].fr.func->name, "heart_beat") == 0)
              ret = p->ob ? p->ob->name : 0;
          funp = p[0].fr.func;
          break;
#ifdef NEW_FUNCTIONS
      case FRAME_FUNP:
          debug_message("'     <function>' in '%20s' ('%20s') %s\n",
                        p[1].prog->name, p[1].ob->name,
                        get_line_number(p[1].pc, p[1].prog));
          funp = (struct function *)&p[0].fr.funp->f.a;
          break;
      case FRAME_FAKE:
          debug_message("'     <function>' in '%20s' ('%20s') %s\n",
                        p[1].prog->name, p[1].ob->name,
                        get_line_number(p[1].pc, p[1].prog));
          funp = 0;
          break;
#endif
      case FRAME_CATCH:
          debug_message("'          CATCH' in '%20s' ('%20s') %s\n",
                        p[1].prog->name, p[1].ob->name,
                        get_line_number(p[1].pc, p[1].prog));
          funp = 0;
          break;
      }
#ifdef ARGUMENTS_IN_TRACEBACK
      if (funp) {
	  ptr = p[1].fp;
          debug_message("arguments were (");
          for (i = 0; i < funp->num_arg; i++) {
		char *buf;

		if (i) {
		    debug_message(",");
		}
		buf = (char *) DMALLOC(50, TAG_TEMPORARY, "dump_trace:1");
		*buf = 0;
		svalue_to_string(&ptr[i], &buf, 50, 0, 0, 0);
		debug_message("%s", buf);
		FREE(buf);
	    }
	    debug_message(")\n");
      }
#endif
#ifdef LOCALS_IN_TRACEBACK
      if (funp && funp->num_local) {
	  ptr = p[1].fp + funp->num_arg;
	  debug_message("locals were: ");
	  for (i = 0; i < funp->num_local; i++) {
	      char *buf;
	      
	      if (i) {
		  debug_message(",");
	      }
	      buf = (char *) DMALLOC(50, TAG_TEMPORARY, "dump_trace:2");
	      *buf = 0;
		svalue_to_string(&ptr[i], &buf, 50, 0, 0, 0);
	      debug_message("%s", buf);
	      FREE(buf);
	  }	
	  debug_message("\n");
      }
#endif
    }
    switch (p[0].framekind) {
    case FRAME_FUNCTION:
      debug_message("'%15s' in '%20s' ('%20s') %s\n",
                    p[0].fr.func->name,
                    current_prog->name, current_object->name,
                    get_line_number(pc, current_prog));
      funp = p[0].fr.func;
      break;
#ifdef NEW_FUNCTIONS
    case FRAME_FUNP:
      debug_message("'     <function>' in '%20s' ('%20s') %s\n",
                    current_prog->name, current_object->name,
                    get_line_number(pc, current_prog));
      funp = (struct function *)&p[0].fr.funp->f.a;
      break;
    case FRAME_FAKE:
      debug_message("'     <function>' in '%20s' ('%20s') %s\n",
                    current_prog->name, current_object->name,
                    get_line_number(pc, current_prog));
      funp = 0;
      break;
#endif
    case FRAME_CATCH:
      debug_message("'          CATCH' in '%20s' ('%20s') %s\n",
                    current_prog->name, current_object->name,
                    get_line_number(pc, current_prog));
      funp = 0;
      break;
    }
#ifdef ARGUMENTS_IN_TRACEBACK
    if (funp) {
	debug_message("arguments were (");
	for (i = 0; i < funp->num_arg; i++) {
	    char *buf;

	    if (i) {
		debug_message(",");
	    }
	    buf = (char *) DMALLOC(50, TAG_TEMPORARY, "dump_trace:3");
	    *buf = 0;
	    svalue_to_string(&fp[i], &buf, 50, 0, 0, 0);
	    debug_message("%s", buf);
	    FREE(buf);
	}
	debug_message(")\n");
    }
#endif
#ifdef LOCALS_IN_TRACEBACK
    if (funp && funp->num_local) {
	ptr = fp + funp->num_arg;
	debug_message("locals were: ");
	for (i = 0; i < funp->num_local; i++) {
	    char *buf;

	    if (i) {
		debug_message(",");
	    }
	    buf = (char *) DMALLOC(50, TAG_TEMPORARY, "dump_trace:4");
	    *buf = 0;
	    svalue_to_string(&ptr[i], &buf, 50, 0, 0, 0);
	    debug_message("%s", buf);
	    FREE(buf);
	}
	debug_message("\n");
    }
#endif
    return ret;
}

struct vector *get_svalue_trace()
{
    struct control_stack *p;
    struct vector *v;
    struct mapping *m;
    char *file;
    int line;
    struct function *funp;

#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
    struct svalue *ptr;
    int i, n;
#ifdef LOCALS_IN_TRACEBACK
    int n2;
#endif
#endif

    if (current_prog == 0)
	return null_array();
    if (csp < &control_stack[0]) {
	return null_array();
    }
    v = allocate_empty_array((csp - &control_stack[0]) + 1);
    for (p = &control_stack[0]; p < csp; p++) {
	m = allocate_mapping(6);
        switch (p[0].framekind) {
	  case FRAME_FUNCTION:
	    add_mapping_string(m, "function", p[0].fr.func->name);
	    funp = p[0].fr.func;
	    break;
	  case FRAME_CATCH:
	    add_mapping_string(m, "function", "CATCH");
	    funp = 0;
	    break;
#ifdef NEW_FUNCTIONS
	  case FRAME_FAKE:
	    add_mapping_string(m, "function", "<function>");
	    funp = 0;
	    break;
	  case FRAME_FUNP:
	    add_mapping_string(m, "function", "<function>");
	    funp = (struct function *)&p[0].fr.funp->f.a;
	    break;
#endif
        }
	add_mapping_string(m, "program", p[1].prog->name);
	add_mapping_object(m, "object", p[1].ob);
	get_explicit_line_number_info(p[1].pc, p[1].prog, &file, &line);
	add_mapping_string(m, "file", file);
	add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
	if (funp) {
	    struct vector *v2;

	    n = funp->num_arg;
	    ptr = p[1].fp;
	    v2 = allocate_empty_array(n);
	    for (i = 0; i < n; i++) {
		assign_svalue_no_free(&v2->item[i], &ptr[i]);
	    }
	    add_mapping_array(m, "arguments", v2);
	    v2->ref--;
	}
#endif
#ifdef LOCALS_IN_TRACEBACK
	if (funp) {
	    struct vector *v2;

	    n = funp->num_arg;
	    n2 = funp->num_local;
	    ptr = p[1].fp;
	    v2 = allocate_empty_array(n2);
	    for (i = 0; i < n2; i++) {
		assign_svalue_no_free(&v2->item[i], &ptr[i + n]);
	    }
	    add_mapping_array(m, "locals", v2);
	    v2->ref--;
	}
#endif
	v->item[(p - &control_stack[0])].type = T_MAPPING;
	v->item[(p - &control_stack[0])].u.map = m;
    }
    m = allocate_mapping(6);
    switch (p[0].framekind) {
    case FRAME_FUNCTION:
      add_mapping_string(m, "function", p[0].fr.func->name);
      funp = p[0].fr.func;
      break;
    case FRAME_CATCH:
      add_mapping_string(m, "function", "CATCH");
      funp = 0;
      break;
#ifdef NEW_FUNCTIONS
    case FRAME_FAKE:
      add_mapping_string(m, "function", "<function>");
      funp = 0;
      break;
    case FRAME_FUNP:
      add_mapping_string(m, "function", "<function>");
      funp = (struct function *)&p[0].fr.funp->f.a;
      break;
#endif
    }
    add_mapping_string(m, "program", current_prog->name);
    add_mapping_object(m, "object", current_object);
    get_line_number_info(&file, &line);
    add_mapping_string(m, "file", file);
    add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
    if (funp) {
	struct vector *v2;

	n = funp->num_arg;
	v2 = allocate_empty_array(n);
	for (i = 0; i < n; i++) {
	    assign_svalue_no_free(&v2->item[i], &fp[i]);
	}
	add_mapping_array(m, "arguments", v2);
	v2->ref--;
    }
#endif
#ifdef LOCALS_IN_TRACEBACK
    if (funp) {
	struct vector *v2;

	n = funp->num_arg;
	n2 = funp->num_local;
	v2 = allocate_empty_array(n2);
	for (i = 0; i < n2; i++) {
	    assign_svalue_no_free(&v2->item[i], &fp[i + n]);
	}
	add_mapping_array(m, "locals", v2);
	v2->ref--;
    }
#endif
    v->item[(csp - &control_stack[0])].type = T_MAPPING;
    v->item[(csp - &control_stack[0])].u.map = m;
    /* return a reference zero vector */
    v->ref--;
    return v;
}

char * get_line_number_if_any()
{
    if (current_prog)
	return get_line_number(pc, current_prog);
    return 0;
}

#define SSCANF_ASSIGN_SVALUE_STRING(S) \
    arg->type = T_STRING; \
    arg->u.string = S; \
    arg->subtype = STRING_MALLOC; \
    arg--; \
    num_arg--

#define SSCANF_ASSIGN_SVALUE_NUMBER(N) \
    arg->type = T_NUMBER; \
    arg->subtype = 0; \
    arg->u.number = N; \
    arg--; \
    num_arg--

#define SSCANF_ASSIGN_SVALUE(T,U,V) \
    arg->type = T; \
    arg->U = V; \
    arg--; \
    num_arg--

/* arg points to the same place it used to */
int inter_sscanf P4(struct svalue *, arg, struct svalue *, s0, struct svalue *, s1, int, num_arg)
{
    char *fmt;			/* Format description */
    char *in_string;		/* The string to be parsed. */
    int number_of_matches;
    char *cp;
    int skipme = 0;		/* Encountered a '*' ? */

    /*
     * First get the string to be parsed.
     */
    CHECK_TYPES(s0, T_STRING, 1, F_SSCANF);
    in_string = s0->u.string;
    if (in_string == 0)
	return 0;

    /*
     * Now get the format description.
     */
    CHECK_TYPES(s1, T_STRING, 2, F_SSCANF);
    fmt = s1->u.string;

    /*
     * Loop for every % or substring in the format.
     */
    for (number_of_matches = 0; num_arg >= 0; number_of_matches++) {
	int i, type;

	while (fmt[0]) {
	    if (fmt[0] == '%' && fmt[1] == '%') {
		if (in_string[0] != '%')
		    return number_of_matches;
		fmt += 2;
		in_string++;
		continue;
	    }
	    if (fmt[0] == '%')
		break;
	    if (in_string[0] == '\0' || fmt[0] != in_string[0])
		return number_of_matches;
	    fmt++;
	    in_string++;
	}

	if (fmt[0] == '\0') {
	    /*
	     * We have reached end of the format string. If there are any
	     * chars left in the in_string, then we put them in the last
	     * variable (if any).
	     */
	    if (in_string[0] && num_arg > 0) {
		number_of_matches++;
		SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
	    }
	    break;
	}
	DEBUG_CHECK(fmt[0] != '%',
		    "In sscanf, should be a %% now !\n");
	skipme = 0;
	if (fmt[1] == '*') {
	    skipme++;
	    fmt++;
	}
	/*
	 * Hmm..maybe we should return number_of_matches here instead of
	 * error.
	 */
	if (skipme == 0 && num_arg < 1)
	    error("Too few arguments to sscanf()\n");

	switch (fmt[1]) {
	case 'd':
	    type = T_NUMBER;
	    break;
	case 'f':
	    type = T_REAL;
	    break;
	case 's':
	    type = T_STRING;
	    break;
	default:
	    error("Bad type : '%%%c' in sscanf() format string\n", fmt[1]);
	}

	fmt += 2;
	/*
	 * Parsing a number is the easy case. Just use strtol() to find the
	 * end of the number.
	 */
	if (type == T_NUMBER) {
	    char *tmp = in_string;
	    int tmp_num;

	    tmp_num = (int) strtol(in_string, &in_string, 10);
	    if (tmp == in_string)	/* No match */
		break;

	    if (!skipme) {
		SSCANF_ASSIGN_SVALUE_NUMBER(tmp_num);
	    }
	    continue;
	}
	/*
	 * float case (bobf@metronet.com 2/24/93) too bad we can't use
	 * strtod() -- not all OS's have it
	 */
	if (type == T_REAL) {
	    char *tmp = in_string;
	    float tmp_num;

	    tmp_num = _strtof(in_string, &in_string);
	    if (tmp == in_string)	/* No match */
		break;

	    if (!skipme) {
		SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
	    }
	    continue;
	}
	/*
	 * Now we have the string case.
	 */

	/*
	 * First case: There were no extra characters to match. Then this is
	 * the last match.
	 */
	if (fmt[0] == '\0') {
	    number_of_matches++;
	    if (!skipme) {
		SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
	    }
	    break;
	}
	/*
	 * If the next char in the format string is a '%' then we have to do
	 * some special checks. Only %d, %f and %% are allowed after a %s
	 */
	if (fmt[0] == '%') {
	    switch (fmt[1]) {
	    case 's':
		error("Illegal to have 2 adjacent %%s's in format string in sscanf()\n");
	    case 'd':
		for (i = 0; in_string[i]; i++) {
		    if (isdigit(in_string[i]))
			break;
		}
		break;
	    case 'f':
		for (i = 0; in_string[i]; i++) {
		    if (isdigit(in_string[i]) ||
			(in_string[i] == '.' && isdigit(in_string[i + 1])))
			break;
		}
		break;
	    case '%':
		if ((cp = strchr(in_string, '%')) == NULL)
		    i = strlen(in_string);
		else
		    i = cp - in_string;
		break;
	    default:
		error("Bad type : '%%%c' in sscanf() format string\n", fmt[1]);
	    }

	    if (!skipme) {
		char *match;

		match = DXALLOC(i + 1, TAG_STRING, "inter_scanf");
		(void) strncpy(match, in_string, i);
		match[i] = '\0';
		SSCANF_ASSIGN_SVALUE_STRING(match);
	    }
	    in_string += i;
	    continue;
	}
	if ((cp = strchr(fmt, '%')) == NULL)
	    cp = fmt + strlen(fmt);

	for (i = 0; in_string[i]; i++) {
	    if (strncmp(in_string + i, fmt, cp - fmt) == 0) {
		char *match;

		/*
		 * Found a match !
		 */
		if (!skipme) {
		    match = DXALLOC(i + 1, TAG_STRING, "inter_scanf");
		    (void) strncpy(match, in_string, i);
		    match[i] = '\0';
		    SSCANF_ASSIGN_SVALUE_STRING(match);
		}
		in_string += (i + cp - fmt);
		fmt = cp;	/* advance fmt to next % */
		break;
	    }
	}
	if (fmt == cp)		/* If match, then do continue. */
	    continue;

	/*
	 * No match was found. Then we stop here, and return the result so
	 * far !
	 */
	break;
    }
    return number_of_matches;
}

/* test stuff ... -- LA */
/* dump # of times each efun has been used */
#ifdef OPCPROF

static int cmpopc P2(opc_t *, one, opc_t *, two)
{
    return (two->count - one->count);
}

void opcdump P1(char *, tfn)
{
    int i, len, limit;
    char tbuf[SMALL_STRING_SIZE], *fn;
    FILE *fp;

    if ((len = strlen(tfn)) >= (SMALL_STRING_SIZE - 7)) {
	add_vmessage("Path '%s' too long.\n", tfn);
	return;
    }
    strcpy(tbuf, tfn);
    strcpy(tbuf + len, ".efun");
    fn = check_valid_path(tbuf, current_object, "opcprof", 1);
    if (!fn) {
	add_vmessage("Invalid path '%s' for writing.\n", tbuf);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	add_vmessage("Unable to open %s.\n", fn);
	return;
    }
    add_vmessage("Dumping to %s ... ", fn);
    limit = sizeof(opc_efun) / sizeof(opc_t);
    for (i = 0; i < limit; i++) {
	fprintf(fp, "%-30s: %10d\n", opc_efun[i].name, opc_efun[i].count);
    }
    fclose(fp);

    strcpy(tbuf, tfn);
    strcpy(tbuf + len, ".eoper");
    fn = check_valid_path(tbuf, current_object, "opcprof", 1);
    if (!fn) {
	add_vmessage("Invalid path '%s' for writing.\n", tbuf);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	add_vmessage("Unable to open %s for writing.\n", fn);
	return;
    }
    for (i = 0; i < BASE; i++) {
	fprintf(fp, "%-30s: %10d\n",
		query_instr_name(i), opc_eoper[i]);
    }
    fclose(fp);
    add_message("done.\n");
}
#endif

/*
 * Reset the virtual stack machine.
 */
void reset_machine P1(int, first)
{
    csp = control_stack - 1;
    if (first)
	sp = &start_of_stack[-1];
    else {
	pop_n_elems(sp - start_of_stack + 1);
    }
    break_sp = start_of_switch_stack + RUNTIME_SWITCH_STACK_SIZE;
}

#ifdef TRACE_CODE
static char *get_arg P2(int, a, int, b)
{
    static char buff[10];
    char *from, *to;

    from = previous_pc[a];
    to = previous_pc[b];
    if (to - from < 2)
	return "";
    if (to - from == 2) {
	sprintf(buff, "%d", from[1]);
	return buff;
    }
    if (to - from == 3) {
	short arg;

	COPY_SHORT(&arg, from + 1);
	sprintf(buff, "%d", (int)arg);
	return buff;
    }
    if (to - from == 5) {
	int arg;

	COPY_INT(&arg, from + 1);
	sprintf(buff, "%d", arg);
	return buff;
    }
    return "";
}

int last_instructions()
{
    int i;

    i = last;
    do {
	if (previous_instruction[i] != 0)
	    printf("%6x: %3d %8s %-25s (%d)\n", previous_pc[i],
		   previous_instruction[i],
		   get_arg(i, (i + 1) %
			   (sizeof previous_instruction / sizeof(int))),
		   get_f_name(previous_instruction[i]),
		   stack_size[i] + 1);
	i = (i + 1) % (sizeof previous_instruction / sizeof(int));
    } while (i != last);
    return last;
}

#endif				/* TRACE_CODE */


#ifdef TRACE
/* Generate a debug message to the user */
void do_trace P3(char *, msg, char *, fname, char *, post)
{
    char *objname;

    if (!TRACEHB)
	return;
    objname = TRACETST(TRACE_OBJNAME) ? (current_object && current_object->name ? current_object->name : "??") : "";
    add_vmessage("*** %d %*s %s %s %s%s", tracedepth, tracedepth, "", msg, objname, fname, post);
}
#endif

int assert_master_ob_loaded P2(char *, fun, char *, arg) {
    /* First we check two special conditions.  If master_ob == -1, main.c
     * hasn't tried to load the master object yet, so we shouldn't.
     * if master_ob_is_loading is set, then the master is in the process
     * of loading.
     */
    if (master_ob_is_loading || (master_ob == (struct object *)-1))
	return -1;
    if (!master_ob || (master_ob->flags & O_DESTRUCTED)) {
        struct object *ob;

        ob = load_object(master_file_name, 0);
	if (!ob) {
	    fprintf(stderr, "%s(%s) failed: Master failed to load.\n", fun, arg);
	    return 0;
	}
    }
    return 1;
}

/* If the master object can't be loaded, we return zero. (struct svalue *)-1
 * means that a secure object is loading.  Default behavior should be that
 * the check succeeds.
 */
struct svalue *apply_master_ob P2(char *, fun, int, num_arg)
{
    int err;

    if ((err = assert_master_ob_loaded("apply_master_ob", fun)) != 1) {
	pop_n_elems(num_arg);
	return (struct svalue *)err;
    }
    call_origin = ORIGIN_DRIVER;
    return sapply(fun, master_ob, num_arg);
}

struct svalue *safe_apply_master_ob P2(char *, fun, int, num_arg)
{
    int err;
    if ((err = assert_master_ob_loaded("safe_apply_master_ob", fun)) != 1) {
	pop_n_elems(num_arg);
	return (struct svalue *)err;
    }
    return safe_apply(fun, master_ob, num_arg, ORIGIN_DRIVER);
}

/*
 * When an object is destructed, all references to it must be removed
 * from the stack.
 */
void remove_object_from_stack P1(struct object *, ob)
{
    struct svalue *svp;

    for (svp = start_of_stack; svp <= sp; svp++) {
	if (svp->type != T_OBJECT)
	    continue;
	if (svp->u.ob != ob)
	    continue;
	free_object(svp->u.ob, "remove_object_from_stack");
	svp->type = T_NUMBER;
	svp->u.number = 0;
    }
}

int strpref P2(char *, p, char *, s)
{
    while (*p)
	if (*p++ != *s++)
	    return 0;
    return 1;
}

static float _strtof P2(char *, nptr, char **, endptr)
{
    register char *s = nptr;
    register float acc;
    register int neg, c, any, div;

    div = 1;
    neg = 0;
    /*
     * Skip white space and pick up leading +/- sign if any.
     */
    do {
	c = *s++;
    } while (isspace(c));
    if (c == '-') {
	neg = 1;
	c = *s++;
    } else if (c == '+')
	c = *s++;

    for (acc = 0, any = 0;; c = *s++) {
	if (isdigit(c))
	    c -= '0';
	else if ((div == 1) && (c == '.')) {
	    div = 10;
	    continue;
	} else
	    break;
	if (div == 1) {
	    acc *= (float) 10;
	    acc += (float) c;
	} else {
	    acc += (float) c / (float) div;
	    div *= 10;
	}
	any = 1;
    }

    if (neg)
	acc = -acc;

    if (endptr != 0)
	*endptr = any ? s - 1 : (char *) nptr;

    return acc;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_stack() {
    struct svalue *sv;

    for (sv = start_of_stack; sv <= sp; sv++) mark_svalue(sv);
}
#endif
