#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "file.h"
#include "patchlevel.h"
#include "backend.h"
#include "simul_efun.h"
#include "debug.h"
#include "eoperators.h"
#include "efunctions.h"
#include "lex.h"
#include "functab_tree.h"
#include "sprintf.h"
#include "swap.h"
#include "comm.h"
#include "port.h"
#include "qsort.h"
#include "compiler.h"
#include "regexp.h"

#ifdef OPCPROF
#include "opc.h"

static int opc_eoper[BASE];
#endif

#ifdef OPCPROF_2D
/* warning, this is typically 4 * 100 * 100 = 40k */
static int opc_eoper_2d[BASE+1][BASE+1];
static int last_eop = 0;
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
    "class"
};
#define TYPE_CODES_END 0x400
#define TYPE_CODES_START 0x2

int master_ob_is_loading;
#ifdef PACKAGE_UIDS
extern userid_t *backbone_uid;
#endif
extern int max_cost;
extern int call_origin;

INLINE void push_indexed_lvalue PROT((int));
static svalue_t *find_value PROT((int));
#ifdef TRACE
static void do_trace_call PROT((function_t *));
#endif
void break_point PROT((void));
static INLINE void do_loop_cond_number PROT((void));
static INLINE void do_loop_cond_local PROT((void));
static void do_catch PROT((char *, unsigned short));
#ifdef OPCPROF
static int cmpopc PROT((opc_t *, opc_t *));
#endif
#ifdef DEBUG
int last_instructions PROT((void));
#endif
static float _strtof PROT((char *, char **));
#ifdef TRACE_CODE
static char *get_arg PROT((int, int));
#endif

#ifdef DEBUG
int foreach_in_progress = 0;
#endif

int inter_sscanf PROT((svalue_t *, svalue_t *, svalue_t *, int));
program_t *current_prog;
short int caller_type;
static int tracedepth;
int num_varargs;

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
svalue_t *fp;		/* Pointer to first argument. */

svalue_t *sp;

int function_index_offset;	/* Needed for inheritance */
int variable_index_offset;	/* Needed for inheritance */
int st_num_arg;

static svalue_t start_of_stack[EVALUATOR_STACK_SIZE];
svalue_t *end_of_stack = start_of_stack + EVALUATOR_STACK_SIZE - 5;

/* Used to throw an error to a catch */
svalue_t catch_value = {T_NUMBER};

/* used by routines that want to return a pointer to an svalue */
svalue_t apply_ret_value = {T_NUMBER};

control_stack_t control_stack[MAX_TRACE];
control_stack_t *csp;	/* Points to last element pushed */

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
 * and array elements.
 *
 * The local variables are allocated on the stack together with the arguments.
 * the register 'frame_pointer' points to the first argument.
 *
 * The global variables must keep their values between executions, and
 * have space allocated at the creation of the object.
 *
 * Elements in arrays are similar to global variables. There is a reference
 * count to the whole array, that states when to deallocate the array.
 * The elements consists of 'svalue_t's, and will thus have to be freed
 * immediately when over written.
 */

/*
 * Push an object pointer on the stack. Note that the reference count is
 * incremented.
 * A destructed object must never be pushed onto the stack.
 */
INLINE
void push_object P1(object_t *, ob)
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
    switch (c) {
    case T_INVALID: return "*invalid*";
    case T_LVALUE: return "*lvalue*";
    case T_LVALUE_BYTE: return "*lvalue_byte*";
    case T_LVALUE_RANGE: return "*lvalue_range*";
    case T_ERROR_HANDLER: return "*error_handler*";
    IF_DEBUG(case T_FREED: return "*freed*");
    }
    return "*unknown*";
}

/*
 * May current_object shadow object 'ob' ? We rely heavily on the fact that
 * function names are pointers to shared strings, which means that equality
 * can be tested simply through pointer comparison.
 */
#ifndef NO_SHADOWS
int validate_shadowing P1(object_t *, ob)
{
    int i, j;
    program_t *shadow = current_object->prog, *victim = ob->prog;
    svalue_t *ret;

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
    if ((int) shadow->num_functions < (int) victim->num_functions) {
	for (i = 0; i < (int) shadow->num_functions; i++) {
	    j = lookup_function(victim->functions, victim->tree_r,
				shadow->functions[i].name);
	    if (j != -1 && victim->functions[j].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->functions[j].name);
	}
    } else {
	for (i = 0; i < (int) victim->num_functions; i++) {
	    j = lookup_function(shadow->functions, shadow->tree_r,
				victim->functions[i].name);
	    if (j != -1 && victim->functions[i].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->functions[i].name);
	}
    }
#else
    for (i = 0; i < (int) shadow->num_functions; i++) {
	for (j = 0; j < (int) victim->num_functions; j++) {
	    if (shadow->functions[i].name != victim->functions[j].name)
		continue;
	    if (victim->functions[j].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->functions[j].name);
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

INLINE void push_nulls P1(int, num)
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
static INLINE svalue_t *find_value P1(int, num)
{
    DEBUG_CHECK2(num >= (int) current_object->prog->num_variables,
		 "Illegal variable access %d(%d).\n",
		 num, current_object->prog->num_variables);
    return &current_object->variables[num];
}
#else
#define find_value(num) (&current_object->variables[num])
#endif

INLINE void
free_string_svalue P1(svalue_t *, v)
{
    char *str = v->u.string;

    if (v->subtype & STRING_COUNTED) {
#ifdef STRING_STATS
	int size = COUNTED_STRLEN(str);
#endif
	SUB_STRING(size);
	if (!(--(COUNTED_REF(str)))) {
	    NDBG(BLOCK(str));
	    if (v->subtype & STRING_HASHED) {
		SUB_NEW_STRING(size, sizeof(block_t));
		deallocate_string(str);
	    } else {
		SUB_NEW_STRING(size, sizeof(malloc_block_t));
		FREE(MSTR_BLOCK(str));
	    }
	} else {
	    NDBG(BLOCK(str));
	}
    }
}

void unlink_string_svalue P1(svalue_t *, s) {
    char *str;

    switch (s->subtype) {
    case STRING_MALLOC:
	if (MSTR_REF(s->u.string) > 1)
	    s->u.string = string_unlink(s->u.string, "unlink_string_svalue");
	break;
    case STRING_SHARED:
	{
	    int l = SHARED_STRLEN(s->u.string);

	    str = new_string(l, "unlink_string_svalue");
	    strncpy(str, s->u.string, l + 1);
	    free_string(s->u.string);
	    s->subtype = STRING_MALLOC;
	    s->u.string = str;
	    break;
	}
    case STRING_CONSTANT:
	s->u.string = string_copy(sp->u.string, "unlink_string_svalue");
	s->subtype = STRING_MALLOC;
	break;
    }
}

/*
 * Free the data that an svalue is pointing to. Not the svalue
 * itself.
 * Use the free_svalue() define to call this
 */
#ifdef DEBUG
INLINE void int_free_svalue P2(svalue_t *, v, char *, tag)
#else
INLINE void int_free_svalue P1(svalue_t *, v)
#endif
{

    if (v->type == T_STRING) {
	char *str = v->u.string;
	
	if (v->subtype & STRING_COUNTED) {
#ifdef STRING_STATS
	    int size = COUNTED_STRLEN(str);
#endif
	    SUB_STRING(size);
	    if (!(--(COUNTED_REF(str)))) {
		NDBG(BLOCK(str));
		if (v->subtype & STRING_HASHED) {
		    SUB_NEW_STRING(size, sizeof(block_t));
		    deallocate_string(str);
		} else {
		    SUB_NEW_STRING(size, sizeof(malloc_block_t));
		    FREE(MSTR_BLOCK(str));
		}
	    } else {
		NDBG(BLOCK(str));
	    }
	}
    } else if (v->type & T_REFED) {
	if (!(--v->u.refed->ref)) {
	    switch (v->type) {
	    case T_OBJECT:
		dealloc_object(v->u.ob, "free_svalue");
		break;
	    case T_CLASS:
		dealloc_class(v->u.arr);
		break;
	    case T_ARRAY:
		dealloc_array(v->u.arr);
		break;
	    case T_BUFFER:
		if (v->u.buf != &null_buf)
		    FREE((char *)v->u.buf);
		break;
	    case T_MAPPING:
		dealloc_mapping(v->u.map);
		break;
	    case T_FUNCTION:
		dealloc_funp(v->u.fp);
		break;
	    }
	}
    } else if (v->type == T_ERROR_HANDLER) {
	(*v->u.error_handler)();
    }
#ifdef DEBUG
    else if (v->type == T_FREED) {
	fatal("T_FREED svalue freed.  Previously freed by %s.\n", v->u.string);
    }
    v->type = T_FREED;
    v->u.string = tag;
#endif
}

/*
 * Free several svalues, and free up the space used by the svalues.
 * The svalues must be sequentially located.
 */
INLINE void free_some_svalues P2(svalue_t *, v, int, num)
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

    tmp = new_string(strlen(str) + 1, "add_slash");
    *tmp = '/';
    strcpy(tmp + 1, str);
    return tmp;
}

/*
 * Assign to a svalue.
 * This is done either when element in array, or when to an identifier
 * (as all identifiers are kept in a array pointed to by the object).
 */

INLINE void assign_svalue_no_free P2(svalue_t *, to, svalue_t *, from)
{
    DEBUG_CHECK(from == 0, "Attempt to assign_svalue() from a null ptr.\n");
    DEBUG_CHECK(to == 0, "Attempt to assign_svalue() to a null ptr.\n");
    *to = *from;

    if (from->type == T_STRING) {
	if (from->subtype & STRING_COUNTED) {
	    ADD_STRING(COUNTED_STRLEN(to->u.string));
	    COUNTED_REF(to->u.string)++;
	    NDBG(BLOCK(to->u.string));
	}
    } else if (from->type & T_REFED) {
	from->u.refed->ref++;
    }
}

INLINE void assign_svalue P2(svalue_t *, dest, svalue_t *, v)
{
    /* First deallocate the previous value. */
    free_svalue(dest, "assign_svalue");
    assign_svalue_no_free(dest, v);
}

INLINE void push_some_svalues P2(svalue_t *, v, int, num)
{
    if (sp + num >= end_of_stack) { too_deep_error = 1; error("stack overflow\n"); }
    while (num--) push_svalue(v++);
}

/*
 * Copies an array of svalues to another location, which should be
 * free space.
 */
INLINE void copy_some_svalues P3(svalue_t *, dest, svalue_t *, v, int, num)
{
    while (num--)
	assign_svalue_no_free(dest+num, v+num);
}

INLINE void transfer_push_some_svalues P2(svalue_t *, v, int, num)
{
    if (sp + num >= end_of_stack) { too_deep_error = 1; error("stack overflow\n"); }
    memcpy(sp + 1, v, num * sizeof(svalue_t));
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

svalue_t global_lvalue_byte = { T_LVALUE_BYTE };

/*
 * Compute the address of an array element.
 */
INLINE void push_indexed_lvalue P1(int, code)
{
    int ind;
    svalue_t *lv;
    
    if (sp->type == T_LVALUE) {
	lv = sp->u.lvalue;
	if (!code && lv->type == T_MAPPING) {
	    sp--;
	    if (!(lv = find_for_insert(lv->u.map, sp, 0)))
		mapping_too_large();
	    free_svalue(sp, "push_indexed_lvalue: 1");
	    sp->type = T_LVALUE;
	    sp->u.lvalue = lv;
	    return;
	}
	
	if (!((--sp)->type == T_NUMBER))
	    error("Illegal type of index\n");

	ind = sp->u.number;

	switch(lv->type){
	case T_STRING:
	     {
		 int len = SVALUE_STRLEN(lv);

		 if (code) ind = len - ind;
		 if (ind >= len || ind < 0)
		     error("Index out of bounds in string index lvalue.\n");
		 unlink_string_svalue(lv);
		 sp->type = T_LVALUE;
		 sp->u.lvalue = &global_lvalue_byte;
		 global_lvalue_byte.u.lvalue_byte = (unsigned char *)&lv->u.string[ind];
		 break;
	     }
	     
	 case T_BUFFER:
	     {
		 if (code) ind = lv->u.buf->size - ind;
		 if (ind >= lv->u.buf->size || ind < 0)
		     error("Buffer index out of bounds.\n");
		 sp->type = T_LVALUE;
		 sp->u.lvalue = &global_lvalue_byte;
		 global_lvalue_byte.u.lvalue_byte = &lv->u.buf->item[ind];
		 break;
	     }
	     
	 case T_ARRAY:
	     {
		 if (code) ind = lv->u.arr->size - ind;
		 if (ind >= lv->u.arr->size || ind < 0)
		     error("Array index out of bounds\n");
		 sp->type = T_LVALUE;
		 sp->u.lvalue = lv->u.arr->item + ind;
		 break;
	     }
	     
	 default:
	     error("Indexing on illegal type.\n");
	 }
    } else {
	/* It is now coming from (x <assign_type> y)[index]... = rhs */
	/* Where x is a _valid_ lvalue */
	/* Hence the reference to sp is at least 2 :) */
	
	if (!code && (sp->type == T_MAPPING)){
	    if (!(lv = find_for_insert(sp->u.map, sp-1, 0)))
		mapping_too_large();
	    sp->u.map->ref--;
	    free_svalue(--sp, "push_indexed_lvalue: 2");
	    sp->type = T_LVALUE;
	    sp->u.lvalue = lv;
	    return;
	}
	
	if (!((sp-1)->type == T_NUMBER))
	    error("Illegal type of index\n");
	
	ind = (sp-1)->u.number;
	
	switch (sp->type){
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
		sp->u.lvalue = &global_lvalue_byte;
		global_lvalue_byte.u.lvalue_byte = (sp+1)->u.buf->item + ind;
		break;
	    }
	    
	case T_ARRAY:
	    {
		if (code) ind = sp->u.arr->size - ind;
		if (ind >= sp->u.arr->size || ind < 0)
		    error("Array index out of bounds.\n");
		sp->u.arr->ref--;
		(--sp)->type = T_LVALUE;
		sp->u.lvalue = (sp+1)->u.arr->item + ind;
		break;
	    }
	    
	default:
	    error("Indexing on illegal type.\n");
	}
    }
}

static struct lvalue_range {
    int ind1, ind2, size;
    svalue_t *owner;
} global_lvalue_range;

static svalue_t global_lvalue_range_sv = { T_LVALUE_RANGE };

INLINE void push_lvalue_range P1(int, code)
{
    int ind1, ind2, size;
    svalue_t *lv;
    
    if (sp->type == T_LVALUE){
	switch((lv = global_lvalue_range.owner = sp->u.lvalue)->type){
	case T_ARRAY:
	    size = lv->u.arr->size;
	    break;
	case T_STRING: {
	    size = SVALUE_STRLEN(lv);
	    unlink_string_svalue(lv);
	    break;
	}
	case T_BUFFER:
	    size = lv->u.buf->size;
	    break;
	default:
	    error("Range lvalue on illegal type\n");
	    IF_DEBUG(size = 0);
	}
    } else
	error("Range lvalue on illegal type\n");
    
    if (!((--sp)->type == T_NUMBER)) error("Illegal 2nd index type to range lvalue\n");
    
    ind2 = (code & 0x01) ? (size - sp->u.number) : sp->u.number;
    if (++ind2 < 0 || (ind2 > size))
	error("The 2nd index to range lvalue must be >= -1 and < sizeof(indexed value)\n");
    
    if (!((--sp)->type == T_NUMBER)) error("Illegal 1st index type to range lvalue\n");
    ind1 = (code & 0x10) ? (size - sp->u.number) : sp->u.number;
    
    if (ind1 < 0 || ind1 > size)
	error("The 1st index to range lvalue must be >= 0 and <= sizeof(indexed value)\n");
    
    global_lvalue_range.ind1 = ind1;
    global_lvalue_range.ind2 = ind2;
    global_lvalue_range.size = size;
    sp->type = T_LVALUE;
    sp->u.lvalue = &global_lvalue_range_sv;
}

INLINE void copy_lvalue_range P1(svalue_t *, from)
{
    int ind1, ind2, size, fsize;
    svalue_t *owner;
    
    ind1 = global_lvalue_range.ind1;
    ind2 = global_lvalue_range.ind2;
    size = global_lvalue_range.size;
    owner = global_lvalue_range.owner;
    
    switch(owner->type){
    case T_ARRAY:
	{
	    array_t *fv, *dv;
	    svalue_t *fptr, *dptr;
	    if (from->type != T_ARRAY) error("Illegal rhs to array range lvalue\n");
	    
	    fv = from->u.arr;
	    fptr = fv->item;
	    
	    if ((fsize = fv->size) == ind2 - ind1){
		dptr = (owner->u.arr)->item + ind1;
		
		if (fv->ref == 1){
		    /* Transfer the svalues */
		    while (fsize--){
			free_svalue(dptr, "copy_lvalue_range : 1");
			*dptr++ = *fptr++;
		    }
		    free_empty_array(fv);
		} else {
		    while (fsize--) assign_svalue(dptr++, fptr++);
		    fv->ref--;
		}
	    } else {
		array_t *old_dv = owner->u.arr;
		svalue_t *old_dptr = old_dv->item;
		
		/* Need to reallocate the array */
		dv = allocate_empty_array(size - ind2 + ind1 + fsize);
		dptr = dv->item;
		
		/* ind1 can range from 0 to sizeof(old_dv) */
		while (ind1--) assign_svalue_no_free(dptr++, old_dptr++);
		
		if (fv->ref == 1){
		    while (fsize--) *dptr++ = *fptr++;
		    free_empty_array(fv);
		} else {
		    while (fsize--) assign_svalue_no_free(dptr++, fptr++);
		    fv->ref--;
		}
		
		/* ind2 can range from 0 to sizeof(old_dv) */
		old_dptr = old_dv->item + ind2;
		size -= ind2;
		
		while (size--) assign_svalue_no_free(dptr++, old_dptr++);
		free_array(old_dv);
		
		owner->u.arr = dv;
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
		
		owner->u.string = tmp = new_string(size - ind2 + ind1 + fsize + 1, "copy_lvalue_range");
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
		FREE_MSTR(dstr);
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
		buffer_t *b;
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

INLINE void assign_lvalue_range P1(svalue_t *, from)
{
    int ind1, ind2, size, fsize;
    svalue_t *owner;
    
    ind1 = global_lvalue_range.ind1;
    ind2 = global_lvalue_range.ind2;
    size = global_lvalue_range.size;
    owner = global_lvalue_range.owner;
    
    switch(owner->type){
    case T_ARRAY:
	{
	    array_t *fv, *dv;
	    svalue_t *fptr, *dptr;
	    if (from->type != T_ARRAY) error("Illegal rhs to array range lvalue\n");
	    
	    fv = from->u.arr;
	    fptr = fv->item;
	    
	    if ((fsize = fv->size) == ind2 - ind1){
		dptr = (owner->u.arr)->item + ind1;
		while (fsize--) assign_svalue(dptr++, fptr++);
	    } else {
		array_t *old_dv = owner->u.arr;
		svalue_t *old_dptr = old_dv->item;
		
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
		free_array(old_dv);
		
		owner->u.arr = dv;
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
		
		owner->u.string = tmp = new_string(size - ind2 + ind1 + fsize + 1, "assign_lvalue_range");
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
		FREE_MSTR(dstr);
	    }
	    break;
	}
	
    case T_BUFFER:
	{
	    if (from->type != T_BUFFER) error("Illegal rhs to buffer range lvalue.\n");
	    
	    if ((fsize = from->u.buf->size) == ind2 - ind1){
		memcpy((owner->u.buf)->item + ind1, from->u.buf->item, fsize);
	    } else {
		buffer_t *b;
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

void bad_argument P4(svalue_t *, val, int, type, int, arg, int, instr)
{
    outbuffer_t outbuf;
    int flag = 0;
    int j = TYPE_CODES_START;
    int k = 0;

    outbuf_zero(&outbuf);
    outbuf_addv(&outbuf, "Bad argument %d to %s%s\nExpected: ", arg, 
		get_f_name(instr), (instr < BASE ? "" : "()"));

    do {
	if (type & j) {
	    if (flag) outbuf_add(&outbuf, " or ");
	    else flag = 1;
	    outbuf_add(&outbuf, type_names[k]);
	}
	k++;
    } while (!((j <<= 1) & TYPE_CODES_END));

    outbuf_add(&outbuf, " Got: ");
    svalue_to_string(val, &outbuf, 0, 0, 0);
    outbuf_add(&outbuf, ".\n");
    outbuf_fix(&outbuf);
    error_needs_free(outbuf.buffer);
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
    csp->framekind = frkind;
    csp->fr.func = (function_t *)funp;
    csp->prev_ob = previous_ob;
    csp->fp = fp;
    csp->prog = current_prog;
    csp->pc = pc;
    csp->function_index_offset = function_index_offset;
    csp->variable_index_offset = variable_index_offset;
#ifdef PROFILE_FUNCTIONS
    if ((frkind & FRAME_MASK) == FRAME_FUNCTION) {
	get_cpu_times(&(csp->entry_secs), &(csp->entry_usecs));
	((function_t *)funp)->calls++;
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
    if ((csp->framekind & FRAME_MASK) == FRAME_FUNCTION) {
	long secs, usecs, dsecs;

	get_cpu_times((unsigned long *) &secs, (unsigned long *) &usecs);
	dsecs = (((secs - csp->entry_secs) * 1000000)
		 + (usecs - csp->entry_usecs));
	csp->fr.func->self += dsecs;
	if (csp != control_stack) {
	    function_t *f;

	    if (((csp - 1)->framekind & FRAME_MASK) == FRAME_FUNCTION) {
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
    csp--;
}

/*
 * Push a pointer to a array on the stack. Note that the reference count
 * is incremented. Newly created arrays normally have a reference count
 * initialized to 1.
 */
INLINE void push_array P1(array_t *, v)
{
    v->ref++;
    sp++;
    sp->type = T_ARRAY;
    sp->u.arr = v;
}

INLINE void push_refed_array P1(array_t *, v)
{
    sp++;
    sp->type = T_ARRAY;
    sp->u.arr = v;
}

INLINE void
push_buffer P1(buffer_t *, b)
{
    b->ref++;
    sp++;
    sp->type = T_BUFFER;
    sp->u.buf = b;
}

INLINE void
push_refed_buffer P1(buffer_t *, b)
{
    sp++;
    sp->type = T_BUFFER;
    sp->u.buf = b;
}

/*
 * Push a mapping on the stack.  See push_array(), above.
 */
INLINE void
push_mapping P1(mapping_t *, m)
{
    m->ref++;
    sp++;
    sp->type = T_MAPPING;
    sp->u.map = m;
}

INLINE void
push_refed_mapping P1(mapping_t *, m)
{
    sp++;
    sp->type = T_MAPPING;
    sp->u.map = m;
}

/*
 * Push a class on the stack.  See push_array(), above.
 */
INLINE void
push_class P1(array_t *, v)
{
    v->ref++;
    sp++;
    sp->type = T_CLASS;
    sp->u.arr = v;
}

INLINE void
push_refed_class P1(array_t *, v)
{
    sp++;
    sp->type = T_CLASS;
    sp->u.arr = v;
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
 * um.  yeah.  Name for historical reasons.  
 * See push_string(..., STRING_CONSTANT) for stuff that is really constant.
 */
INLINE
void push_constant_string P1(char *, p)
{
    sp++;
    sp->type = T_STRING;
    sp->u.string = make_shared_string(p);
    sp->subtype = STRING_SHARED;
}

#ifdef TRACE
static void do_trace_call P1(function_t *, funp)
{
    do_trace("Call direct ", funp->name, " ");
    if (TRACEHB) {
	if (TRACETST(TRACE_ARGS)) {
	    int i;

	    add_vmessage(command_giver, " with %d arguments: ", funp->num_arg);
	    for (i = funp->num_arg - 1; i >= 0; i--) {
		print_svalue(&sp[-i]);
		add_message(command_giver, " ");
	    }
	}
	add_message(command_giver, "\n");
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
    } else {
	/* Correct number of arguments and local variables */
	push_nulls(local + num_arg - actual);
    }
    fp = sp - (csp->num_local_variables = local + num_arg) + 1;
}

INLINE void setup_varargs_variables P3(int, actual, int, local, int, num_arg) {
    array_t *arr;
    if (actual >= num_arg) {
	int n = actual - num_arg + 1;
	/* Aggregate excessive arguments */
	arr = allocate_empty_array(n);
	while (n--)
	    arr->item[n] = *sp--;
    } else {
	/* Correct number of arguments and local variables */
	push_nulls(num_arg - 1 - actual);
	arr = null_array();
    }
    push_refed_array(arr);
    push_nulls(local);
    fp = sp - (csp->num_local_variables = local + num_arg) + 1;
}

INLINE function_t *
setup_new_frame P1(function_t *, funp)
{
    function_index_offset = variable_index_offset = 0;
    while (funp->flags & NAME_INHERITED) {
	function_index_offset +=
	    current_prog->inherit[funp->offset].function_index_offset;
	variable_index_offset +=
	    current_prog->inherit[funp->offset].variable_index_offset;
	current_prog = current_prog->inherit[funp->offset].prog;
	funp = &current_prog->functions[funp->function_index_offset];
    }
    /* Remove excessive arguments */
    if (funp->flags & NAME_TRUE_VARARGS)
	setup_varargs_variables(csp->num_local_variables, funp->num_local, 
				funp->num_arg);
    else
	setup_variables(csp->num_local_variables, funp->num_local, 
			funp->num_arg);
#ifdef TRACE
    tracedepth++;
    if (TRACEP(TRACE_CALL)) {
	do_trace_call(funp);
    }
#endif
    return funp;
}

INLINE function_t *setup_inherited_frame P1(function_t *, funp)
{
    while (funp->flags & NAME_INHERITED) {
	function_index_offset +=
	    current_prog->inherit[funp->offset].function_index_offset;
	variable_index_offset +=
	    current_prog->inherit[funp->offset].variable_index_offset;
	current_prog = current_prog->inherit[funp->offset].prog;
	funp = &current_prog->functions[funp->function_index_offset];
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

#ifdef DEBUG
void break_point()
{
    /* The current implementation of foreach leaves some stuff lying on the
       stack */
    if (!foreach_in_progress && sp - fp - csp->num_local_variables + 1 != 0)
	fatal("Bad stack pointer.\n");
}
#endif

program_t fake_prog = { "<function>" };
unsigned char fake_program = F_RETURN;

void setup_fake_frame P1(funptr_t *, fun) {
    if (csp == &control_stack[MAX_TRACE-1]) {
	too_deep_error = 1;
	error("Too deep recursion.\n");
    }
    csp++;
    csp->caller_type = caller_type;
    csp->framekind = FRAME_FAKE | FRAME_OB_CHANGE;
    csp->fr.funp = fun;
    csp->ob = current_object;
    csp->prev_ob = previous_ob;
    csp->fp = fp;
    csp->prog = current_prog;
    csp->pc = pc;
    pc = (char *)&fake_program;
    csp->function_index_offset = function_index_offset;
    csp->variable_index_offset = variable_index_offset;
    caller_type = ORIGIN_FUNCTION_POINTER;
    csp->num_local_variables = 0;
    current_prog = &fake_prog;
    previous_ob = current_object;
    current_object = fun->hdr.owner;
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
    csp--;
}

/* num_arg args are on the stack, and the args from the array vec should be
 * put in front of them.  This is so that the order of arguments is logical.
 * 
 * evaluate( (: f, a :), b) -> f(a,b) and not f(b, a) which would happen
 * if we simply pushed the args from vec at this point.  (Note that the
 * old function pointers are broken in this regard)
 */
int merge_arg_lists P3(int, num_arg, array_t *, arr, int, start) {
    int num_arr_arg = arr->size - start;
    svalue_t *sptr;

    if (num_arr_arg) {
	sptr = (sp += num_arr_arg);
	if (num_arg) {
	    /* We need to do some stack movement so that the order
	       of arguments is logical */
	    while (num_arg--) {
		*sptr = *(sptr - num_arr_arg);
		sptr--;
	    }
	}
	num_arg = arr->size;
	while (--num_arg >= start)
	    assign_svalue_no_free(sptr--, &arr->item[num_arg]);
	return (sp - sptr); /* could just return num_arr_arg if num_arg is 0 but .... -Sym */
    }	    
    return num_arg;
}

void cfp_error P1(char *, s) {
    remove_fake_frame();
    error(s);
}

svalue_t *
call_function_pointer P2(funptr_t *, funp, int, num_arg)
{
    static func_t *oefun_table = efun_table - BASE;
    function_t *func;

    if (funp->hdr.owner->flags & O_DESTRUCTED)
	error("Owner of function pointer is destructed.\n");

    setup_fake_frame(funp);
    if (current_object->flags & O_SWAPPED)
	load_ob_from_swap(current_object);

    switch (funp->hdr.type) {
    case FP_SIMUL:
	if (funp->hdr.args) {
	    check_for_destr(funp->hdr.args);
	    num_arg = merge_arg_lists(num_arg, funp->hdr.args, 0);
	}
	call_simul_efun(funp->f.simul.index, num_arg);
	break;
    case FP_EFUN:
	{
	    int i, def;
	    
	    fp = sp - num_arg + 1;
	    if (funp->hdr.args) {
		check_for_destr(funp->hdr.args);
		num_arg = merge_arg_lists(num_arg, funp->hdr.args, 0);
	    }
	    i = funp->f.efun.index;
	    if (num_arg == instrs[i].min_arg - 1 && 
		((def = instrs[i].Default) != DEFAULT_NONE)) {
		if (def == DEFAULT_THIS_OBJECT) {
		    if (current_object && !(current_object->flags & O_DESTRUCTED))
			push_object(current_object);
		    else
			*(++sp)=const0;
		} else {
		    (++sp)->type = T_NUMBER;
		    sp->u.number = def;
		}
		num_arg++;
	    } else
		if (num_arg < instrs[i].min_arg) {
		    error("Too few arguments to efun %s in efun pointer.\n", instrs[i].name);
		} else if (num_arg > instrs[i].max_arg && instrs[i].max_arg != -1) {
		    error("Too many arguments to efun %s in efun pointer.\n", instrs[i].name);
		}
	    /* possibly we should add TRACE, OPC, etc here;
	       also on eval_cost here, which is ok for just 1 efun */
	    {
		int j, n = instrs[i].min_arg;
		st_num_arg = num_arg;
		
		for (j = 0; j < n; j++) {
		    CHECK_TYPES(sp - num_arg + j + 1, instrs[i].type[j], j + 1, i);
		}
		(*oefun_table[i])();

		free_svalue(&apply_ret_value, "call_function_pointer");
		if (instrs[i].ret_type == TYPE_NOVALUE)
		    apply_ret_value = const0;
		else
		    apply_ret_value = *sp--;
		remove_fake_frame();
		return &apply_ret_value;
	    }
	}
    case FP_LOCAL | FP_NOT_BINDABLE: {
	fp = sp - num_arg + 1;

	/* After the fake frame, current_object is funp->hdr.owner - Sym */
	func = &current_object->prog->functions[funp->f.local.index];

	if (func->flags & NAME_UNDEFINED)
	    error("Undefined function: %s\n", func->name);

	push_control_stack(FRAME_FUNCTION, func);

	caller_type = ORIGIN_LOCAL;
	current_prog = funp->hdr.owner->prog;

	if (funp->hdr.args) {
	    array_t *v = funp->hdr.args;

	    check_for_destr(v);
	    num_arg = merge_arg_lists(num_arg, v, 0);
	}

	csp->num_local_variables = num_arg;
	func = setup_new_frame(func);

	call_program(current_prog, func->offset);
	break;
    }
    case FP_FUNCTIONAL: 
    case FP_FUNCTIONAL | FP_NOT_BINDABLE: {
	fp = sp - num_arg + 1;

	push_control_stack(FRAME_FUNP, funp);
	caller_type = ORIGIN_FUNCTIONAL;
	current_prog = funp->f.functional.prog;

	if (funp->hdr.args) {
	    array_t *v = funp->hdr.args;

	    check_for_destr(v);
	    num_arg = merge_arg_lists(num_arg, v, 0);
	}

	setup_variables(num_arg, funp->f.functional.num_local,
			funp->f.functional.num_arg);

	function_index_offset = funp->f.functional.fio;
	variable_index_offset = funp->f.functional.vio;
	call_program(funp->f.functional.prog, funp->f.functional.offset);
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

svalue_t *
safe_call_function_pointer P2(funptr_t *, funp, int, num_arg)
{
    error_context_t econ;
    svalue_t *ret;

    save_context(&econ);
    if (!SETJMP(econ.context)) {
	ret = call_function_pointer(funp, num_arg);
    } else {
	restore_context(&econ);
	/* condition was restored to where it was when we came in */
	pop_n_elems(num_arg);
	ret = NULL;
    }
    pop_context(&econ);
    return ret;
}

/*
 * When a array is given as argument to an efun, all items have to be
 * checked if there would be a destructed object.
 * A bad problem currently is that a array can contain another array, so this
 * should be tested too. But, there is currently no prevention against
 * recursive arrays, which means that this can not be tested. Thus, MudOS
 * may crash if a array contains a array that contains a destructed object
 * and this top-most array is used as an argument to an efun.
 */
/* MudOS won't crash when doing simple operations like assign_svalue
 * on a destructed object. You have to watch out, of course, that you don't
 * apply a function to it.
 * to save space it is preferable that destructed objects are freed soon.
 *   amylaar
 */
void check_for_destr P1(array_t *, v)
{
    int i = v->size;
    
    while (i--) {
	if ((v->item[i].type == T_OBJECT) && (v->item[i].u.ob->flags & O_DESTRUCTED)) {
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

static INLINE void do_loop_cond_local()
{
    svalue_t *s1, *s2;
    int i;
    
    s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
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
	if (s1->type == T_OBJECT && (s1->u.ob->flags & O_DESTRUCTED)) {
	    free_object(s1->u.ob, "do_loop_cond:1");
	    *s1 = const0;
	}
	if (s2->type == T_OBJECT && (s2->u.ob->flags & O_DESTRUCTED)) {
	    free_object(s2->u.ob, "do_loop_cond:2");
	    *s2 = const0;
	}
	if (s1->type == T_NUMBER && s2->type == T_NUMBER) {
	    i = s1->u.number < s2->u.number;
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
	i = 0;
    }
    if (i) {
	unsigned short offset;
	
	COPY_SHORT(&offset, pc);
	pc -= offset;
    } else pc += 2;
}

static INLINE void do_loop_cond_number()
{
    svalue_t *s1;
    int i;
    
    s1 = fp + EXTRACT_UCHAR(pc++); /* a from (a < b) */
    LOAD_INT(i, pc);
    if (s1->type == T_NUMBER) {
	if (s1->u.number < i){
	    unsigned short offset;
	    
	    COPY_SHORT(&offset, pc);
	    pc -= offset;
	} else pc += 2;
    } else if (s1->type == T_REAL) {
	if (s1->u.real < i){
	    unsigned short offset;
	    
	    COPY_SHORT(&offset, pc);
	    pc -= offset;
	} else pc += 2;
    } else error("Right side of < is a number, left side is not.\n");
}

#ifdef LPC_TO_C
void
call_program P2(program_t *, prog, POINTER_INT, offset) {
    if (prog->program_size)
	eval_instruction(prog->program + offset);
    else {
	DEBUG_CHECK(!offset, "Null function pointer in jump_table.\n");
	(*
	 ( void (*) PROT((void)) ) offset	/* cast to a function pointer */
	 )();
    }
}
#endif

/*
 * Evaluate instructions at address 'p'. All program offsets are
 * to current_prog->program. 'current_prog' must be setup before
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
    int i, n;
    float real;
    svalue_t *lval;
    int instruction;
#if defined(TRACE_CODE) || defined(TRACE) || defined(OPCPROF) || defined(OPCPROF_2D)
    int real_instruction;
#endif
    unsigned short offset;
    static func_t *oefun_table = efun_table - BASE + ONEARG_MAX;
#ifndef DEBUG
    static func_t *ooefun_table = efun_table - BASE;
#endif
    static instr_t *instrs2 = instrs + ONEARG_MAX;
    
    IF_DEBUG(svalue_t *expected_stack);

    /* Next F_RETURN at this level will return out of eval_instruction() */
    csp->framekind |= FRAME_EXTERNAL;
    pc = p;
    while (1) {
	instruction = EXTRACT_UCHAR(pc++);
#if defined(TRACE_CODE) || defined(TRACE) || defined(OPCPROF) || defined(OPCPROF_2D)
	if (instruction >= F_EFUN0 && instruction <= F_EFUNV)
	    real_instruction = EXTRACT_UCHAR(pc) + ONEARG_MAX;
	else
	    real_instruction = instruction;
#  ifdef TRACE_CODE
	previous_instruction[last] = real_instruction;
	previous_pc[last] = pc - 1;
	stack_size[last] = sp - fp - csp->num_local_variables;
	last = (last + 1) % (sizeof previous_instruction / sizeof(int));
#  endif
#  ifdef TRACE
	if (TRACEP(TRACE_EXEC)) {
	    do_trace("Exec ", get_f_name(real_instruction), "\n");
	}
#  endif
#  ifdef OPCPROF
	if (real_instruction < BASE)
	    opc_eoper[real_instruction]++;
	else
	    opc_efun[real_instruction-BASE].count++;
#  endif
#  ifdef OPCPROF_2D
	if (real_instruction < BASE) {
	    if (last_eop) opc_eoper_2d[last_eop][real_instruction]++;
	    last_eop = real_instruction;
	} else {
	    if (last_eop) opc_eoper_2d[last_eop][BASE]++;
	    last_eop = BASE;
	}
#  endif
#endif
	if (!--eval_cost) {
	    debug_message("object /%s: eval_cost too big %d\n", 
			  current_object->name, max_cost);
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
	case F_PUSH:		/* Push a number of things onto the stack */
	    n = EXTRACT_UCHAR(pc++);
	    while (n--) {
		i = EXTRACT_UCHAR(pc++);
		switch (i & PUSH_WHAT) {
		case PUSH_STRING:
		    DEBUG_CHECK1((i & PUSH_MASK) >= current_prog->num_strings,
				 "string %d out of range in F_STRING!\n",
				 i & PUSH_MASK);
		    (++sp)->type = T_STRING;
		    sp->subtype = STRING_SHARED;
		    sp->u.string = ref_string(current_prog->strings[i & PUSH_MASK]);
		    break;
		case PUSH_LOCAL:
		    lval = fp + (i & PUSH_MASK);
		    DEBUG_CHECK((fp - lval) >= csp->num_local_variables,
				"Tried to push non-existent local\n");
		    if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED)) {
			*++sp = const0;
			assign_svalue(lval, &const0);
		    } else {
			push_svalue(lval);
		    }
		    break;
		case PUSH_GLOBAL:
		    lval = find_value((int)((i & PUSH_MASK) + variable_index_offset));
		    if ((lval->type == T_OBJECT) && (lval->u.ob->flags & O_DESTRUCTED)) {
			*++sp = const0;
			assign_svalue(lval, &const0);
		    } else {
			push_svalue(lval);
		    }
		    break;
		case PUSH_NUMBER:
		    push_number(i & PUSH_MASK);
		    break;
		}
	    }
	    break;
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
		++*global_lvalue_byte.u.lvalue_byte;
		break;
	    default:
		error("++ of non-numeric argument\n");
	    }
	    break;
	case F_WHILE_DEC:
	    {
		svalue_t *s;

		s = fp + EXTRACT_UCHAR(pc++);
		if (s->type == T_NUMBER) {
		    i = s->u.number--;
		} else if (s->type == T_REAL) {
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
		pc = current_prog->program + offset;
	    }
	    if (i) {
		sp--;		/* when sp is an integer svalue, its cheaper
				 * to do this */
	    } else {
		pop_stack();
	    }
	    break;
#endif
	case F_BRANCH:		/* relative offset */
	    COPY_SHORT(&offset, pc);
	    pc += offset;
	    break;
	case F_BBRANCH:		/* relative offset */
	    COPY_SHORT(&offset, pc);
	    pc -= offset;
	    break;
	case F_BRANCH_NE:
	    f_ne();
	    if ((sp--)->u.number) {
		COPY_SHORT(&offset, pc);
		pc += offset;
	    } else
		pc += 2;
	    break;
	case F_BRANCH_GE:
	    f_ge();
	    if ((sp--)->u.number) {
		COPY_SHORT(&offset, pc);
		pc += offset;
	    } else
		pc += 2;
	    break;
	case F_BRANCH_LE:
	    f_le();
	    if ((sp--)->u.number) {
		COPY_SHORT(&offset, pc);
		pc += offset;
	    } else
		pc += 2;
	    break;
	case F_BRANCH_EQ:
	    f_eq();
	    if ((sp--)->u.number) {
		COPY_SHORT(&offset, pc);
		pc += offset;
	    } else
		pc += 2;
	    break;
	case F_BBRANCH_LT:
	    f_lt();
	    if ((sp--)->u.number) {
		COPY_SHORT(&offset, pc);
		pc -= offset;
	    } else
		pc += 2;
	    break;
	case F_BRANCH_WHEN_ZERO: /* relative offset */
	    if (sp->type == T_NUMBER) {
		if (!((sp--)->u.number)) {
		    COPY_SHORT(&offset, pc);
		    pc += offset;
		    break;
		}
	    } else pop_stack();
	    pc += 2;		/* skip over the offset */
	    break;
	case F_BRANCH_WHEN_NON_ZERO: /* relative offset */
	    if (sp->type == T_NUMBER) {
		if (!((sp--)->u.number)) {
		    pc += 2;
		    break;
		}
	    } else pop_stack();
	    COPY_SHORT(&offset, pc);
	    pc += offset;
	    break;
	case F_BBRANCH_WHEN_ZERO: /* relative backwards offset */
	    if (sp->type == T_NUMBER) {
		if (!((sp--)->u.number)) {
		    COPY_SHORT(&offset, pc);
		    pc -= offset;
		    break;
		}
	    } else pop_stack();
	    pc += 2;
	    break;
	case F_BBRANCH_WHEN_NON_ZERO: /* relative backwards offset */
	    if (sp->type == T_NUMBER) {
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
	    if (sp->type == T_NUMBER) {
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
	    if (sp->type == T_NUMBER) {
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
		svalue_t *s;
		
		s = fp + EXTRACT_UCHAR(pc++);
		if (s->type == T_NUMBER) {
		    s->u.number++;
		} else if (s->type == T_REAL) {
		    s->u.real++;
		} else {
		    error("++ of non-numeric argument\n");
		}
	    }
	    if (*pc == F_LOOP_COND_LOCAL) {
		pc++;
		do_loop_cond_local();
	    } else if (*pc == F_LOOP_COND_NUMBER) {
		pc++;
		do_loop_cond_number();
	    }
	    break;
	case F_LOOP_COND_LOCAL:
	    do_loop_cond_local();
	    break;
	case F_LOOP_COND_NUMBER:
	    do_loop_cond_number();
	    break;
	case F_TRANSFER_LOCAL:
	    {
		svalue_t *s;
		
		s = fp + EXTRACT_UCHAR(pc++);
		DEBUG_CHECK((fp-s) >= csp->num_local_variables,
			    "Tried to push non-existent local\n");
		if ((s->type == T_OBJECT) && (s->u.ob->flags & O_DESTRUCTED)) {
		    *++sp = const0;
		    assign_svalue(s, &const0);
		} else {
		    *++sp = *s;
		    s->type = T_NUMBER;
		}
		break;
	    }
	case F_LOCAL:
	    {
		svalue_t *s;
		
		s = fp + EXTRACT_UCHAR(pc++);
		DEBUG_CHECK((fp-s) >= csp->num_local_variables,
			    "Tried to push non-existent local\n");
		
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
	case F_LT:
	    f_lt();
	    break;
	case F_ADD:
	    {
		switch (sp->type) {
		case T_BUFFER:
		    {
			if (!((sp-1)->type == T_BUFFER)) {
			    error("Bad type argument to +. Had %s and %s.\n",
				  type_name((sp - 1)->type), type_name(sp->type));
			} else {
			    buffer_t *b;
			     
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
		case T_ARRAY:
		    {
			if (!((sp-1)->type == T_ARRAY)) {
			    error("Bad type argument to +. Had %s and %s\n",
				  type_name((sp - 1)->type), type_name(sp->type));
			} else {
			    /* add_array now free's the arrays */
			    (sp-1)->u.arr = add_array((sp - 1)->u.arr, sp->u.arr);
			    sp--;
			    break;
			}
		    } /* end of x + T_ARRAY */
		case T_MAPPING:
		    {
			if ((sp-1)->type == T_MAPPING) {
			    mapping_t *map;
			
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
	    sp--;		/* points to the RHS */
	    switch (lval->type) {
	    case T_STRING:
		if (sp->type == T_STRING) {
		    SVALUE_STRING_JOIN(lval, sp, "f_add_eq: 1");
		} else if (sp->type == T_NUMBER) {
		    char buff[20];
		     
		    sprintf(buff, "%d", sp->u.number);
		    EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
		} else if (sp->type == T_REAL) {
		    char buff[40];
		     
		    sprintf(buff, "%f", sp->u.real);
		    EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
		} else {
		    bad_argument(sp, T_STRING | T_NUMBER | T_REAL, 2, instruction);
		}
		break;
	    case T_NUMBER:
		if (sp->type == T_NUMBER) {
		    lval->u.number += sp->u.number;
		    /* both sides are numbers, no freeing required */
		} else if (sp->type == T_REAL) {
		    lval->u.number += sp->u.real;
		    /* both sides are numbers, no freeing required */
		} else {
		    error("Left hand side of += is a number (or zero); right side is not a number.\n");
		}
		break;
	    case T_REAL:
		if (sp->type == T_NUMBER) {
		    lval->u.real += sp->u.number;
		    /* both sides are numerics, no freeing required */
		}
		if (sp->type == T_REAL) {
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
		    buffer_t *b;
		     
		    b = allocate_buffer(lval->u.buf->size + sp->u.buf->size);
		    memcpy(b->item, lval->u.buf->item, lval->u.buf->size);
		    memcpy(b->item + lval->u.buf->size, sp->u.buf->item,
			   sp->u.buf->size);
		    free_buffer(sp->u.buf);
		    free_buffer(lval->u.buf);
		    lval->u.buf = b;
		}
		break;
	    case T_ARRAY:
		if (sp->type != T_ARRAY)
		    bad_argument(sp, T_ARRAY, 2, instruction);
		else {
		    /* add_array now frees the arrays */
		    lval->u.arr = add_array(lval->u.arr, sp->u.arr);
		}
		break;
	    case T_MAPPING:
		if (sp->type != T_MAPPING)
		    bad_argument(sp, T_MAPPING, 2, instruction);
		else {
		    absorb_mapping(lval->u.map, sp->u.map);
		    free_mapping(sp->u.map); /* free RHS */
		    /* LHS not freed because its being reused */
		}
		break;
	    case T_LVALUE_BYTE:
		if (sp->type != T_NUMBER)
		    error("Bad right type to += of char lvalue.\n");
		else *global_lvalue_byte.u.lvalue_byte += sp->u.number;
		break;
	    default:
		bad_arg(1, instruction);
	    }
	     
	    if (instruction == F_ADD_EQ) { /* not void add_eq */
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

	case F_FOREACH:
	    {
		int flags = EXTRACT_UCHAR(pc++);

		IF_DEBUG(foreach_in_progress++);
		if (flags & 4) {
		    CHECK_TYPES(sp, T_MAPPING, 2, F_FOREACH);
		    
		    push_refed_array(mapping_indices(sp->u.map));
		    
		    (++sp)->type = T_NUMBER;
		    sp->u.lvalue = (sp-1)->u.arr->item;
		    sp->subtype = (sp-1)->u.arr->size;
		    
		    (++sp)->type = T_LVALUE;
		    if (flags & 2)
			sp->u.lvalue = find_value((int)(EXTRACT_UCHAR(pc++) + variable_index_offset));
		    else
			sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
		} else {
		    CHECK_TYPES(sp, T_ARRAY, 2, F_FOREACH);

		    (++sp)->type = T_NUMBER;
		    sp->u.lvalue = (sp-1)->u.arr->item;
		    sp->subtype = (sp-1)->u.arr->size;
		}

		(++sp)->type = T_LVALUE;
		if (flags & 1)
		    sp->u.lvalue = find_value((int)(EXTRACT_UCHAR(pc++) + variable_index_offset));
		else
		    sp->u.lvalue = fp + EXTRACT_UCHAR(pc++);
		break;
	    }
	case F_NEXT_FOREACH:
	    if ((sp-1)->type == T_LVALUE) {
		/* mapping */
		if ((sp-2)->subtype--) {
		    svalue_t *key = (sp-2)->u.lvalue++;
		    svalue_t *value = find_in_mapping((sp-4)->u.map, key);
		    
		    assign_svalue((sp-1)->u.lvalue, key);
		    assign_svalue(sp->u.lvalue, value);
		    COPY_SHORT(&offset, pc);
		    pc -= offset;
		    break;
		}
	    } else {
		/* array */
		if ((sp-1)->subtype--) {
		    assign_svalue(sp->u.lvalue, (sp-1)->u.lvalue++);
		    COPY_SHORT(&offset, pc);
		    pc -= offset;
		    break;
		}
	    }
	    pc += 2;
	    /* fallthrough */
	case F_EXIT_FOREACH:
	    IF_DEBUG(foreach_in_progress--);
	    if ((sp-1)->type == T_LVALUE) {
		/* mapping */
		sp -= 3;
		free_array((sp--)->u.arr);
		free_mapping((sp--)->u.map);
	    } else {
		/* array */
		sp -= 2;
		free_array((sp--)->u.arr);
	    }
	    break;

	case F_EXPAND_VARARGS:
	    {
		svalue_t *s, *t;
		array_t *arr;
		
		i = EXTRACT_UCHAR(pc++);
		s = sp - i;
		
		if (s->type != T_ARRAY)
		    error("Item being expanded with ... is not an array\n");
		
		arr = s->u.arr;
		n = arr->size;
		num_varargs += n - 1;
		if (!n) {
		    t = s;
		    while (t < sp) {
			*t = *(t + 1);
			t++;
		    }
		    sp--;
		} else if (n == 1) {
		    assign_svalue_no_free(s, &arr->item[0]);
		} else {
		    t = sp;
		    sp += n - 1;
		    while (t > s) {
			*(t + n - 1) = *t;
			t--;
		    }
		    t = s + n - 1;
		    if (arr->ref == 1) {
			memcpy(s, arr->item, n * sizeof(svalue_t));
			free_empty_array(arr);
			break;
		    } else {
			while (n--)
			    assign_svalue_no_free(t--, &arr->item[n]);
		    }
		}
		free_array(arr);
		break;
	    }
	    
	case F_NEW_CLASS:
	    {
		array_t *cl;

		cl = allocate_class(&current_prog->classes[EXTRACT_UCHAR(pc++)]);
		push_refed_class(cl);
	    }
	    break;
	case F_AGGREGATE:
	    {
		array_t *v;
		
		LOAD_SHORT(offset, pc);
		offset += num_varargs;
		num_varargs = 0;
		v = allocate_empty_array((int) offset);
		/*
		 * transfer svalues in reverse...popping stack as we go
		 */
		while (offset--)
		    v->item[offset] = *sp--;
		(++sp)->type = T_ARRAY;
		sp->u.arr = v;
	    }
	    break;
	case F_AGGREGATE_ASSOC:
	    {
		mapping_t *m;
		
		LOAD_SHORT(offset, pc);

		offset += num_varargs;
		num_varargs = 0;
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
		    *global_lvalue_byte.u.lvalue_byte = ((sp - 1)->u.number & 0xff);
		}
		break;
	    default:
		assign_svalue(sp->u.lvalue, sp - 1);
		break;
	    case T_LVALUE_RANGE:
		assign_lvalue_range(sp - 1);
		break;
	    }
	    sp--;		/* ignore lvalue */
	    /* rvalue is already in the correct place */
	    break;
	case F_VOID_ASSIGN_LOCAL:
	    if (sp->type != T_INVALID) {
		lval = fp + EXTRACT_UCHAR(pc++);
		free_svalue(lval, "F_VOID_ASSIGN_LOCAL");
		*lval = *sp--;
	    } else {
		sp--;
		pc++;
	    }
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
			    *global_lvalue_byte.u.lvalue_byte = (sp--)->u.number & 0xff;
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
	case F_CALL_FUNCTION_BY_ADDRESS:
	    {
		function_t *funp;
		
		LOAD_SHORT(offset, pc);
		offset += function_index_offset;
		/*
		 * Find the function in the function table. As the
		 * function may have been redefined by inheritance, we
		 * must look in the last table, which is pointed to by
		 * current_object.
		 */
		DEBUG_CHECK(offset >= current_object->prog->num_functions,
			    "Illegal function index\n");
		
		funp = &current_object->prog->functions[offset];

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
		csp->num_local_variables = EXTRACT_UCHAR(pc++) + num_varargs;
		num_varargs = 0;
		function_index_offset = variable_index_offset = 0;
		funp = setup_new_frame(funp);
		csp->pc = pc;	/* The corrected return address */
#ifdef LPC_TO_C
		if (current_prog->program_size) {
#endif
		    pc = current_prog->program + funp->offset;
#ifdef LPC_TO_C
		} else {
		    DEBUG_CHECK(!(funp->offset),
				"Null function pointer in jump_table.\n");
		    (*
		     ( void (*) PROT((void)) ) (funp->offset)
		     )();
		}
#endif
	    }
	    break;
	case F_CALL_INHERITED:
	    {
		inherit_t *ip = current_prog->inherit + EXTRACT_UCHAR(pc++);
		program_t *temp_prog = ip->prog;
		function_t *funp;
		
		LOAD_SHORT(offset, pc);

		funp = &temp_prog->functions[offset];
		
		push_control_stack(FRAME_FUNCTION, funp);

		caller_type = ORIGIN_LOCAL;
		current_prog = temp_prog;
		
		csp->num_local_variables = EXTRACT_UCHAR(pc++) + num_varargs;
		num_varargs = 0;
		
		function_index_offset += ip->function_index_offset;
		variable_index_offset += ip->variable_index_offset;
		
		funp = setup_inherited_frame(funp);
		csp->pc = pc;
#ifdef LPC_TO_C
		if (current_prog->program_size) {
#endif
		    pc = current_prog->program + funp->offset;
#ifdef LPC_TO_C
		} else {
		    DEBUG_CHECK(!(funp->offset),
				"Null function pointer in jump_table.\n");
		    (*
		     ( void (*) PROT((void)) ) (funp->offset)
		     )();
		}
#endif
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
		sp->u.number = --(*global_lvalue_byte.u.lvalue_byte);
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
		--(*global_lvalue_byte.u.lvalue_byte);
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
			if ((sp--)->type == T_NUMBER){
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
	    f_ge();
	    break;
	case F_GT:
	    f_gt();
	    break;
	case F_GLOBAL:
	    {
		svalue_t *s;
		
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
		sp->u.number = ++*global_lvalue_byte.u.lvalue_byte;
		break;
	    default:
		error("++ of non-numeric argument\n");
	    }
	    break;
	case F_MEMBER:
	    { 
		array_t *arr;
		 
		if (sp->type != T_CLASS)
		    error("Tried to take a member of something that isn't a class.\n");
		i = EXTRACT_UCHAR(pc++);
		arr = sp->u.arr;
		if (i >= arr->size) error("Class has no corresponding member.\n");
		assign_svalue_no_free(sp, &arr->item[i]);
		free_array(arr);
		break;
	    }
	case F_MEMBER_LVALUE:
	    { 
		array_t *arr;
		 
		if (sp->type != T_CLASS)
		    error("Tried to take a member of something that isn't a class.\n");
		i = EXTRACT_UCHAR(pc++);
		arr = sp->u.arr;
		if (i >= arr->size) error("Class has no corresponding member.\n");
		sp->type = T_LVALUE;
		sp->u.lvalue = arr->item + i;
		free_array(arr);
		break;
	    }
	case F_INDEX:
	    switch (sp->type) {
	    case T_MAPPING:
		{
		    svalue_t *v;
		    mapping_t *m;

		    v = find_in_mapping(m = sp->u.map, sp - 1);
		    assign_svalue(--sp, v); /* v will always have a
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
	    case T_ARRAY:
		{
		    array_t *arr;

		    if ((sp-1)->type != T_NUMBER)
			error("Indexing an array with an illegal type\n");
		    i = (sp - 1)->u.number;
		    if (i<0) error("Negative index passed to array.\n");
		    arr = sp->u.arr;
		    if (i >= arr->size) error("Array index out of bounds.\n");
		    assign_svalue_no_free(--sp, &arr->item[i]);
		    free_array(arr);
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
	    case T_ARRAY:
		{
		    array_t *arr = sp->u.arr;

		    if ((sp-1)->type != T_NUMBER)
			error("Indexing an array with an illegal type\n");
		    i = arr->size - (sp - 1)->u.number;
		    if (i < 0 || i >= arr->size) error("Array index out of bounds.\n");
		    assign_svalue_no_free(--sp, &arr->item[i]);
		    free_array(arr);
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
		pc = current_prog->program + offset;
	    } else {
		pc += 2;
	    }
	    if (i) {
		sp--;		/* cheaper to do this when sp is an integer
				 * svalue */
	    } else {
		pop_stack();
	    }
	    break;
#endif
#ifdef F_JUMP
	case F_JUMP:
	    COPY_SHORT(&offset, pc);
	    pc = current_prog->program + offset;
	    break;
#endif
	case F_LE:
	    f_le();
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
			if ((--sp)->type == T_NUMBER){
			    sp->type = T_REAL;
			    sp->u.real = sp->u.number * (sp+1)->u.real;
			}
			else sp->u.real *= (sp+1)->u.number;
			break;
		    }
		     
		case T_MAPPING:
		    {
			mapping_t *m;
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
		sp->u.number = (*global_lvalue_byte.u.lvalue_byte)--;
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
		sp->u.number = (*global_lvalue_byte.u.lvalue_byte)++;
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
	case F_RETURN_ZERO:
	    {
		/*
		 * Deallocate frame and return.
		 */
		pop_n_elems(csp->num_local_variables);
		sp++;
		DEBUG_CHECK(sp != fp, "Bad stack at F_RETURN\n");
		*sp = const0;
		pop_control_stack();
#ifdef TRACE
		tracedepth--;
		if (TRACEP(TRACE_RETURN)) {
		    do_trace("Return", "", "");
		    if (TRACEHB) {
			if (TRACETST(TRACE_ARGS)) {
			    add_message(command_giver, " with value: 0");
			}
			add_message(command_giver, "\n");
		    }
		}
#endif
		/* The control stack was popped just before */
		if (csp[1].framekind & FRAME_EXTERNAL)
		    return;
		break;
	    }
	    break;
	case F_RETURN:
	    {
		svalue_t sv;
		
		if (csp->num_local_variables) {
		    sv = *sp--;
		    /*
		     * Deallocate frame and return.
		     */
		    pop_n_elems(csp->num_local_variables);
		    sp++;
		    DEBUG_CHECK(sp != fp, "Bad stack at F_RETURN\n");
		    *sp = sv;	/* This way, the same ref counts are
				 * maintained */
		}
		pop_control_stack();
#ifdef TRACE
		tracedepth--;
		if (TRACEP(TRACE_RETURN)) {
		    do_trace("Return", "", "");
		    if (TRACEHB) {
			if (TRACETST(TRACE_ARGS)) {
			    add_message(command_giver, " with value: ");
			    print_svalue(sp);
			}
			add_message(command_giver, "\n");
		    }
		}
#endif
		/* The control stack was popped just before */
		if (csp[1].framekind & FRAME_EXTERNAL)
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
	    DEBUG_CHECK1(offset >= current_prog->num_strings,
			 "string %d out of range in F_STRING!\n",
			 offset);
	    (++sp)->type = T_STRING;
	    sp->subtype = STRING_SHARED;
	    sp->u.string = ref_string(current_prog->strings[offset]);
	    break;
	case F_SHORT_STRING:
	    DEBUG_CHECK1(EXTRACT_UCHAR(pc) >= current_prog->num_strings,
			 "string %d out of range in F_STRING!\n",
			 EXTRACT_UCHAR(pc));
	    (++sp)->type = T_STRING;
	    sp->subtype = STRING_SHARED;
	    sp->u.string = ref_string(current_prog->strings[EXTRACT_UCHAR(pc++)]);
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
		    if (sp->type == T_REAL) sp->u.real -= (sp+1)->u.number;
		    else {
			sp->type = T_REAL;
			sp->u.real = sp->u.number - (sp+1)->u.real;
		    }
		    break;

		case T_ARRAY:
		    {
			/*
			 * subtract_array already takes care of
			 * destructed objects
			 */
			sp->u.arr = subtract_array(sp->u.arr, (sp+1)->u.arr);
			break;
		    }
		    
		default:
		    if (!((sp++)->type & (T_NUMBER|T_REAL|T_ARRAY)))
			error("Bad left type to -.\n");
		    else if (!(sp->type & (T_NUMBER|T_REAL|T_ARRAY)))
			error("Bad right type to -.\n");
		    else error("Arguments to - do not have compatible types.\n");
		}
		break;
	    }
	case F_SUB_EQ:
	    f_sub_eq();
	    break;
	case F_SIMUL_EFUN:
	    {
		unsigned short index;
		int num_args;
    
		LOAD_SHORT(index, pc);
		num_args = EXTRACT_UCHAR(pc++) + num_varargs;
		num_varargs = 0;
		call_simul_efun(index, num_args);
	    }
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
		offset = pc + offset - current_prog->program;
		pc += 2;
		
		do_catch(pc, offset);
		
		pc = current_prog->program + offset;
		
		break;
	    }
	case F_END_CATCH:
	    {
		pop_stack();	/* discard expression value */
		free_svalue(&catch_value, "F_END_CATCH");
		catch_value = const0;
		/* We come here when no longjmp() was executed */
		pop_control_stack();
		push_number(0);
		return;		/* return to do_catch */
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
#define Instruction (instruction + ONEARG_MAX)
#ifdef DEBUG
#define CALL_THE_EFUN goto call_the_efun
#else
#define CALL_THE_EFUN (*oefun_table[instruction])(); continue
#endif
	case F_EFUN0:
	    st_num_arg = 0;
	    instruction = EXTRACT_UCHAR(pc++);
	    CALL_THE_EFUN;
	case F_EFUN1:
	    st_num_arg = 1;
	    instruction = EXTRACT_UCHAR(pc++);
	    CHECK_TYPES(sp, instrs2[instruction].type[0], 1, Instruction);
	    CALL_THE_EFUN;
	case F_EFUN2:
	    st_num_arg = 2;
	    instruction = EXTRACT_UCHAR(pc++);
	    CHECK_TYPES(sp - 1, instrs2[instruction].type[0], 1, Instruction);
	    CHECK_TYPES(sp, instrs2[instruction].type[1], 2, Instruction);
	    CALL_THE_EFUN;
	case F_EFUN3:
	    st_num_arg = 3;
	    instruction = EXTRACT_UCHAR(pc++);
	    CHECK_TYPES(sp - 2, instrs2[instruction].type[0], 1, Instruction);
	    CHECK_TYPES(sp - 1, instrs2[instruction].type[1], 2, Instruction);
	    CHECK_TYPES(sp, instrs2[instruction].type[2], 3, Instruction);
	    CALL_THE_EFUN;
	case F_EFUNV:
	    {
		int i, num;
		st_num_arg = EXTRACT_UCHAR(pc++) + num_varargs;
		num_varargs = 0;
		instruction = EXTRACT_UCHAR(pc++);
		num = instrs2[instruction].min_arg;
		for (i = 1; i <= num; i++) {
		    CHECK_TYPES(sp - st_num_arg + i, instrs2[instruction].type[i-1], i, Instruction);
		}
		CALL_THE_EFUN;
	    }
	default:
	    /* optimized 1 arg efun */
	    st_num_arg = 1;
	    CHECK_TYPES(sp, instrs[instruction].type[0], 1, instruction);
#ifndef DEBUG
	    (*ooefun_table[instruction])();
	    continue;
#else
	    instruction -= ONEARG_MAX;
	call_the_efun:
	    /* We have an efun.  Execute it
	     */
	    if (Instruction > NUM_OPCODES) {
		fatal("Undefined instruction %s (%d)\n",
		      get_f_name(Instruction), Instruction);
	    }
	    if (Instruction < BASE) {
		fatal("No case for eoperator %s (%d)\n",
		      get_f_name(Instruction), Instruction);
	    }
	    if (instrs2[instruction].ret_type == TYPE_NOVALUE)
		expected_stack = sp - st_num_arg;
	    else
		expected_stack = sp - st_num_arg + 1;
	    num_arg = st_num_arg;
	    
	    (*oefun_table[instruction]) ();
	    if (expected_stack != sp)
		fatal("Bad stack after evaluation. Instruction %d, num arg %d\n",
		      instruction, num_arg);
#endif
	} /* switch (instruction) */
	DEBUG_CHECK1(sp < fp + csp->num_local_variables - 1,
		     "Bad stack after evaluation. Instruction %d\n",
		     instruction);
    } /* while (1) */
}

static void
do_catch P2(char *, pc, unsigned short, new_pc_offset)
{
    error_context_t econ;
    
    /*
     * Save some global variables that must be restored separately after a
     * longjmp. The stack will have to be manually popped all the way.
     */
    save_context(&econ);
    push_control_stack(FRAME_CATCH, 0);
#if defined(DEBUG) || defined(TRACE_CODE)
    csp->num_local_variables = (csp - 1)->num_local_variables; /* marion */
#endif

    if (SETJMP(econ.context)) {
	/*
	 * They did a throw() or error. That means that the control stack
	 * must be restored manually here.
	 */
	restore_context(&econ);
	sp++;
	*sp = catch_value;
	catch_value = const1;

	/* if it's too deep or max eval, we can't let them catch it */
	if (max_eval_error) {
	    pop_context(&econ);
	    error("Can't catch eval cost too big error.\n");
	}
	if (too_deep_error) {
	    pop_context(&econ);
	    error("Can't catch too deep recursion error.\n");
	}
    } else {
	assign_svalue(&catch_value, &const1);
	/* note, this will work, since csp->extern_call won't be used */
	eval_instruction(pc);
    }
    pop_context(&econ);
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
    function_t *pr;
    function_t *pr_inherited;
    program_t *progp;
    program_t *oprogp;
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

int apply_low P3(char *, fun, object_t *, ob, int, num_arg)
{
    /*
     * static memory is initialized to zero by the system or so Jacques says
     * :)
     */
    cache_entry_t *entry;
    function_t *pr;
    program_t *progp;
    int ix;
    static int cache_mask = APPLY_CACHE_SIZE - 1;
    char *funname;
    int i, j;
    int local_call_origin = call_origin;
    IF_DEBUG(control_stack_t *save_csp);
    
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
	ix = (progp->id_number ^ (POINTER_INT) fun ^
	      ((POINTER_INT) fun >> APPLY_CACHE_BITS)) & cache_mask;
	entry = &cache[ix];
	if ((entry->id == progp->id_number)
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
	        push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE, entry->pr);
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
	    } /* when we come here, the cache has told us
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
		i = lookup_function(progp->functions,
				    progp->tree_r, funname);
		if (i == -1 || 
		    (progp->functions[i].flags & NAME_UNDEFINED)
		    || ((progp->functions[i].type & (TYPE_MOD_STATIC | TYPE_MOD_PRIVATE))
			&& current_object != ob && !(local_call_origin & (ORIGIN_DRIVER | ORIGIN_CALL_OUT)))) {
		    ;
		} else {
		    pr = (function_t *) & progp->functions[i];
#else
		/* comparing pointers okay since both are shared strings */
		for (pr = progp->functions;
		     pr < progp->functions + progp->num_functions;
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
		    push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE, pr);
		    caller_type = local_call_origin;
		    /* The searched function is found */
		    entry->id = progp->id_number;
		    entry->pr = pr;
		    entry->name = pr->name;
		    csp->num_local_variables = num_arg;
		    current_prog = progp;
		    entry->oprogp = current_prog; /* before
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
	    entry->id = progp->id_number;
	    /* Beek - 99% of the time it's a shared string already */
	    entry->name = make_shared_string(fun);
	    entry->progp = (program_t *) 0;
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
    } /* !failure */
    /* Failure. Deallocate stack. */
    pop_n_elems(num_arg);
    return 0;
}
    
/*
 * Arguments are supposed to be
 * pushed (using push_string() etc) before the call. A pointer to a
 * 'svalue_t' will be returned. It will be a null pointer if the called
 * function was not found. Otherwise, it will be a pointer to a static
 * area in apply(), which will be overwritten by the next call to apply.
 * Reference counts will be updated for this value, to ensure that no pointers
 * are deallocated.
 */
    
svalue_t *apply P4(char *, fun, object_t *, ob, int, num_arg,
		   int, where)
{
    IF_DEBUG(svalue_t *expected_sp);
    
    tracedepth = 0;
    call_origin = where;
    
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


/* Reason for the following 1. save cache space 2. speed :) */
/* The following is to be called only from reset_object for */
/* otherwise extra checks are needed - Sym                  */

void call___INIT P1(object_t *, ob)
{
    function_t *pr;
    program_t *progp;
    int num_functions;
    IF_DEBUG(svalue_t *expected_sp);
    IF_DEBUG(control_stack_t *save_csp);
    
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
    num_functions = progp->num_functions;
    if (!num_functions) return;
    
    /* ___INIT turns out to be always the last function */
    pr = progp->functions + num_functions - 1;
    if (*pr->name != APPLY___INIT_SPECIAL_CHAR) return;
    push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE, pr);
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

svalue_t *
safe_apply P4(char *, fun, object_t *, ob, int, num_arg, int, where)
{
    svalue_t *ret;
    error_context_t econ;

    save_context(&econ);
    if (!SETJMP(econ.context)) {
	if (!(ob->flags & O_DESTRUCTED)) {
	    ret = apply(fun, ob, num_arg, where);
	} else ret = NULL;
    } else {
	restore_context(&econ);
	ret = NULL;
    }
    pop_context(&econ);
    return ret;
}

/*
 * Call a function in all objects in a array.
 */
array_t *call_all_other P3(array_t *, v, char *, func, int, numargs)
{
    int size;
    svalue_t *tmp, *vptr, *rptr;
    array_t *ret;
    object_t *ob;
    int i;

    tmp = sp;
    (++sp)->type = T_ARRAY;
    sp->u.arr = ret = allocate_array(size = v->size);
    if (size && (sp + numargs >= end_of_stack)) {
	too_deep_error = 1;
	error("stack overflow\n");
    }
    for (vptr = v->item, rptr = ret->item; size--; vptr++, rptr++) {
	if (vptr->type == T_OBJECT) {
	    ob = vptr->u.ob;
	} else if (vptr->type == T_STRING) {
	    ob = find_object(vptr->u.string);
	    if (!ob || !object_visible(ob))
		continue;
	} else continue;
	if (ob->flags & O_DESTRUCTED) 
	    continue;
	i = numargs;
	while (i--) push_svalue(tmp - i);
	call_origin = ORIGIN_CALL_OTHER;
	if (apply_low(func, ob, numargs)) *rptr = *sp--;
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
char *function_exists P2(char *, fun, object_t *, ob)
{
    function_t *pr;
    char *funname;
    
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    int i;
#endif
    
    DEBUG_CHECK(ob->flags & O_DESTRUCTED,
		"function_exists() on destructed object\n");
    
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    pr = ob->prog->functions;
    /* all function names are in the shared string table */
    if ((funname = findstring(fun))) {
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	i = lookup_function(ob->prog->functions,
			    ob->prog->tree_r, funname);
	if (i != -1) {
	    program_t *progp;
	    
	    pr = (function_t *) & ob->prog->functions[i];
	    if ((pr->flags & NAME_UNDEFINED) ||
		((pr->type & TYPE_MOD_STATIC) && current_object != ob))
		return 0;
#else
	for (; pr < ob->prog->functions + ob->prog->num_functions; pr++) {
	    program_t *progp;
	    
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
		progp = progp->inherit[pr->offset].prog;
		pr = &progp->functions[pr->function_index_offset];
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
    
int is_static P2(char *, fun, object_t *, ob)
{
    char *funname;
    
    DEBUG_CHECK(ob->flags & O_DESTRUCTED,
		"is_static() on destructed object\n");
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    /* all function names are in the shared string table */
    if ((funname = findstring(fun))) {
	function_t *pr;
	
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	int i;
	
	i = lookup_function(ob->prog->functions,
			    ob->prog->tree_r, funname);
	if (i != -1) {
	    pr = (function_t *) & ob->prog->functions[i];
#else
	function_t *limit;
	    
	limit = ob->prog->functions + ob->prog->num_functions;
	for (pr = ob->prog->functions; pr < limit; pr++) {
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
    
void call_function P2(program_t *, progp, function_t *, pr)
{
    if (pr->flags & NAME_UNDEFINED)
	return;
    push_control_stack(FRAME_FUNCTION | FRAME_OB_CHANGE, pr);
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

int find_line P4(char *, p, program_t *, progp,
		 char **, ret_file, int *, ret_line )
{
    int offset;
    unsigned char *lns;
    short abs_line;
    int file_idx;
    
    *ret_file = "";
    *ret_line = 0;
    
    if (!progp) return 1;
    if (progp == &fake_prog) return 2;
    
#if defined(LPC_TO_C)
    /* currently no line number info for compiled programs */
    if (progp->program_size == 0) 
	return 3;
#endif
    
    /*
     * Load line numbers from swap if necessary.  Leave them in memory until
     * look_for_objects_to_swap() swaps them back out, since more errors are
     * likely.
     */
    if (!progp->line_info) {
	load_line_numbers(progp);
	if (!progp->line_info)
	    return 4;
    }
    offset = p - progp->program;
    DEBUG_CHECK2(offset > (int) progp->program_size,
		 "Illegal offset %d in object /%s\n", offset, progp->name);
    
    lns = progp->line_info;
    while (offset > *lns) {
	offset -= *lns;
	lns += 3;
    }
    
    COPY_SHORT(&abs_line, lns + 1);
    
    translate_absolute_line(abs_line, &progp->file_info[2], 
			    &file_idx, ret_line);
    
    *ret_file = progp->strings[file_idx - 1];
    return 0;
}

void get_explicit_line_number_info P4(char *, p, program_t *, prog,
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

char* get_line_number P2(char *, p, program_t *, progp)
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
    control_stack_t *p;
    char *ret = 0;
    function_t *funp;

#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
    svalue_t *ptr;
    int i;
#endif

    if (current_prog == 0)
	return 0;
    if (csp < &control_stack[0]) {
	return 0;
    }
#ifdef TRACE_CODE
    if (how)
	(void) last_instructions();
#endif
    for (p = &control_stack[0]; p < csp; p++) {
	switch (p[0].framekind & FRAME_MASK) {
	case FRAME_FUNCTION:
	    debug_message("'%15s' in '%20s' ('%20s') %s\n",
			  p[0].fr.func->name, p[1].prog->name, p[1].ob->name,
			  get_line_number(p[1].pc, p[1].prog));
	    if (strcmp(p[0].fr.func->name, "heart_beat") == 0)
		ret = p->ob ? p->ob->name : 0;
	    funp = p[0].fr.func;
	    break;
	case FRAME_FUNP:
	    debug_message("'     <function>' in '%20s' ('%20s') %s\n",
			  p[1].prog->name, p[1].ob->name,
			  get_line_number(p[1].pc, p[1].prog));
	    funp = (function_t *)&p[0].fr.funp->f.functional;
	    break;
	case FRAME_FAKE:
	    debug_message("'     <function>' in '%20s' ('%20s') %s\n",
			  p[1].prog->name, p[1].ob->name,
			  get_line_number(p[1].pc, p[1].prog));
	    funp = 0;
	    break;
	case FRAME_CATCH:
	    debug_message("'          CATCH' in '%20s' ('%20s') %s\n",
			  p[1].prog->name, p[1].ob->name,
			  get_line_number(p[1].pc, p[1].prog));
	    funp = 0;
	    break;
#ifdef DEBUG
	default:
	    fatal("unknown type of frame\n");
	    funp = 0;
#endif
	}
#ifdef ARGUMENTS_IN_TRACEBACK
	if (funp) {
	    ptr = p[1].fp;
	    debug_message("arguments were (");
	    for (i = 0; i < funp->num_arg; i++) {
		outbuffer_t outbuf;

		if (i) {
		    debug_message(",");
		}
		outbuf_zero(&outbuf);
		svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
		/* don't need to fix length here */
		debug_message("%s", outbuf.buffer);
		FREE_MSTR(outbuf.buffer);
	    }
	    debug_message(")\n");
	}
#endif
#ifdef LOCALS_IN_TRACEBACK
	if (funp && funp->num_local) {
	    ptr = p[1].fp + funp->num_arg;
	    debug_message("locals were: ");
	    for (i = 0; i < funp->num_local; i++) {
		outbuffer_t outbuf;
	      
		if (i) {
		    debug_message(",");
		}
		outbuf_zero(&outbuf);
		svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
		/* no need to fix length */
		debug_message("%s", outbuf.buffer);
		FREE_MSTR(outbuf.buffer);
	    }	
	    debug_message("\n");
	}
#endif
    }
    switch (p[0].framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
	debug_message("'%15s' in '%20s' ('%20s') %s\n",
		      p[0].fr.func->name,
		      current_prog->name, current_object->name,
		      get_line_number(pc, current_prog));
	funp = p[0].fr.func;
	break;
    case FRAME_FUNP:
	debug_message("'     <function>' in '%20s' ('%20s') %s\n",
		      current_prog->name, current_object->name,
		      get_line_number(pc, current_prog));
	funp = (function_t *)&p[0].fr.funp->f.functional;
	break;
    case FRAME_FAKE:
	debug_message("'     <function>' in '%20s' ('%20s') %s\n",
		      current_prog->name, current_object->name,
		      get_line_number(pc, current_prog));
	funp = 0;
	break;
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
	    outbuffer_t outbuf;

	    if (i) {
		debug_message(",");
	    }
	    outbuf_zero(&outbuf);
	    svalue_to_string(&fp[i], &outbuf, 0, 0, 0);
	    /* no need to fix length */
	    debug_message("%s", outbuf.buffer);
	    FREE_MSTR(outbuf.buffer);
	}
	debug_message(")\n");
    }
#endif
#ifdef LOCALS_IN_TRACEBACK
    if (funp && funp->num_local) {
	ptr = fp + funp->num_arg;
	debug_message("locals were: ");
	for (i = 0; i < funp->num_local; i++) {
	    outbuffer_t outbuf;

	    if (i) {
		debug_message(",");
	    }
	    outbuf_zero(&outbuf);
	    svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
	    /* no need to fix length */
	    debug_message("%s", outbuf.buffer);
	    FREE_MSTR(outbuf.buffer);
	}
	debug_message("\n");
    }
#endif
    return ret;
}

array_t *get_svalue_trace()
{
    control_stack_t *p;
    array_t *v;
    mapping_t *m;
    char *file;
    int line;
    function_t *funp;

#if defined(ARGUMENTS_IN_TRACEBACK) || defined(LOCALS_IN_TRACEBACK)
    svalue_t *ptr;
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
	switch (p[0].framekind & FRAME_MASK) {
	case FRAME_FUNCTION:
	    add_mapping_string(m, "function", p[0].fr.func->name);
	    funp = p[0].fr.func;
	    break;
	case FRAME_CATCH:
	    add_mapping_string(m, "function", "CATCH");
	    funp = 0;
	    break;
	case FRAME_FAKE:
	    add_mapping_string(m, "function", "<function>");
	    funp = 0;
	    break;
	case FRAME_FUNP:
	    add_mapping_string(m, "function", "<function>");
	    funp = (function_t *)&p[0].fr.funp->f.functional;
	    break;
#ifdef DEBUG
	default:
	    fatal("unknown type of frame\n");
	    funp = 0;
#endif
	}
	add_mapping_string(m, "program", p[1].prog->name);
	add_mapping_object(m, "object", p[1].ob);
	get_explicit_line_number_info(p[1].pc, p[1].prog, &file, &line);
	add_mapping_string(m, "file", file);
	add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
	if (funp) {
	    array_t *v2;

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
	    array_t *v2;

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
    switch (p[0].framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
	add_mapping_string(m, "function", p[0].fr.func->name);
	funp = p[0].fr.func;
	break;
    case FRAME_CATCH:
	add_mapping_string(m, "function", "CATCH");
	funp = 0;
	break;
    case FRAME_FAKE:
	add_mapping_string(m, "function", "<function>");
	funp = 0;
	break;
    case FRAME_FUNP:
	add_mapping_string(m, "function", "<function>");
	funp = (function_t *)&p[0].fr.funp->f.functional;
	break;
    }
    add_mapping_string(m, "program", current_prog->name);
    add_mapping_object(m, "object", current_object);
    get_line_number_info(&file, &line);
    add_mapping_string(m, "file", file);
    add_mapping_pair(m, "line", line);
#ifdef ARGUMENTS_IN_TRACEBACK
    if (funp) {
	array_t *v2;

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
	array_t *v2;

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
    /* return a reference zero array */
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
int inter_sscanf P4(svalue_t *, arg, svalue_t *, s0, svalue_t *, s1, int, num_arg)
{
    char *fmt;			/* Format description */
    char *in_string;		/* The string to be parsed. */
    int number_of_matches;
    int skipme;			/* Encountered a '*' ? */
    int base = 10;
    int num;
    char *match, old_char;
    register char *tmp;
    
    /*
     * First get the string to be parsed.
     */
    CHECK_TYPES(s0, T_STRING, 1, F_SSCANF);
    in_string = s0->u.string;

    /*
     * Now get the format description.
     */
    CHECK_TYPES(s1, T_STRING, 2, F_SSCANF);
    fmt = s1->u.string;

    /*
     * Loop for every % or substring in the format.
     */
    for (number_of_matches = 0; num_arg >= 0; number_of_matches++) {
	while (*fmt) {
	    if (*fmt == '%') {
		if (*++fmt == '%') {
		    if (*in_string++ != '%') return number_of_matches;
		    fmt++;
		    continue;
		}
		break;
	    }
	    if (*fmt++ != *in_string++) return number_of_matches;
	}
	
	if (!*fmt) {
	    if (*s1->u.string && (fmt[-1] == '%'))
		error("Format string cannot end in '%%' in sscanf()\n");
	    
	    /*
	     * We have reached the end of the format string.  If there are
	     * any chars left in the in_string, then we put them in the
	     * last variable (if any).
	     */
	    if (*in_string && num_arg) {
		number_of_matches++;
		SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
	    }
	    break;
	}
	DEBUG_CHECK(fmt[-1] != '%', "In sscanf, should be a %% now!\n");
	
	if ((skipme = (*fmt == '*'))) fmt++;
	else if (num_arg < 1) {
	    /*
	     * Hmm ... maybe we should return number_of_matches here instead
	     * of an error
	     */
	    error("Too few arguments to sscanf()\n");
	}
	
	switch (*fmt++) {
	case 'x':
	    base = 16;
	    /* fallthrough */
	case 'd':
	    {
		tmp = in_string;
		num = (int) strtol(in_string, &in_string, base);
		if (tmp == in_string) return number_of_matches;
		if (!skipme) {
		    SSCANF_ASSIGN_SVALUE_NUMBER(num);
		}
		base = 10;
		continue;
	    }
	case 'f':
	    {
		float tmp_num;
		
		tmp = in_string;
		tmp_num = _strtof(in_string, &in_string);
		if (tmp == in_string)return number_of_matches;
		if (!skipme) {
		    SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
		}
		continue;
	    }
	case '(':
	    {
		struct regexp *reg;
		
		tmp = fmt; /* 1 after the ( */
		num = 1;
		while (1) {
		    switch (*tmp) {
		    case '\\':
			if (*++tmp) {
			    tmp++;
			    continue;
			}
		    case '\0':
			error("Bad regexp format: '%%%s' in sscanf format string\n", fmt);
		    case '(':
			num++;
			/* FALLTHROUGH */
		    default:
			tmp++;
			continue;
		    case ')':
			if (!--num) break;
			tmp++;
			continue;
		    }
		    {
			int n = tmp - fmt;
			char *buf = (char *)DXALLOC(n + 1, TAG_TEMPORARY,
						    "sscanf regexp");
			memcpy(buf, fmt, n);
			buf[n] = 0;
			regexp_user = EFUN_REGEXP;
			reg = regcomp(buf, 0);
			FREE(buf);
			if (!reg) error(regexp_error);
			if (!regexec(reg, in_string) || (in_string != reg->startp[0]))
			    return number_of_matches;
			if (!skipme) {
			    n = *reg->endp - in_string;
			    buf = new_string(n, "sscanf regexp return");
			    memcpy(buf, in_string, n);
			    buf[n] = 0;
			    SSCANF_ASSIGN_SVALUE_STRING(buf);
			}				
			in_string = *reg->endp;
			FREE((char *)reg);
			fmt = ++tmp;
			break;
		    }
		}
		continue;
	    }
	case 's':
	    break;
	default:
	    error("Bad type : '%%%c' in sscanf() format string\n", fmt[-1]);
	}
	
	/*
	 * Now we have the string case.
	 */

	/*
	 * First case: There were no extra characters to match. Then this is
	 * the last match.
	 */
	if (!*fmt) {
	    number_of_matches++;
	    if (!skipme) {
		SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
	    }
	    break;
	}
	/*
	 * If the next char in the format string is a '%' then we have to do
	 * some special checks. Only %d, %f, %x, %(regexp) and %% are allowed
	 * after a %s
	 */
	if (*fmt++ == '%') {
	    int skipme2;
	    
	    tmp = in_string;
	    if ((skipme2 = (*fmt == '*'))) fmt++;
	    else if (num_arg < 2) error("Too few arguments to sscanf().\n");
	    
	    number_of_matches++;
	    
	    switch (*fmt++) {
	    case 's':
		error("Illegal to have 2 adjacent %%s's in format string in sscanf()\n");
	    case 'x':
		do {
		    while (*tmp && (*tmp != '0')) tmp++;
		    if (*tmp == '0') {
			if ((tmp[1] == 'x' || tmp[1] == 'X') &&
			    isxdigit(tmp[2])) break;
			tmp += 2;
		    }
		} while (*tmp);
		break;
	    case 'd':
		while (*tmp && !isdigit(*tmp)) tmp++;
		break;
	    case 'f':
		while (*tmp && !isdigit(*tmp) && 
		       (*tmp != '.' || !isdigit(tmp[1]))) tmp++;
		break;
	    case '%':
		while (*tmp && (*tmp != '%')) tmp++;
		break;
	    case '(':
		{
		    struct regexp *reg;
		    
		    tmp = fmt;
		    num = 1;
		    while (1) {
			switch (*tmp) {
			case '\\':
			    if (*++tmp) {
				tmp++;
				continue;
			    }
			case '\0':
			    error("Bad regexp format : '%%%s' in sscanf format string\n", fmt);
			case '(':
			    num++;
			    /* FALLTHROUGH */
			default:
			    tmp++;
			    continue;
			    
			case ')':
			    if (!--num) break;
			    tmp++;
			    continue;
			}
			{
			    int n = tmp - fmt;
			    char *buf = (char *)DXALLOC(n + 1, TAG_TEMPORARY,
							"sscanf regexp");
			    memcpy(buf, fmt, n);
			    buf[n] = 0;
			    regexp_user = EFUN_REGEXP;
			    reg = regcomp(buf, 0);
			    FREE(buf);
			    if (!reg) error(regexp_error);
			    if (!regexec(reg, in_string)) {
				if (!skipme) {
				    SSCANF_ASSIGN_SVALUE_STRING(string_copy(in_string, "sscanf"));
				}
				FREE((char *)reg);
				return number_of_matches;
			    } else {
				if (!skipme) {
				    match = new_string(num = (*reg->startp - in_string), "inter_sscanf");
				    memcpy(match, in_string, num);
				    match[num] = 0;
				    SSCANF_ASSIGN_SVALUE_STRING(match);
				}
				in_string = *reg->endp;
				if (!skipme2) {
				    match = new_string(num = (*reg->endp - *reg->startp), "inter_sscanf");
				    memcpy(match, *reg->startp, num);
				    match[num] = 0;
				    SSCANF_ASSIGN_SVALUE_STRING(match);
				}
				FREE((char *)reg);
			    }
			    fmt = ++tmp;
			    break;
			}
		    }
		    continue;
		}
		
	    default:
		error("Bad type : '%%%c' in sscanf() format string\n", fmt[-1]);
	    }
	    
	    if (!skipme) {
		match = new_string(num = (tmp - in_string), "inter_sscanf");
		memcpy(match, in_string, num);
		match[num] = 0;
		SSCANF_ASSIGN_SVALUE_STRING(match);
	    }
	    if (!*(in_string = tmp)) return number_of_matches;
	    switch (fmt[-1]) {
	    case 'x':
		base = 16;
	    case 'd':
		{
		    num = (int) strtol(in_string, &in_string, base);
		    /* We already knew it would be matched - Sym */
		    if (!skipme2) {
			SSCANF_ASSIGN_SVALUE_NUMBER(num);
		    }
		    base = 10;
		    continue;
		}
	    case 'f':
		{
		    float tmp_num = _strtof(in_string, &in_string);
		    if (!skipme2) {
			SSCANF_ASSIGN_SVALUE(T_REAL, u.real, tmp_num);
		    }
		    continue;
		}
	    case '%':
		in_string++;
		continue; /* on the big for loop */
	    }
	}
	if ((tmp = strchr(fmt, '%')) != NULL) num = tmp - fmt + 1;
	else {
	    tmp = fmt + (num = strlen(fmt));
	    num++;
	}
	
	old_char = *--fmt;
	match = in_string;
    
	/* This loop would be even faster if it used replace_string's skiptable
	   algorithm.  Maybe that algorithm should be lifted so it can be
	   used in strsrch as well has here, etc? */
	while (*in_string) {
	    if ((*in_string == old_char) && !strncmp(in_string, fmt, num)) {
		/*
		 * Found a match !
		 */
		if (!skipme) {
		    char *newmatch;
		    
		    newmatch = new_string(skipme = (in_string - match), "inter_sscanf");
		    memcpy(newmatch, match, skipme);
		    newmatch[skipme] = 0;
		    SSCANF_ASSIGN_SVALUE_STRING(newmatch);
		}
		in_string += num;
		fmt = tmp; /* advance fmt to next % */
		break;
	    }
	    in_string++;
	}
	if (fmt == tmp)    /* If match, then do continue. */
	    continue;
	
	/*
	 * No match was found. Then we stop here, and return the result so
	 * far !
	 */
	break;
    }
    return number_of_matches;
}

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
	error("Path '%s' too long.\n", tfn);
	return;
    }
    strcpy(tbuf, tfn);
    strcpy(tbuf + len, ".efun");
    fn = check_valid_path(tbuf, current_object, "opcprof", 1);
    if (!fn) {
	error("Invalid path '%s' for writing.\n", tbuf);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	error("Unable to open %s.\n", fn);
	return;
    }
    limit = sizeof(opc_efun) / sizeof(opc_t);
    for (i = 0; i < limit; i++) {
	fprintf(fp, "%-30s: %10d\n", opc_efun[i].name, opc_efun[i].count);
    }
    fclose(fp);

    strcpy(tbuf, tfn);
    strcpy(tbuf + len, ".eoper");
    fn = check_valid_path(tbuf, current_object, "opcprof", 1);
    if (!fn) {
	error("Invalid path '%s' for writing.\n", tbuf);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	error("Unable to open %s for writing.\n", fn);
	return;
    }
    for (i = 0; i < BASE; i++) {
	fprintf(fp, "%-30s: %10d\n",
		query_instr_name(i), opc_eoper[i]);
    }
    fclose(fp);
}
#endif

/* dump # of times each efun has been used */
#ifdef OPCPROF_2D
typedef struct {
    int op1, op2;
    int num_calls;
} sort_elem_t;

int sort_elem_cmp P2(sort_elem_t *, se1, sort_elem_t *, se2) {
    return se2->num_calls - se1->num_calls;
}

void opcdump P1(char *, tfn)
{
    int ind, i, j, len, limit;
    char tbuf[SMALL_STRING_SIZE], *fn;
    FILE *fp;
    sort_elem_t ops[(BASE + 1) * (BASE + 1)];

    if ((len = strlen(tfn)) >= (SMALL_STRING_SIZE - 10)) {
	error("Path '%s' too long.\n", tfn);
	return;
    }
    strcpy(tbuf, tfn);
    strcpy(tbuf + len, ".eop-2d");
    fn = check_valid_path(tbuf, current_object, "opcprof", 1);
    if (!fn) {
	error("Invalid path '%s' for writing.\n", tbuf);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	error("Unable to open %s for writing.\n", fn);
	return;
    }
    for (i = 0; i <= BASE; i++) {
	for (j = 0; j <= BASE; j++) {
	    ind = i * (BASE + 1) + j;
	    ops[ind].num_calls = opc_eoper_2d[i][j];
	    ops[ind].op1 = i;
	    ops[ind].op2 = j;
	}
    }
    quickSort((char *) ops, (BASE + 1) * (BASE + 1), sizeof(sort_elem_t),
	      sort_elem_cmp);
    for (i = 0; i < (BASE + 1) * (BASE + 1); i++) {
	if (ops[i].num_calls)
	    fprintf(fp, "%-30s %-30s: %10d\n", query_instr_name(ops[i].op1),
		    query_instr_name(ops[i].op2), ops[i].num_calls);
    }
    fclose(fp);
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
	IF_DEBUG(foreach_in_progress = 0);
    }
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
	    debug_message("%6x: %3d %8s %-25s (%d)\n", previous_pc[i],
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
    add_vmessage(command_giver, "*** %d %*s %s %s %s%s", tracedepth, tracedepth, "", msg, objname, fname, post);
}
#endif

int assert_master_ob_loaded P2(char *, fun, char *, arg) {
    /* First we check two special conditions.  If master_ob == -1, main.c
     * hasn't tried to load the master object yet, so we shouldn't.
     * if master_ob_is_loading is set, then the master is in the process
     * of loading.
     */
    if (master_ob_is_loading || (master_ob == (object_t *)-1))
	return -1;
    if (!master_ob || (master_ob->flags & O_DESTRUCTED)) {
	object_t *ob;

	ob = load_object(master_file_name, 0);
	if (!ob) {
	    debug_message("%s(%s) failed: Master failed to load.\n", fun, arg);
	    return 0;
	}
    }
    return 1;
}

/* If the master object can't be loaded, we return zero. (svalue_t *)-1
 * means that a secure object is loading.  Default behavior should be that
 * the check succeeds.
 */
svalue_t *apply_master_ob P2(char *, fun, int, num_arg)
{
    POINTER_INT err;
    IF_DEBUG(svalue_t *expected_sp);

    if ((err = assert_master_ob_loaded("apply_master_ob", fun)) != 1) {
	pop_n_elems(num_arg);
	return (svalue_t *)err;
    }
    call_origin = ORIGIN_DRIVER;

#ifdef TRACE
    if (TRACEP(TRACE_APPLY)) {
	do_trace("Apply", "", "\n");
    }
#endif

    IF_DEBUG(expected_sp = sp - num_arg);
    if (apply_low(fun, master_ob, num_arg) == 0)
	return 0;
    free_svalue(&apply_ret_value, "sapply");
    apply_ret_value = *sp--;
    DEBUG_CHECK(expected_sp != sp,
		"Corrupt stack pointer.\n");
    return &apply_ret_value;
}

svalue_t *safe_apply_master_ob P2(char *, fun, int, num_arg)
{
    POINTER_INT err;
    if ((err = assert_master_ob_loaded("safe_apply_master_ob", fun)) != 1) {
	pop_n_elems(num_arg);
	return (svalue_t *)err;
    }
    return safe_apply(fun, master_ob, num_arg, ORIGIN_DRIVER);
}

/*
 * When an object is destructed, all references to it must be removed
 * from the stack.
 */
void remove_object_from_stack P1(object_t *, ob)
{
    svalue_t *svp;

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
    svalue_t *sv;

    for (sv = start_of_stack; sv <= sp; sv++) mark_svalue(sv);
}
#endif

/* Be careful.  This assumes there will be a frame pushed right after this,
   as we use econ->save_csp + 1 to restore */
void save_context P1(error_context_t *, econ) {
    econ->save_command_giver = command_giver;
    econ->save_sp = sp;
    econ->save_csp = csp;
    econ->save_context = current_error_context;

    current_error_context = econ;
}

void pop_context P1(error_context_t *, econ) {
    current_error_context = econ->save_context;
}

/* can the error handler do this ? */
void restore_context P1(error_context_t *, econ) {
    command_giver = econ->save_command_giver;
    csp = econ->save_csp + 1;
    pop_control_stack();
    pop_n_elems(sp - econ->save_sp);
}

