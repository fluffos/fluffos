#if !defined(NeXT)
#include <varargs.h>
#endif
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
/* sys/types.h is here to enable include of comm.h below */
#include <sys/types.h> 
#include <sys/stat.h>
#ifdef MARK
#include <prof.h>
#endif
#include <memory.h>

#ifdef MARK
#define CASE(x) case x: MARK(x);
#else
#define CASE(x) case x:
#endif

#include "lint.h"
#include "lang.tab.h"
#include "exec.h"
#include "interpret.h"
#include "config.h"
#include "object.h"
#include "wiz_list.h"
#include "instrs.h"
#include "patchlevel.h"
#include "comm.h"
#include "switch.h"

#ifdef RUSAGE			/* Defined in config.h */
#include <sys/resource.h>
extern int getrusage PROT((int, struct rusage *));
#ifdef sun
extern int getpagesize();
#endif
#ifndef RUSAGE_SELF
#define RUSAGE_SELF	0
#endif
#endif

extern struct object *master_ob;
extern struct wiz_list *back_bone_uid;
extern char *master_file_name;

extern void print_svalue PROT((struct svalue *));
static struct svalue *sapply PROT((char *, struct object *, int));
static void do_trace PROT((char *, char *, char *));
static int apply_low PROT((char *, struct object *, int));
static int inter_sscanf PROT((int));
static int strpref PROT((char *, char *));
extern int do_rename PROT((char *, char *, int));     

extern struct object *previous_ob;
extern char *last_verb;
extern struct svalue const0, const1, const0u;
struct program *current_prog;
extern int current_time;
extern struct object *current_heart_beat, *current_interactive;

static int tracedepth;
#define TRACE_CALL 1
#define TRACE_CALL_OTHER 2
#define TRACE_RETURN 4
#define TRACE_ARGS 8
#define TRACE_EXEC 16
#define TRACE_HEART_BEAT 32
#define TRACE_APPLY 64
#define TRACE_OBJNAME 128
#define TRACETST(b) (command_giver->interactive->trace_level & (b))
#define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#define TRACEHB (current_heart_beat == 0 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))

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
 * loaded separetly. X will then be reloaded again.
 */
extern int d_flag;

extern int current_line, eval_cost;

/*
 * These are the registers used at runtime.
 * The control stack saves registers to be restored when a function
 * will return. That means that control_stack[0] will have almost no
 * interesting values, as it will terminate execution.
 */
static char *pc;		/* Program pointer. */
static struct svalue *fp;	/* Pointer to first argument. */
static struct svalue *sp;	/* Points to value of last push. */
static short *break_sp;		/* Points to address to branch to
				 * at next F_BREAK			*/
static int function_index_offset; /* Needed for inheritance */
static int variable_index_offset; /* Needed for inheritance */

static struct svalue start_of_stack[EVALUATOR_STACK_SIZE];
struct svalue catch_value;	/* Used to throw an error to a catch */

static struct control_stack control_stack[MAX_TRACE];
static struct control_stack *csp;	/* Points to last element pushed */

void get_version(buff)
char *buff;
{
  sprintf(buff, "MudOS driver %s%d", 
	  VERSION, PATCH_LEVEL);
}

/*
 * May current_object shadow object 'ob' ? We rely heavily on the fact that
 * function names are pointers to shared strings, which means that equality
 * can be tested simply through pointer comparison.
 */
#ifndef NO_SHADOWS /* LPCA */
int validate_shadowing(ob)
    struct object *ob;
{
    int i, j;
    struct program *shadow = current_object->prog, *victim = ob->prog;
    struct svalue *ret;

	if (current_object->shadowing)
		error("shadow: Already shadowing.\n");
	if (current_object->shadowed)
		error("shadow: Can't shadow when shadowed.\n");
	if (current_object->super)
		error("The shadow must not reside inside another object.\n");
	if (ob->flags & O_MASTER)
		error("shadow: cannot shadow the master object.\n");
	if (ob->shadowing)
		error("Can't shadow a shadow.\n");
    for (i=0; i < shadow->num_functions; i++) {
	for (j=0; j < victim->num_functions; j++) {
	    if (shadow->functions[i].name != victim->functions[j].name)
		continue;
	    if (victim->functions[j].type & TYPE_MOD_NO_MASK)
		error("Illegal to shadow 'nomask' function \"%s\".\n",
		      victim->functions[j].name);
	}
    }
    push_object(ob);
    ret = apply_master_ob("valid_shadow", 1);
    if (!(ob->flags & O_DESTRUCTED) &&
	ret && !(ret->type == T_NUMBER && ret->u.number == 0))
    {
	return 1;
    }
    return 0;
}
#endif

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
void push_object(ob)
    struct object *ob;
{
    sp++;
    if (sp == &start_of_stack[EVALUATOR_STACK_SIZE])
	fatal("stack overflow\n");
    sp->type = T_OBJECT;
    sp->u.ob = ob;
    add_ref(ob, "push_object");
}

/*
 * Push a number on the value stack.
 */
INLINE
void push_number(n)
    int n;
{
    sp++;
    if (sp == &start_of_stack[EVALUATOR_STACK_SIZE])
		fatal("stack overflow\n");
    sp->type = T_NUMBER;
    sp->u.number = n;
}

/*
 * Push undefined (const0u) onto the value stack.
 */
INLINE
void push_undefined()
{
    sp++;
    if (sp == &start_of_stack[EVALUATOR_STACK_SIZE])
		fatal("stack overflow\n");
	*sp = const0u;
}

/*
 * Push a string on the value stack.
 */
INLINE
void push_string(p, type)
    char *p;
    int type;
{
    sp++;
    if (sp == &start_of_stack[EVALUATOR_STACK_SIZE])
	fatal("stack overflow\n");
    sp->type = T_STRING;
    sp->subtype = type;
    switch(type) {
    case STRING_MALLOC:
	sp->u.string = string_copy(p);
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
static INLINE struct svalue *find_value(num)
    int num;
{
#ifdef DEBUG
    if (num >= current_object->prog->num_variables) {
	fatal("Illegal variable access %d(%d). See trace above.\n",
	    num, current_object->prog->num_variables);
    }
#endif
    return &current_object->variables[num];
}

/*
 * Free the data that an svalue is pointing to. Not the svalue
 * itself.
 */
void free_svalue(v)
    struct svalue *v;
{
    switch(v->type) {
    case T_STRING:
	switch(v->subtype) {
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
    case T_MAPPING:
        free_mapping(v->u.map);
    }
    *v = const0; /* marion - clear this value all away */
}

/*
 * Prepend a slash in front of a string.
 */
static char *add_slash(str)
    char *str;
{
    char *tmp;

    tmp = xalloc(strlen(str)+2);
    strcpy(tmp,"/"); strcat(tmp,str);
    return tmp;
}

/*
 * Assign to a svalue.
 * This is done either when element in vector, or when to an identifier
 * (as all identifiers are kept in a vector pointed to by the object).
 */

INLINE void assign_svalue_no_free(to, from)
    struct svalue *to;
    struct svalue *from;
{
#ifdef DEBUG
    if (from == 0)
	fatal("Null pointer to assign_svalue().\n");
#endif
    *to = *from;
    switch(from->type) {
    case T_STRING:
	switch(from->subtype) {
	case STRING_MALLOC:	/* No idea to make the string shared */
	    to->u.string = string_copy(from->u.string);
	    break;
	case STRING_CONSTANT:	/* Good idea to make it shared */
	    to->subtype = STRING_SHARED;
	    /* FALL THROUGH ! */
	case STRING_SHARED:	/* It already is shared */
	    to->u.string = make_shared_string(from->u.string);
	    break;
	default:
	    fatal("Bad string type %d\n", from->subtype);
	}
	break;
    case T_OBJECT:
	add_ref(to->u.ob, "ass to var");
	break;
    case T_POINTER:
	to->u.vec->ref++;
	break;
    case T_MAPPING:
        to->u.map->ref++;
         break;
    }
}

INLINE void assign_svalue(dest, v)
    struct svalue *dest;
    struct svalue *v;
{
    /* First deallocate the previous value. */
    free_svalue(dest);
    assign_svalue_no_free(dest, v);
}

void push_svalue(v)
    struct svalue *v;
{
    sp++;
    assign_svalue_no_free(sp, v);
}

/*
 * Pop the top-most value of the stack.
 * Don't do this if it is a value that will be used afterwards, as the
 * data may be sent to FREE(), and destroyed.
 */
static INLINE void pop_stack() {
#ifdef DEBUG
    if (sp < start_of_stack)
	fatal("Stack underflow.\n");
#endif
    free_svalue(sp);
    sp--;
}

/*
 * Compute the address of an array element.
 */
static INLINE void push_indexed_lvalue()
{
    struct svalue *i, *vec, *item;
    int ind;

    i = sp;
    vec = sp - 1;
    if (vec->type == T_MAPPING) {
       struct mapping *m = vec->u.map;
       vec = find_for_insert(m, i);
       pop_stack();
       free_svalue(sp);
       sp->type = T_LVALUE;
       sp->u.lvalue = vec;
       return;
    }
    ind = i->u.number;
    pop_stack();
    if (i->type != T_NUMBER || i->u.number < 0)
	error("Illegal index\n");
    if (vec->type == T_STRING) {
	static struct svalue one_character;
	/* marion says: this is a crude part of code */
	one_character.type = T_NUMBER;
	if (ind > strlen(vec->u.string) || ind < 0)
	    one_character.u.number = 0;
	else
	    one_character.u.number = vec->u.string[ind];
	free_svalue(sp);
	sp->type = T_LVALUE;
	sp->u.lvalue = &one_character;
	return;
    }
    if (vec->type != T_POINTER) error("Indexing on illegal type.\n");
    if (ind >= vec->u.vec->size) error ("Index out of bounds\n");
    item = &vec->u.vec->item[ind];
    if (vec->u.vec->ref == 1) {
	static struct svalue quickfix = { T_NUMBER };
	/* marion says: but this is crude too */
	/* marion blushes. */
	assign_svalue (&quickfix, item);
	item = &quickfix;
    }
    free_svalue(sp);		/* This will make 'vec' invalid to use */
    sp->type = T_LVALUE;
    sp->u.lvalue = item;
}

#ifdef OPCPROF
#define MAXOPC 512
static int opcount[MAXOPC];
#endif

/*
 * Deallocate 'n' values from the stack.
 */
INLINE
void pop_n_elems(n)
    int n;
{
#ifdef DEBUG
    if (n < 0)
	fatal("pop_n_elems: %d elements.\n", n);
#endif
    for (; n>0; n--)
	pop_stack();
}

void bad_arg(arg, instr)
    int arg, instr;
{
    error("Bad argument %d to %s()\n", arg, get_f_name(instr));
}

INLINE
static void push_control_stack(funp)
    struct function *funp;
{
    if (csp == &control_stack[MAX_TRACE-1])
	error("Too deep recursion.\n");
    csp++;
    csp->funp = funp;	/* Only used for tracebacks */
    csp->ob = current_object;
    csp->prev_ob = previous_ob;
    csp->fp = fp;
    csp->prog = current_prog;
    /* csp->extern_call = 0; It is set by eval_instruction() */
    csp->pc = pc;
    csp->function_index_offset = function_index_offset;
    csp->variable_index_offset = variable_index_offset;
    csp->break_sp = break_sp;
}

/*
 * Pop the control stack one element, and restore registers.
 * extern_call must not be modified here, as it is used imediately after pop.
 */
static void pop_control_stack() {
#ifdef DEBUG
    if (csp == control_stack - 1)
	fatal("Popped out of the control stack");
#endif
    current_object = csp->ob;
    current_prog = csp->prog;
    previous_ob = csp->prev_ob;
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
INLINE void push_vector(v)
    struct vector *v;
{
    v->ref++;
    sp++;
    sp->type = T_POINTER;
    sp->u.vec = v;
}

/*
 * Push a mapping on the stack.  See push_vector(), above.
 */
INLINE void push_mapping(m)
    struct mapping *m;
{
    m->ref++;
    sp++;
    sp->type = T_MAPPING;
    sp->u.map = m;
}

/*
 * Push a string on the stack that is already malloced.
 */
static void INLINE push_malloced_string(p)
    char *p;
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
void push_constant_string(p)
    char *p;
{
    sp++;
    sp->type = T_STRING;
    sp->u.string = p;
    sp->subtype = STRING_CONSTANT;
}

static void do_trace_call(funp)
    struct function *funp;
{
    do_trace("Call direct ", funp->name, " ");
    if (TRACEHB) {
        if (TRACETST(TRACE_ARGS)) {
            int i;
            add_message(" with %d arguments: ", funp->num_arg);
            for(i = funp->num_arg-1; i >= 0; i--) {
                print_svalue(&sp[-i]);
                add_message(" ");
            }
        }
        add_message("\n");
    }
}

/*
 * Argument is the function to execute. If it is defined by inheritance,
 * then search for the real definition, and return it.
 * There is a number of arguments on the stack. Normalize them and initialize
 * local variables, so that the called function is pleased.
 */
static struct function *setup_new_frame(funp)
    struct function *funp;
{
    function_index_offset = 0;
    variable_index_offset = 0;
    while(funp->flags & NAME_INHERITED) {
	function_index_offset +=
	    current_prog->inherit[funp->offset].function_index_offset;
	variable_index_offset +=
	    current_prog->inherit[funp->offset].variable_index_offset;
	current_prog =
	    current_prog->inherit[funp->offset].prog;
	funp = &current_prog->functions[funp->function_index_offset];
    }
    /* Remove excessive arguments */
    while(csp->num_local_variables > funp->num_arg) {
	pop_stack();
	csp->num_local_variables--;
    }
    /* Correct number of arguments and local variables */
    while(csp->num_local_variables < funp->num_arg + funp->num_local) {
	push_number(0);
	csp->num_local_variables++;
    }
    tracedepth++;
    if (TRACEP(TRACE_CALL)) {
	do_trace_call(funp);
    }
    fp = sp - csp->num_local_variables + 1;
    break_sp = (short*)(sp+1);
    return funp;
}

static void break_point()
{
    if (sp - fp - csp->num_local_variables + 1 != 0)
	fatal("Bad stack pointer.\n");
}

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
void push_pop_error_context (push)
    int push;
{
    extern jmp_buf error_recovery_context;
    extern int error_recovery_context_exists;
    static struct error_context_stack {
    jmp_buf old_error_context;
    int old_exists_flag;
    struct control_stack *save_csp;
    struct object *save_command_giver;
    struct svalue *save_sp;
    struct error_context_stack *next;
    } *ecsp = 0, *p;

    if (push == 1) {
    /*
     * Save some global variables that must be restored separately
     * after a longjmp. The stack will have to be manually popped all
     * the way.
     */
    p = (struct error_context_stack *)xalloc (sizeof *p);
    p->save_sp = sp;
    p->save_csp = csp;
    p->save_command_giver = command_giver;
    memcpy (
        (char *)p->old_error_context,
        (char *)error_recovery_context,
        sizeof error_recovery_context);
    p->old_exists_flag = error_recovery_context_exists;
    p->next = ecsp;
    ecsp = p;
    } else {
    p = ecsp;
    if (p == 0)
        fatal("Catch: error context stack underflow");
    if (push == 0) {
#ifdef DEBUG
        if (csp != p->save_csp-1)
        fatal("Catch: Lost track of csp");
#if 0
        /*
         * This test is not valid! The statement catch(exec("...")) will
         * change the value of command_giver.
         */
        if (command_giver != p->save_command_giver)
        fatal("Catch: Lost track of command_giver");
#endif
#endif
    } else {
        /* push == -1 !
         * They did a throw() or error. That means that the control
         * stack must be restored manually here.
         */
        csp = p->save_csp;
        pop_n_elems (sp - p->save_sp);
        command_giver = p->save_command_giver;
    }
    memcpy ((char *)error_recovery_context,
        (char *)p->old_error_context,
        sizeof error_recovery_context);
    error_recovery_context_exists = p->old_exists_flag;
    ecsp = p->next;
    FREE((char *)p);
    }
}

static struct vector *prepend_vector(v, a)
struct vector *v;
struct svalue *a;
{
	int i;
	struct vector *new;

	new = allocate_array(v->size+1);
	assign_svalue(&new->item[0], a);
	for (i=0; i<v->size; ++i)
		assign_svalue(&new->item[i+1], &v->item[i]);
	return new;
}

/*
 * When a vector is given as argument to an efun, all items has to be
 * checked if there would be an destructed object.
 * A bad problem currently is that a vector can contain another vector, so this
 * should be tested too. But, there is currently no prevention against
 * recursive vectors, which means that this can not be tested. Thus, the game
 * may crash if a vector contains a vector that contains a destructed object
 * and this top-most vector is used as an argument to an efun.
 */
/* The game won't crash when doing simple operations like assign_svalue
 * on a destructed object. You have to watch out, of course, that you don't
 * apply a function to it.
 * to save space it is preferable that destructed objects are freed soon.
 *   amylaar
 */
void check_for_destr(v)
    struct vector *v;
{
    int i;

    for (i=0; i < v->size; i++) {
	if (v->item[i].type != T_OBJECT)
	    continue;
	if (!(v->item[i].u.ob->flags & O_DESTRUCTED))
	    continue;
	assign_svalue(&v->item[i], &const0);
    }
}

static struct vector *append_vector(v, a)
struct vector *v;
struct svalue *a;
{
   int i;
   struct vector *new;

   new = allocate_array(v->size+1);
   for (i=0; i<v->size; ++i)
      assign_svalue(&new->item[i], &v->item[i]);
   assign_svalue(&new->item[i], a);
   return new;
}

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
int previous_instruction[60];
int stack_size[60];
char *previous_pc[60];
static int last;
#endif
static void eval_instruction(p)
    char *p;
{
    struct object *ob;
    int i, num_arg;
    int instruction;
    struct svalue *expected_stack, *argp;

    /* Next F_RETURN at this level will return out of eval_instruction() */
    csp->extern_call = 1;
    pc = p;
again:
    instruction = EXTRACT_UCHAR(pc);
#ifdef TRACE_CODE
    previous_instruction[last] = instruction + F_OFFSET;
    previous_pc[last] = pc;
    stack_size[last] = sp - fp - csp->num_local_variables;
    last = (last + 1) % (sizeof previous_instruction / sizeof (int));
#endif
    pc++;
    if (current_object->user)
	current_object->user->cost++;
    eval_cost++;
    if (eval_cost > MAX_COST) {
	printf("eval_cost too big %d\n", eval_cost);
        eval_cost = 0;
	error("Too long evaluation. Execution aborted.\n");
    }
    /*
     * Execute current instruction. Note that all functions callable
     * from LPC must return a value. This does not apply to control
     * instructions, like F_JUMP.
     */
    if (instrs[instruction].min_arg != instrs[instruction].max_arg) {
	num_arg = EXTRACT_UCHAR(pc);
	pc++;
	if (num_arg > 0) {
	    if (instrs[instruction].type[0] != 0 &&
		(instrs[instruction].type[0] & (sp-num_arg+1)->type) == 0) {
		bad_arg(1, instruction + F_OFFSET);
	    }
	}
	if (num_arg > 1) {
	    if (instrs[instruction].type[1] != 0 &&
		(instrs[instruction].type[1] & (sp-num_arg+2)->type) == 0) {
		bad_arg(2, instruction + F_OFFSET);
	    }
	}
    } else {
	num_arg = instrs[instruction].min_arg;
	if (instrs[instruction].min_arg > 0) {
	    if (instrs[instruction].type[0] != 0 &&
		(instrs[instruction].type[0] & (sp-num_arg+1)->type) == 0) {
		bad_arg(1, instruction + F_OFFSET);
	    }
	}
	if (instrs[instruction].min_arg > 1) {
	    if (instrs[instruction].type[1] != 0 &&
		(instrs[instruction].type[1] & (sp-num_arg+2)->type) == 0) {
		bad_arg(2, instruction + F_OFFSET);
	    }
	}
	/*
	 * Safety measure. It is supposed that the evaluator knows
	 * the number of arguments.
	 */
	num_arg = -1;
    }
    if (num_arg != -1) {
	expected_stack = sp - num_arg + 1;
#ifdef DEBUG
    } else {
	expected_stack = 0;
#endif
    }
    instruction += F_OFFSET;
#ifdef OPCPROF
    if (instruction >= 0 && instruction < MAXOPC) opcount[instruction]++;
#endif
    /*
     * Execute the instructions. The number of arguments are correct,
     * and the type of the two first arguments are also correct.
     */
    if (TRACEP(TRACE_EXEC)) {
	do_trace("Exec ", get_f_name(instruction), "\n");
    }
    switch(instruction) {
    default:
	fatal("Undefined instruction %s (%d)\n", get_f_name(instruction),
	      instruction);
	/*NOTREACHED*/
    CASE(F_REGEXP);
    {
	struct vector *v;
	v = match_regexp((sp-1)->u.vec, sp->u.string);
	pop_n_elems(2);
	if (v == 0)
	    push_number(0);
	else {
	    push_vector(v);
	    v->ref--;		/* Will make ref count == 1 */
	}
	break;
    }
#ifdef F_SHADOW
    CASE(F_SHADOW);
	ob = (sp-1)->u.ob;
	if (sp->u.number == 0) {
	    ob = ob->shadowed;
	    pop_n_elems(2);
	    if (ob)
		push_object(ob);
	    else
		push_number(0);
	    break;
	}
	if (validate_shadowing(ob)) {
	    /*
	     * The shadow is entered first in the chain.
	     */
	    while (ob->shadowed)
		ob = ob->shadowed;
	    current_object->shadowing = ob;
	    ob->shadowed = current_object;
	    pop_n_elems(2);
	    push_object(ob);
	    break;
	}
	pop_n_elems(2);
	push_number(0);
	break;
#endif
    CASE(F_POP_VALUE);
	pop_stack();
	break;
    CASE(F_DUP);
	sp++;
	assign_svalue_no_free(sp, sp-1);
	break;
    CASE(F_JUMP_WHEN_ZERO);
    {
	unsigned short offset;

	((char *)&offset)[0] = pc[0];
	((char *)&offset)[1] = pc[1];
	if (sp->type == T_NUMBER && sp->u.number == 0)
	    pc = current_prog->program + offset;
	else
	    pc += 2;
	pop_stack();
	break;
    }
    CASE(F_JUMP);
    {
	unsigned short offset;

	((char *)&offset)[0] = pc[0];
	((char *)&offset)[1] = pc[1];
	pc = current_prog->program + offset;
	break;
    }
    CASE(F_JUMP_WHEN_NON_ZERO);
    {
	unsigned short offset;

	((char *)&offset)[0] = pc[0];
	((char *)&offset)[1] = pc[1];
	if (sp->type == T_NUMBER && sp->u.number == 0)
	    pc += 2;
	else
	    pc = current_prog->program + offset;
	pop_stack();
	break;
    }
    CASE(F_INDIRECT);
#ifdef DEBUG
	if (sp->type != T_LVALUE)
	    fatal("Bad type to F_INDIRECT\n");
#endif
	assign_svalue(sp, sp->u.lvalue);
	/*
	 * Fetch value of a variable. It is possible that it is a variable
	 * that points to a destructed object. In that case, it has to
	 * be replaced by 0.
	 */
	if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_svalue(sp);
	    *sp = const0;
	}
	break;
    CASE(F_IDENTIFIER);
	sp++;
	assign_svalue_no_free(sp, find_value((int)(EXTRACT_UCHAR(pc) +
						   variable_index_offset)));
	pc++;
	/*
	 * Fetch value of a variable. It is possible that it is a variable
	 * that points to a destructed object. In that case, it has to
	 * be replaced by 0.
	 */
	if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_svalue(sp);
	    *sp = const0;
	}
	break;
    CASE(F_PUSH_IDENTIFIER_LVALUE);
	sp++;
	sp->type = T_LVALUE;
	sp->u.lvalue = find_value((int)(EXTRACT_UCHAR(pc) +
					variable_index_offset));
	pc++;
	break;
    CASE(F_PUSH_INDEXED_LVALUE);
	push_indexed_lvalue();
	break;
    CASE(F_INDEX);
        if ((sp-1)->type == T_MAPPING) {
           struct svalue *v;
           v = find_in_mapping((sp-1)->u.map, sp);
           pop_stack();
           if (!v)
              assign_svalue(sp, &const0);
           else
              assign_svalue(sp, v);
        } else {
           push_indexed_lvalue();
           assign_svalue_no_free(sp, sp->u.lvalue);
        }
	/*
	 * Fetch value of a variable. It is possible that it is a variable
	 * that points to a destructed object. In that case, it has to
	 * be replaced by 0.
	 */
	if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_svalue(sp);
	    sp->type = T_NUMBER;
	    sp->u.number = 0;
	}
	break;
    CASE(F_LOCAL_NAME);
	sp++;
	assign_svalue_no_free(sp, fp + EXTRACT_UCHAR(pc));
	pc++;
	/*
	 * Fetch value of a variable. It is possible that it is a variable
	 * that points to a destructed object. In that case, it has to
	 * be replaced by 0.
	 */
	if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	    free_svalue(sp);
	    *sp = const0;
	}
	break;
    CASE(F_PUSH_LOCAL_VARIABLE_LVALUE);
	sp++;
	sp->type = T_LVALUE;
	sp->u.lvalue = fp + EXTRACT_UCHAR(pc);
	pc++;
	break;
    CASE(F_RETURN);
    {
	struct svalue sv;

	sv = *sp--;
	/*
	 * Deallocate frame and return.
	 */
	for (i=0; i < csp->num_local_variables; i++)
	    pop_stack();
	sp++;
#ifdef DEBUG
	if (sp != fp)
	    fatal("Bad stack at F_RETURN\n"); /* marion */
#endif
	*sp = sv;	/* This way, the same ref counts are maintained */
	pop_control_stack();
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
	if (csp[1].extern_call)	/* The control stack was popped just before */
	    return;
	break;
    }
    CASE(F_BREAK_POINT);
	break_point();	/* generated by lang.y when -d. Will check stack. */
	break;
    CASE(F_NEW);
    CASE(F_CLONE_OBJECT);
	ob = clone_object(sp->u.string);
	pop_stack();
	if (ob) {
	    sp++;
	    sp->type = T_OBJECT;
	    sp->u.ob = ob;
	    add_ref(ob, "F_NEW");
	} else {
	    push_number(0);
	}
	break;
    CASE(F_AGGREGATE_ASSOC);
    {
        struct mapping *m;
        unsigned short num;

        ((char *)&num)[0] = pc[0];
        ((char *)&num)[1] = pc[1];
        pc += 2;
        m = load_mapping_from_aggregate(sp - num + 1, num);
        pop_n_elems((int)num);
        sp++;
        sp->type = T_MAPPING;
        sp->u.map = m;  /* Ref count already initialized */
        break;
    }
    CASE(F_AGGREGATE);
    {
	struct vector *v;
	unsigned short num;

	((char *)&num)[0] = pc[0];
	((char *)&num)[1] = pc[1];
	pc += 2;
	v = allocate_array((int)num);
	for (i=0; i < num; i++)
	    assign_svalue_no_free(&v->item[i], sp + i - num + 1);
	pop_n_elems((int)num);
	sp++;
	sp->type = T_POINTER;
	sp->u.vec = v;		/* Ref count already initialized */
	break;
    }
    CASE(F_TAIL);
	if (tail(sp->u.string))
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_CALL_FUNCTION_BY_ADDRESS);
    {
	unsigned short func_index;
	struct function *funp;

	((char *)&func_index)[0] = pc[0];
	((char *)&func_index)[1] = pc[1];
	pc += 2;
	func_index += function_index_offset;
	/*
	 * Find the function in the function table. As the function may have
	 * been redefined by inheritance, we must look in the last table,
	 * which is pointed to by current_object.
	 */
#ifdef DEBUG
	if (func_index >= current_object->prog->num_functions)
	    fatal("Illegal function index\n");
#endif

	/* NOT current_prog, which can be an inherited object. */
	funp = &current_object->prog->functions[func_index];

	if (funp->flags & NAME_UNDEFINED)
	    error("Undefined function: %s\n", funp->name);
	/* Save all important global stack machine registers */
	push_control_stack(funp);	/* return pc is adjusted later */

	/* This assigment must be done after push_control_stack() */
	current_prog = current_object->prog;
	/*
	 * If it is an inherited function, search for the real
	 * definition.
	 */
	csp->num_local_variables = EXTRACT_UCHAR(pc);
	pc++;
	funp = setup_new_frame(funp);
	csp->pc = pc;	/* The corrected return address */
	pc = current_prog->program + funp->offset;
	csp->extern_call = 0;
	break;
    }
    CASE(F_SAVE_OBJECT);
	save_object(current_object, sp->u.string);
	/* The argument is returned */
	break;
    CASE(F_FIND_OBJECT);
	ob = find_object2(sp->u.string);
	pop_stack();
	if (ob) 
	  {
            if (ob->flags & O_HIDDEN) 
	      { 
		if (current_object->flags & O_PRIVILEGED)
                  push_object(ob);
		else
                  push_number(0);
	      }
	    else
	      push_object (ob);
	  } else
	    push_number(0);
	break;
    CASE(F_FIND_PLAYER);
	ob = find_living_object(sp->u.string, 1);
	pop_stack();
	if (!ob)
	    push_number(0);
	else
	    push_object(ob);
	break;
    CASE(F_FIND_WIZARD);
    {
        struct vector *v;
        v = find_wizard(sp->u.string);
        if (!v)
           push_number(0);
        else {
           push_vector(v);
           v->ref--;
        }
        break;
    }
    CASE(F_WIZARDS);
    {
	struct vector *v;
        v = wizards();
	if (!v)
	  push_number (0);
	else {
	  push_vector(v);
	  v->ref--;
	}
        break;
    }
    CASE(F_WRITE_FILE);
	i = write_file((sp-1)->u.string, sp->u.string);
	pop_n_elems(2);
	push_number(i);
	break;
    CASE(F_READ_FILE);
    {
	char *str;
	struct svalue *arg = sp- num_arg + 1;
	int start = 0, len = 0;

	if (num_arg > 1)
	    start = arg[1].u.number;
	if (num_arg == 3) {
	    if (arg[2].type != T_NUMBER)
		bad_arg(2, instruction);
	    len = arg[2].u.number;
	}

	str = read_file(arg[0].u.string, start, len);
	pop_n_elems(num_arg);
	if (str == 0)
	    push_number(0);
	else {
	    push_string(str, STRING_MALLOC);
	    FREE(str);
	}
	break;
    }
	CASE(F_TYPE);
	{
		int k;

		k = sp->type;
		pop_stack();
		push_number(k);
	}
	CASE(F_MAP_DELETE); /* delete a mapping element */
	{
	mapping_delete((sp-1)->u.map,sp);
	pop_stack();
	break; /* all functions must leave exactly 1 element on stack */
	}
    CASE(F_READ_BYTES);
    {
	char *str;
	struct svalue *arg = sp- num_arg + 1;
	int start = 0, len = 0;

	if (num_arg > 1)
	    start = arg[1].u.number;
	if (num_arg == 3) {
	    if (arg[2].type != T_NUMBER)
		bad_arg(2, instruction);
	    len = arg[2].u.number;
	}

	str = read_bytes(arg[0].u.string, start, len);
	pop_n_elems(num_arg);
	if (str == 0)
	    push_number(0);
	else {
	    push_string(str, STRING_MALLOC);
	    FREE(str);
	}
	break;
    }
    CASE(F_WRITE_BYTES);
	i = write_bytes((sp-2)->u.string, (sp-1)->u.number, sp->u.string);
	pop_n_elems(3);
	push_number(i);
	break;
    CASE(F_FILE_SIZE);
	i = file_size(sp->u.string);
	pop_stack();
	push_number(i);
	break;
    CASE(F_NEXT_LIVING);
        ob = sp->u.ob->next_hashed_living;
        pop_stack();
        if (!ob)
           push_number(0);
        else
           push_object(ob);
        break;
    CASE(F_FIND_LIVING);
	ob = find_living_object(sp->u.string, 0);
	pop_stack();
	if (!ob)
	    push_number(0);
	else
	    push_object(ob);
	break;
    CASE(F_RECEIVE);
        if (current_object->interactive) {
           struct object *save_command_giver = command_giver;
           command_giver = current_object;
           add_message("%s", sp->u.string);
           command_giver = save_command_giver;
           assign_svalue(sp, &const1);
        } else
           assign_svalue(sp, &const0);
        break;
    CASE(F_TELL_OBJECT);
	tell_object((sp-1)->u.ob, sp->u.string);
	pop_stack();	/* Return first argument */
	break;
    CASE(F_RESTORE_OBJECT);
	i = restore_object(current_object, sp->u.string);
	pop_stack();
	push_number(i);
	break;
    CASE(F_THIS_PLAYER);
	pop_n_elems(num_arg);
	if (num_arg && current_interactive &&
	    !(current_interactive->flags & O_DESTRUCTED))
	    push_object(current_interactive);
	else if (command_giver && !(command_giver->flags & O_DESTRUCTED))
	    push_object(command_giver);
	else
	    push_number(0);
	break;
#ifdef F_FIRST_INVENTORY
    CASE(F_FIRST_INVENTORY);
	ob = first_inventory(sp);
	pop_stack();
	if (ob)
	    push_object(ob);
	else
	    push_number(0);
	break;
#endif /* F_FIRST_INVENTORY */
    CASE(F_LIVING);
	if (sp->u.ob->flags & O_ENABLE_COMMANDS)
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
#ifdef F_GETUID
    CASE(F_GETUID);
	/*
	 * Are there any reasons to support this one in -o mode ?
	 */
	ob = sp->u.ob;
#ifdef DEBUG
	if (ob->user == 0)
	    fatal("User is null pointer\n");
#endif
	{   char *tmp;
	    tmp = ob->user->name;
	    pop_stack();
	    push_string(tmp, STRING_CONSTANT);
	}
	break;
#endif /* F_GETUID */
#ifdef F_GETEUID
    CASE(F_GETEUID);
	/*
	 * Are there any reasons to support this one in -o mode ?
	 */
	ob = sp->u.ob;

	if (ob->eff_user) {
	    char *tmp;
	    tmp = ob->eff_user->name;
	    pop_stack();
	    push_string(tmp, STRING_CONSTANT);
	}
	else {
	    pop_stack();
	    push_number(0);
	}
	break;
#endif /* F_GETEUID */
#ifdef F_EXPORT_UID
    CASE(F_EXPORT_UID);
	if (current_object->eff_user == 0)
	    error("Illegal to export uid 0\n");
	ob = sp->u.ob;
	if (ob->eff_user)	/* Only allowed to export when null */
	    break;
	ob->user = current_object->eff_user;
	break;
#endif /* F_EXPORT_UID */
#ifdef F_SETEUID
    CASE(F_SETEUID);
    {
	struct svalue *ret;

	if (sp->type == T_NUMBER) {
	    if (sp->u.number != 0)
		bad_arg(1, F_SETEUID);
	    current_object->eff_user = 0;
	    pop_stack();
	    push_number(1);
	    break;
	}
	argp = sp;
	if (argp->type != T_STRING)
	    bad_arg(1, F_SETEUID);
	push_object(current_object);
	push_string(argp->u.string, STRING_CONSTANT);
	ret = apply_master_ob("valid_seteuid", 2);
	if (ret == 0 || ret->type != T_NUMBER || ret->u.number != 1) {
	    pop_stack();
	    push_number(0);
	    break;
	}
	current_object->eff_user = add_name(argp->u.string);
	pop_stack();
	push_number(1);
	break;
    }
#endif /* F_SETEUID */
#ifdef F_SETUID
    CASE(F_SETUID)
	setuid();
	push_number(0);
	break;
#endif /* F_SETUID */
#ifdef F_CREATOR
    CASE(F_CREATOR);
	ob = sp->u.ob;
	if (ob->user == 0) {
	    assign_svalue(sp, &const0);
	} else {
	    pop_stack();
	    push_string(ob->user->name, STRING_CONSTANT);
	}
	break;
#endif
	CASE(F_SHUTDOWN);
	{
		struct svalue *ret;

		push_string(make_shared_string(current_object->eff_user->name),
          STRING_SHARED);
		ret = apply_master_ob("valid_shutdown",1);
		if (!IS_ZERO(ret)) {
#if !defined(_AIX) && !defined(NeXT)
			startshutdowngame();
#else
			startshutdowngame(1);
#endif
		}
		else {
			add_message("Permission to shutdown denied by master object.\n");
		}
		push_number(0);
		break;
	}
    CASE(F_EXPLODE);
    {
	struct vector *v;
	v = explode_string((sp-1)->u.string, sp->u.string);
	pop_n_elems(2);
	if (v) {
	    push_vector(v);	/* This will make ref count == 2 */
	    v->ref--;
	} else {
	    push_number(0);
	}
	break;
    }
    CASE(F_FILTER_ARRAY);
    {
	struct vector *v;
	struct svalue *arg;

	arg = sp - num_arg + 1; ob = 0;

	if (arg[2].type == T_OBJECT)
	    ob = arg[2].u.ob;
	else if (arg[2].type == T_STRING) 
	    ob = find_object(arg[2].u.string);

	if (!ob)
	    error("Bad third argument to filter_array()\n");

	if (arg[0].type == T_POINTER) {
	    check_for_destr(arg[0].u.vec);
	    v = filter(arg[0].u.vec, arg[1].u.string, ob,
		       num_arg > 3 ? sp : (struct svalue *)0); 
	} else {
	    v = 0;
	}
	
	pop_n_elems(num_arg);
	if (v) {
	    push_vector(v); /* This will make ref count == 2 */
	    v->ref--;
	} else {
	    push_number(0);
	}
	break;
    }
    CASE(F_SET_BIT);
    {
	char *str;
	int len, old_len, ind;

	if (sp->u.number > MAX_BITS)
	    error("set_bit: too big bit number: %d\n", sp->u.number);
	len = strlen((sp-1)->u.string);
	old_len = len;
	ind = sp->u.number/6;
	if (ind >= len)
	    len = ind + 1;
	str = xalloc(len+1);
	str[len] = '\0';
	if (old_len)
	    memcpy(str, (sp-1)->u.string, old_len);
	if (len > old_len)
	    memset(str + old_len, ' ', len - old_len);
	if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	    error("Illegal bit pattern in set_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') | (1 << (sp->u.number % 6))) + ' ';
	pop_n_elems(2);
	sp++;
	sp->u.string = str;
	sp->subtype = STRING_MALLOC;
	sp->type = T_STRING;
	break;
    }
    CASE(F_CLEAR_BIT);
    {
	char *str;
	int len, ind;

	if (sp->u.number > MAX_BITS)
	    error("clear_bit: too big bit number: %d\n", sp->u.number);
	len = strlen((sp-1)->u.string);
	ind = sp->u.number/6;
	if (ind >= len) {
	    /* Return first argument unmodified ! */
	    pop_stack();
	    break;
	}
	str = xalloc(len+1);
	memcpy(str, (sp-1)->u.string, len+1);	/* Including null byte */
	if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	    error("Illegal bit pattern in clear_bit character %d\n", ind);
	str[ind] = ((str[ind] - ' ') & ~(1 << (sp->u.number % 6))) + ' ';
	pop_n_elems(2);
	sp++;
	sp->type = T_STRING;
	sp->subtype = STRING_MALLOC;
	sp->u.string = str;
	break;
    }
    CASE(F_TEST_BIT);
    {
	int len;

	len = strlen((sp-1)->u.string);
	if (sp->u.number/6 >= len) {
	    pop_n_elems(2);
	    push_number(0);
	    break;
	}
	if (((sp-1)->u.string[sp->u.number/6] - ' ') & (1 << (sp->u.number % 6))) {
	    pop_n_elems(2);
	    push_number(1);
	} else {
	    pop_n_elems(2);
	    push_number(0);
	}
	break;
    }
    CASE(F_QUERY_LOAD_AVERAGE);
	push_string(query_load_av(), STRING_MALLOC);
	break;
    CASE(F_CATCH);
	/*
	 * Catch/Throw - catch errors in system or other peoples routines.
	 */
    {
	extern jmp_buf error_recovery_context;
	extern int error_recovery_context_exists;
	unsigned short new_pc_offset;

	/*
	 * Compute address of next instruction after the CATCH statement.
	 */
	((char *)&new_pc_offset)[0] = pc[0];
	((char *)&new_pc_offset)[1] = pc[1];
	pc += 2;

	push_control_stack(0);
	csp->num_local_variables = 0;	/* No extra variables */
	csp->pc = current_prog->program + new_pc_offset;
	csp->num_local_variables = (csp-1)->num_local_variables; /* marion */
	/*
	 * Save some global variables that must be restored separately
	 * after a longjmp. The stack will have to be manually popped all
	 * the way.
	 */
	push_pop_error_context (1);
	
	/* signal catch OK - print no err msg */
   	error_recovery_context_exists = 2;
	if (setjmp(error_recovery_context)) {
	    /*
	     * They did a throw() or error. That means that the control
	     * stack must be restored manually here.
	     * Restore the value of expected_stack also. It is always 0
	     * for catch().
	     */
	    expected_stack = 0;
	    push_pop_error_context (-1);
	    pop_control_stack();
	    assign_svalue_no_free(++sp, &catch_value);
	}

	/* next error will return 1 by default */
	assign_svalue(&catch_value, &const1);
	break;
    }
    CASE(F_THROW);
	/* marion
	 * the return from catch is now done by a 0 throw
	 */
	assign_svalue(&catch_value, sp--);
	if (catch_value.type == T_NUMBER && catch_value.u.number == 0) {
	    /* We come here when no longjmp() was executed. */
	    pop_control_stack();
	    push_pop_error_context (0);
	    push_number(0);
	} else throw_error(); /* do the longjump, with extra checks... */
	break;
    CASE(F_NOTIFY_FAIL);
	set_notify_fail_message(sp->u.string);
	/* Return the argument */
	break;
    CASE(F_QUERY_IDLE);
	i = query_idle(sp->u.ob);
	pop_stack();
	push_number(i);
	break;
    CASE(F_IN_EDIT);
        i = sp->u.ob->interactive && sp->u.ob->interactive->ed_buffer;
		pop_stack();
		if (i)
           push_number(1);
        else
           push_number(0);
        break;
    CASE(F_IN_INPUT);
        i = sp->u.ob->interactive && sp->u.ob->interactive->input_to;
		pop_stack();
		if (i)
           push_number(1);
        else
           push_number(0);
        break;
    CASE(F_ENABLE_WIZARD);
        if (current_object->interactive)
	  {
	    current_object->flags |= O_IS_WIZARD;
	    push_number (1);
	  }
	else
	  push_number(0);
        break;
    CASE(F_ENABLE_PRIVILEGES);
        if (current_object->flags & O_PRIVILEGED)
           sp->u.ob->flags |= O_PRIVILEGED;
        break;
    CASE(F_SET_HIDE);
        if (!(current_object->flags & O_PRIVILEGED))
           break;
        if (sp->u.number)
           current_object->flags |= O_HIDDEN;
        else
           current_object->flags &= ~O_HIDDEN;
        break;
    CASE(F_PRIVP);
        i = (int)sp->u.ob->flags & O_PRIVILEGED;
        pop_stack();
        push_number(i != 0);
        break;
    CASE(F_WIZARDP);
        i = (int)sp->u.ob->flags & O_IS_WIZARD;
        pop_stack();
        push_number(i != 0);
        break;
    CASE(F_USERP);
        i = (int)sp->u.ob->flags & O_ONCE_INTERACTIVE;
        pop_stack();
        push_number(i != 0);
        break;
    CASE(F_INTERACTIVE);
        i = (int)sp->u.ob->interactive;
	pop_stack();
	push_number(i);
	break;
    CASE(F_IMPLODE);
    {
	char *str;
	check_for_destr((sp-1)->u.vec);
	str = implode_string((sp-1)->u.vec, sp->u.string);
	pop_n_elems(2);
	if (str) {
	    sp++;
	    sp->type = T_STRING;
	    sp->subtype = STRING_MALLOC;
	    sp->u.string = str;
	} else {
	    push_number(0);
	}
	break;
    }
    CASE(F_QUERY_SNOOP);
    {

	if (command_giver == 0 || sp->u.ob->interactive == 0 || (command_giver->flags & O_DESTRUCTED)) {
	    assign_svalue(sp, &const0);
	    break;
	}
	assert_master_ob_loaded("query_snoop()");
	if (current_object == master_ob)
	    ob = query_snoop(sp->u.ob);
	else
	    ob = 0;
	pop_stack();
	if (ob)
	    push_object(ob);
	else
	    push_number(0);
	break;
    }
    CASE(F_QUERY_IP_NUMBER);
    CASE(F_QUERY_IP_NAME);
    {
	extern char *query_ip_number PROT((struct object *));
 	extern char *query_ip_name PROT((struct object *));
	char *tmp;

	if (num_arg == 1 && sp->type != T_OBJECT)
	    error("Bad optional argument to query_ip_number()\n");
	if (instruction == F_QUERY_IP_NAME)
	    tmp = query_ip_name(num_arg ? sp->u.ob : 0);
	else
	    tmp = query_ip_number(num_arg ? sp->u.ob : 0);
	if (num_arg)
	    pop_stack();
	if (tmp == 0)
	    push_number(0);
	else
	    push_string(tmp, STRING_MALLOC);
	break;
    }
    CASE(F_QUERY_HOST_NAME);
    {
	extern char *query_host_name();
	char *tmp;

	tmp = query_host_name();
	if (tmp)
	    push_string(tmp, STRING_CONSTANT);
	else
	    push_number(0);
	break;
    }
#ifdef F_NEXT_INVENTORY
    CASE(F_NEXT_INVENTORY);
	ob = sp->u.ob;
	pop_stack();
	if (ob->next_inv)
	    push_object(ob->next_inv);
	else
	    push_number(0);
	break;
#endif /* F_NEXT_INVENTORY */
    CASE(F_ALL_INVENTORY);
    {
	struct vector *vec;
	vec = all_inventory(sp->u.ob);
	pop_stack();
	if (vec == 0) {
	    push_number(0);
	} else {
	    push_vector(vec); /* This will make ref count == 2 */
	    vec->ref--;
	}
	break;
    }
    CASE(F_DEEP_INVENTORY);
    {
	struct vector *vec;

	vec = deep_inventory(sp->u.ob, 0);
	free_svalue(sp);
	sp->type = T_POINTER;
	sp->u.vec = vec;
	break;
    }
    CASE(F_ENVIRONMENT);
	if (num_arg) {
	    ob = environment(sp);
	    pop_stack();
	} else if (!(current_object->flags & O_DESTRUCTED)) {
	    ob = current_object->super;
	} else
	    ob = 0;
	if (ob)
	    push_object(ob);
	else
	    push_number(0);
	break;
    CASE(F_MASTER);
        assert_master_ob_loaded("master()");
        push_object(master_ob);
        break;
    CASE(F_THIS_OBJECT);
	   if (current_object->flags & O_DESTRUCTED) /* Fixed from 3.1.1 */
		   push_number(0);
	   else
		   push_object(current_object);
	break;
    CASE(F_PREVIOUS_OBJECT);
	if (previous_ob == 0 || (previous_ob->flags & O_DESTRUCTED))
	    push_number(0);
	else
	    push_object(previous_ob);
	break;
    CASE(F_SWAP);
	(void)swap(sp->u.ob);
	break;
    CASE(F_TRACE);
	{
	    int ot = -1;
	    if (command_giver && command_giver->interactive &&
		command_giver->flags & O_IS_WIZARD) {
	      ot = command_giver->interactive->trace_level;
	      command_giver->interactive->trace_level = sp->u.number;
	    }
	    pop_stack();
	    push_number(ot);
	}
	break;
    CASE(F_TRACEPREFIX);
	{
	    char *old = 0;

	    if (command_giver && command_giver->interactive &&
		command_giver->flags & O_IS_WIZARD)
	      {
		old = command_giver->interactive->trace_prefix;
		if (sp->type == T_STRING) {
		  command_giver->interactive->trace_prefix = 
		    make_shared_string(sp->u.string);
		} else
		  command_giver->interactive->trace_prefix = 0;
	      }
	    pop_stack();
	    if (old) {
		push_string(old, STRING_SHARED);   /* Will incr ref count */
		free_string(old);
	    } else {
		push_number(0);
	    }
	}
	break;
    CASE(F_TIME);
	push_number(current_time);
	break;
    CASE(F_WIZLIST);
	if (num_arg) {
	    wizlist(sp->u.string);
	} else {
	    wizlist(0);
	    push_number(0);
	}
	break;
#ifdef F_TRANSFER
    CASE(F_TRANSFER);
    {
	struct object *dest;

	if (sp->type == T_STRING) {
	    dest = find_object(sp->u.string);
	    if (dest == 0)
		error("Object not found.\n");
	} else {
	    dest = sp->u.ob;
	}
	i = transfer_object((sp-1)->u.ob, dest);
	pop_n_elems(2);
	push_number(i);
	break;
    }
#endif
#ifdef F_ADD_WORTH
    CASE(F_ADD_WORTH);
    {
       struct wiz_list *user;
        assert_master_ob_loaded("add_worth");
        if (current_object->user != back_bone_uid &&
            current_object->user != master_ob->user)
	    error("Illegal call of add_worth.\n");
	if (num_arg == 2) {
            if (sp->type == T_STRING)
               user = add_name(sp->u.string);
            else
               user = sp->u.ob->user;
            if (user)
                user->total_worth += (sp-1)->u.number;
	    pop_stack();
	} else {
	    if (previous_ob == 0)
		break;
	    if (previous_ob->user)
		previous_ob->user->total_worth += sp->u.number;
	}
	break;
    }
#endif /* F_ADD_WORTH */
	CASE(F_ADD);
	{
		struct svalue ret;
		switch((sp-1)->type)
		{
		case T_STRING:
			{
				switch(sp->type)
				{
				case T_STRING:
					{
						char *res;
						res=xalloc(strlen((sp-1)->u.string)+strlen(sp->u.string)+1);
						(void)strcpy(res, (sp-1)->u.string);
						(void)strcat(res, sp->u.string);
						ret.type = T_STRING;
						ret.subtype = STRING_SHARED;
						ret.u.string = make_shared_string(res);
						free(res);
						break;
					} 
				case T_NUMBER:
					{
						char buff[20];
						char *res;
						sprintf(buff, "%d", sp->u.number);
						res = xalloc(strlen((sp-1)->u.string) + strlen(buff) + 1);
						strcpy(res, (sp-1)->u.string);
						strcat(res, buff);
						ret.type = T_STRING;
						ret.subtype = STRING_SHARED;
						ret.u.string = make_shared_string(res);
						free(res);
						break;
					}
				case T_POINTER:
					ret.type = T_POINTER;
					ret.u.vec = prepend_vector(sp->u.vec, sp-1);
					break;
				default: 
					error("Bad type argument to +. %d %d\n", 
					(sp-1)->type, sp->type);
				}
				break;
			} 
		case T_NUMBER:
			{
				ret.type = sp->type;
				switch(sp->type)
				{
				case T_NUMBER:
					ret.u.number = sp->u.number + (sp-1)->u.number;
					break;
				case T_STRING:
					{
						char buff[20], *res;
						sprintf(buff, "%d", (sp-1)->u.number);
						res = xalloc(strlen(sp->u.string) + strlen(buff) + 1);
						strcpy(res, buff);
						strcat(res, sp->u.string);
						ret.subtype = STRING_SHARED;
						ret.u.string = make_shared_string(res);
						free(res);
						break;
					}
				case T_POINTER:
					ret.u.vec = prepend_vector(sp->u.vec, sp-1);
					break;
				default: 
					error("Bad type argument to +. %d %d\n", 
					(sp-1)->type, sp->type);
				}
				break;
			}
		case T_POINTER:
			{
				ret.type = T_POINTER;
				switch(sp->type)
				{
				case T_POINTER:
					ret.u.vec = add_array((sp-1)->u.vec, sp->u.vec);
					break;
				case T_NUMBER:
				case T_STRING:
					ret.u.vec = append_vector((sp-1)->u.vec, sp);
					break;
				default: 
					error("Bad type argument to +. %d %d\n", 
					(sp-1)->type, sp->type);
				}
				break;
			} 
		case T_MAPPING:
			if (sp->type == T_MAPPING) {
				ret.type = T_MAPPING;
				ret.u.map = add_mapping((sp-1)->u.map, sp->u.map);
			}
			else {
				error("Bad type argument to +. %d %d\n", (sp-1)->type,
					sp->type);
			}
			break;
		default: 
			error("Bad type argument to +. %d %d\n", 
			(sp-1)->type, sp->type);
		}
		pop_n_elems(2);
		push_svalue(&ret);
		free_svalue(&ret);
		break;
	}      
    CASE(F_SUBTRACT);
	if ((sp-1)->type == T_POINTER && sp->type == T_POINTER) {
	    extern struct vector *subtract_array
	      PROT((struct vector *,struct vector*));
	    struct vector *v;

	    v = sp->u.vec;
	    if (v->ref > 1) {
		v = slice_array(v, 0, v->size-1 );
		v->ref--;
            }
	    sp--;
	    /* subtract_array already takes care of destructed objects */
	    v = subtract_array(sp->u.vec, v);
	    free_vector(sp->u.vec);
	    sp->u.vec = v;
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, F_SUBTRACT);
	if (sp->type != T_NUMBER)
	    bad_arg(2, F_SUBTRACT);
	i = (sp-1)->u.number - sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_AND);
	if (sp->type == T_POINTER && (sp-1)->type == T_POINTER) {
	    extern struct vector *intersect_array
	      PROT((struct vector *, struct vector *));
	    (sp-1)->u.vec = intersect_array(sp->u.vec, (sp-1)->u.vec);
	    sp--;
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, F_AND);
	if (sp->type != T_NUMBER)
	    bad_arg(2, F_AND);
	i = (sp-1)->u.number & sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_OR);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, F_OR);
	if (sp->type != T_NUMBER)
	    bad_arg(2, F_OR);
	i = (sp-1)->u.number | sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_XOR);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number ^ sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_LSH);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number << sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_RSH);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number >> sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_MULTIPLY);
	if ((sp-1)->type != sp->type)
	    bad_arg(1, instruction);
	if (sp->type == T_NUMBER) {
	    i = (sp-1)->u.number * sp->u.number;
	    sp--;
	    sp->u.number = i;
            break;
        } else if (sp->type == T_MAPPING) {
            struct mapping *m;
            m = compose_mapping((sp-1)->u.map, sp->u.map);
            pop_n_elems(2);
            push_mapping(m);
            break;
        }
        bad_arg(2, instruction);
        break;
    CASE(F_DIVIDE);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	if (sp->u.number == 0)
	    error("Division by zero\n");
	i = (sp-1)->u.number / sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_MOD);
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	if (sp->u.number == 0)
	    error("Modulus by zero.\n");
	i = (sp-1)->u.number % sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_GT);
	if ((sp-1)->type == T_STRING && sp->type == T_STRING) {
	    i = strcmp((sp-1)->u.string, sp->u.string) > 0;
	    pop_n_elems(2);
	    push_number(i);
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number > sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_GE);
	if ((sp-1)->type == T_STRING && sp->type == T_STRING) {
	    i = strcmp((sp-1)->u.string, sp->u.string) >= 0;
	    pop_n_elems(2);
	    push_number(i);
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number >= sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_LT);
	if ((sp-1)->type == T_STRING && sp->type == T_STRING) {
	    i = strcmp((sp-1)->u.string, sp->u.string) < 0;
	    pop_n_elems(2);
	    push_number(i);
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number < sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_LE);
	if ((sp-1)->type == T_STRING && sp->type == T_STRING) {
	    i = strcmp((sp-1)->u.string, sp->u.string) <= 0;
	    pop_n_elems(2);
	    push_number(i);
	    break;
	}
	if ((sp-1)->type != T_NUMBER)
	    bad_arg(1, instruction);
	if (sp->type != T_NUMBER)
	    bad_arg(2, instruction);
	i = (sp-1)->u.number <= sp->u.number;
	sp--;
	sp->u.number = i;
	break;
    CASE(F_EQ);
	if ((sp-1)->type != sp->type) {
	    pop_stack();
	    assign_svalue(sp, &const0);
	    break;
	}
	switch(sp->type) {
	case T_NUMBER:
	    i = (sp-1)->u.number == sp->u.number;
	    break;
	case T_POINTER:
	    i = (sp-1)->u.vec == sp->u.vec;
	    break;
	case T_STRING:
	    i = strcmp((sp-1)->u.string, sp->u.string) == 0;
	    break;
	case T_OBJECT:
	    i = (sp-1)->u.ob == sp->u.ob;
	    break;
	default:
	    i = 0;
	    break;
	}
	pop_n_elems(2);
	push_number(i);
	break;
    CASE(F_NE);
	if ((sp-1)->type != sp->type) {
	    pop_stack();
	    assign_svalue(sp, &const1);
	    break;
	}
	switch(sp->type) {
	case T_NUMBER:
	    i = (sp-1)->u.number != sp->u.number;
	    break;
	case T_STRING:
	    i = strcmp((sp-1)->u.string, sp->u.string);
	    break;
	case T_POINTER:
	    i = (sp-1)->u.vec != sp->u.vec;
	    break;
	case T_OBJECT:
	    i = (sp-1)->u.ob != sp->u.ob;
	    break;
	default:
	    fatal("Illegal type to !=\n");
	}
	pop_n_elems(2);
	push_number(i);
	break;
#ifdef F_LOG_FILE
    CASE(F_LOG_FILE);
	log_file((sp-1)->u.string, sp->u.string);
	pop_stack();
	break;	/* Return first argument */
#endif /* F_LOG_FILE */
    CASE(F_NOT);
	if (sp->type == T_NUMBER && sp->u.number == 0)
	    sp->u.number = 1;
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_COMPL);
	if (sp->type != T_NUMBER)
	    error("Bad argument to ~\n");
	sp->u.number = ~ sp->u.number;
	break;
    CASE(F_NEGATE);
	if (sp->type != T_NUMBER)
	    error("Bad argument to unary minus\n");
	sp->u.number = - sp->u.number;
	break;
    CASE(F_INC);
	if (sp->type != T_LVALUE)
	    error("Bad argument to ++\n");
	if (sp->u.lvalue->type != T_NUMBER)
	    error("++ of non-numeric argument\n");
	sp->u.lvalue->u.number++;
	assign_svalue(sp, sp->u.lvalue);
	break;
    CASE(F_DEC);
	if (sp->type != T_LVALUE)
	    error("Bad argument to --\n");
	if (sp->u.lvalue->type != T_NUMBER)
	    error("-- of non-numeric argument\n");
	sp->u.lvalue->u.number--;
	assign_svalue(sp, sp->u.lvalue);
	break;
    CASE(F_POST_INC);
	if (sp->type != T_LVALUE)
	    error("Bad argument to ++\n");
	if (sp->u.lvalue->type != T_NUMBER)
	    error("++ of non-numeric argument\n");
	sp->u.lvalue->u.number++;
	assign_svalue(sp, sp->u.lvalue);
	sp->u.number--;
	break;
    CASE(F_POST_DEC);
	if (sp->type != T_LVALUE)
	    error("Bad argument to --\n");
	if (sp->u.lvalue->type != T_NUMBER)
	    error("-- of non-numeric argument\n");
	sp->u.lvalue->u.number--;
	assign_svalue(sp, sp->u.lvalue);
	sp->u.number++;
	break;
    CASE(F_CALL_OTHER);
    {
	struct svalue *arg, tmp;

        if (current_object->flags & O_DESTRUCTED) {
            /* No external calls allowed */
            pop_n_elems(num_arg);
            push_undefined();
            break;
        }
        
	arg = sp - num_arg + 1;
       
        if (arg[1].u.string[0] == ':')
           error("Illegal function name in call_other: %s\n",
                 arg[1].u.string);     
	if (arg[0].type == T_OBJECT)
	    ob = arg[0].u.ob;
	else if (arg[0].type == T_POINTER) {
            extern int call_all_other PROT((struct vector *, char *, int));

            (void)call_all_other(arg[0].u.vec, arg[1].u.string, num_arg-2);
            pop_n_elems(2);  /* get rid of args 0 and 1 */
            push_number(0);
            break;
        } else {
	    ob = find_object(arg[0].u.string);
	    if (ob == 0)
		error("call_other() failed\n");
	}
	/*
	 * Send the remaining arguments to the function.
	 */
	if (TRACEP(TRACE_CALL_OTHER)) {
	    do_trace("Call other ", arg[1].u.string, "\n");
	}
	if (apply_low(arg[1].u.string, ob, num_arg-2) == 0) {
	    /* Function not found */
	    pop_n_elems(2);
	    push_undefined();
	    break;
	}
	/*
	 * The result of the function call is on the stack. But, so
	 * is the function name and object that was called.
	 * These have to be removed.
	 */
	tmp = *sp--;	/* Copy the function call result */
	pop_n_elems(2);	/* Remove old arguments to call_other */
	*++sp = tmp;	/* Re-insert function result */
	break;
    }
	CASE(F_UNDEFINEDP);
	if (IS_UNDEFINED(sp))
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_INTP);
	if (sp->type == T_NUMBER)
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_STRINGP);
	if (sp->type == T_STRING)
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_OBJECTP);
	if (sp->type == T_OBJECT)
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_POINTERP);
	if (sp->type == T_POINTER)
	    assign_svalue(sp, &const1);
	else
	    assign_svalue(sp, &const0);
	break;
    CASE(F_MAPP);
        if (sp->type == T_MAPPING)
            assign_svalue(sp, &const1);
        else
            assign_svalue(sp, &const0);
        break;
    CASE(F_EXTRACT);
    {
	int len, from, to;
	struct svalue *arg;
	char *res;

	arg = sp - num_arg + 1;
	len = strlen(arg[0].u.string);
	if (num_arg == 1)
	    break;		/* Simply return argument */
	from = arg[1].u.number;
	if (from < 0)
	    from = len + from;
	if (from >= len) {
	    pop_n_elems(num_arg);
	    push_string("", STRING_CONSTANT);
	    break;
	}
	if (num_arg == 2) {
	    res = string_copy(arg->u.string + from);
	    pop_n_elems(2);
	    push_malloced_string(res);
	    break;
	}
	if (arg[2].type != T_NUMBER)
	    error("Bad third argument to extract()\n");
	to = arg[2].u.number;
	if (to < 0)
	    to = len + to;
	if (to < from) {
	    pop_n_elems(3);
	    push_string("", STRING_CONSTANT);
	    break;
	}
	if (to >= len)
	    to = len-1;
	if (to == len-1) {
	    res = string_copy(arg->u.string + from);
	    pop_n_elems(3);
	    push_malloced_string(res);
	    break;
	}
	res = xalloc(to - from + 2);
	strncpy(res, arg[0].u.string + from, to - from + 1);
	res[to - from + 1] = '\0';
	pop_n_elems(3);
	push_malloced_string(res);
	break;
    }
    CASE(F_RANGE);
    {
	if (sp[-1].type != T_NUMBER)
	    error("Bad type of start interval to [ .. ] range.\n");
	if (sp[0].type != T_NUMBER)
	    error("Bad type of end interval to [ .. ] range.\n");
	if (sp[-2].type == T_POINTER) {
	    struct vector *v;

	    v = slice_array(sp[-2].u.vec, sp[-1].u.number, sp[0].u.number);
	    pop_n_elems(3);
	    if (v) {
		push_vector(v);
		v->ref--;	/* Will make ref count == 1 */
	    } else {
		push_number(0);
	    }
	} else if (sp[-2].type == T_STRING) {
	    int len, from, to;
	    char *res;
	    
	    len = strlen(sp[-2].u.string);
	    from = sp[-1].u.number;
	    if (from < 0)
		from = len + from;
	    if (from >= len) {
		pop_n_elems(3);
		push_string("", STRING_CONSTANT);
		break;
	    }
	    to = sp[0].u.number;
	    if (to < 0)
		to = len + to;
	    if (to < from) {
		pop_n_elems(3);
		push_string("", STRING_CONSTANT);
		break;
	    }
	    if (to >= len)
		to = len-1;
	    if (to == len-1) {
		res = string_copy(sp[-2].u.string + from);
		pop_n_elems(3);
		push_malloced_string(res);
		break;
	    }
	    res = xalloc(to - from + 2);
	    strncpy(res, sp[-2].u.string + from, to - from + 1);
	    res[to - from + 1] = '\0';
	    pop_n_elems(3);
	    push_malloced_string(res);
	} else {
	    error("Bad argument to [ .. ] range operand.\n");
	}
	break;
    }
    CASE(F_QUERY_VERB);
	if (last_verb == 0) {
	    push_number(0);
	    break;
	}
	push_string(last_verb, STRING_CONSTANT);
	break;
    CASE(F_EXEC);

	i = replace_interactive((sp-1)->u.ob, sp->u.ob, current_prog->name);
	pop_stack();
	pop_stack();
	push_number(i);
	break;

    CASE(F_FILE_NAME);
    {
	char *name,*res;

	/* This function now returns a leading '/', except when -o flag */
	name = sp->u.ob->name;
	res = add_slash(name);
	pop_stack();
	push_malloced_string(res);
	break;
    }
    CASE(F_USERS);
	push_vector(users());	/* users() has already set ref count to 1 */
	sp->u.vec->ref--;
	break;
    CASE(F_CALL_OUT);
	{
	    struct svalue *arg = sp - num_arg + 1;

	    if (!(current_object->flags & O_DESTRUCTED))
		new_call_out(current_object, arg[0].u.string, arg[1].u.number,
			     num_arg == 3 ? sp : 0);
	    pop_n_elems(num_arg);
	    push_number(0);
	}
	break;
    CASE(F_CALL_OUT_INFO);
	push_vector(get_all_call_outs());
	sp->u.vec->ref--;	/* Was set to 1 at allocation */
	break;
    CASE(F_REMOVE_CALL_OUT);
	i = remove_call_out(current_object, sp->u.string);
	pop_stack();
	push_number(i);
	break;
    CASE(F_FIND_CALL_OUT);
        i = find_call_out(current_object, sp->u.string);
  	pop_stack();
	push_number(i);
        break;
#ifdef F_DEEP_INHERIT_LIST
    CASE(F_DEEP_INHERIT_LIST)
    {
	struct vector *vec;
	extern struct vector *deep_inherit_list PROT((struct object *));

	vec = deep_inherit_list(sp->u.ob);
	pop_stack();
	push_vector(vec);
	break;
    }
#endif /* F_DEEP_INHERIT_LIST */
#ifdef F_INHERIT_LIST
    CASE(F_INHERIT_LIST)
	{
		struct vector *vec;
		extern struct vector *inherit_list PROT((struct object *));

		vec = inherit_list(sp->u.ob);
		pop_stack();
		push_vector(vec);
		break;
    }
#endif F_INHERIT_LIST
    CASE(F_WRITE);
	do_write(sp);
	break;
    CASE (F_MEMBER_ARRAY);
    {
	struct vector *v;

	v = sp->u.vec;
	check_for_destr(v);
	for (i=0; i < v->size; i++) {
	    if (v->item[i].type != (sp-1)->type)
		continue;
	    switch((sp-1)->type) {
	    case T_STRING:
		if (strcmp((sp-1)->u.string, v->item[i].u.string) == 0)
		    break;
		continue;
	    case T_POINTER:
		if ((sp-1)->u.vec == v->item[i].u.vec)
		    break;
		continue;
	    case T_OBJECT:
		if ((sp-1)->u.ob == v->item[i].u.ob)
		    break;
		continue;
	    case T_NUMBER:
		if ((sp-1)->u.number == v->item[i].u.number)
		    break;
		continue;
	    default:
		fatal("Bad type to member_array(): %d\n", (sp-1)->type);
	    }
	    break;
	}
	if (i == v->size)
	    i = -1;		/* Return -1 for failure */
	pop_n_elems(2);
	push_number(i);
	break;
    }
    CASE(F_MOVE_OBJECT);
    {
	struct object *o1, *o2;

	if (sp->type == T_OBJECT)
	    o2 = sp->u.ob;
	else {
	    o2 = find_object(sp->u.string);
	    if (o2 == 0)
		error("move_object failed\n");
	}
        if (num_arg == 1)
           o1 = current_object;
        else {
           o1 = (sp-1)->u.ob;
           pop_stack();
        }
	move_object(o1, o2);
	break;
    }
    CASE(F_FUNCTION_EXISTS);
    {
	char *str, *res;

	str = function_exists((sp-1)->u.string, sp->u.ob);
	pop_n_elems(2);
	if (str) {
	    res = add_slash(str);
	    if (str = strrchr (res, '.'))
		*str = 0;
	    push_malloced_string(res);
	} else {
	    push_number(0);
	}
	break;
    }
    CASE(F_SNOOP);
	/* This one takes a variable number of arguments. It returns
	 * 0 or an object.
	 */
	if (!command_giver) {
	    pop_n_elems(num_arg);
	    push_number(0);
	} else {
	    ob = 0; /* Do not remove this, it is not 0 by default */
	    switch (num_arg) {
	    case 1:
		if (new_set_snoop(sp->u.ob, 0))
		    ob = sp->u.ob;
		break;
	    case 2:
		if (new_set_snoop((sp-1)->u.ob, sp->u.ob))
		    ob = sp->u.ob;
		break;
	    default:
		ob = 0;
		break;
	    }
	    pop_n_elems(num_arg);
	    if (ob)
		push_object(ob);
	    else
		push_number(0);
	}
	break;
    CASE(F_COMMANDS);
    {
        struct vector *v;
        v = commands(current_object);
        push_vector(v);
        break;
    }
    CASE(F_REMOVE_ACTION);
    {
        remove_action((sp-1)->u.string, sp->u.string);
        pop_stack();
        break;
    }
    CASE(F_ADD_ACTION);
    {
	struct svalue *arg = sp - num_arg + 1;
	if (num_arg == 3) {
	    if (arg[2].type != T_NUMBER)
		bad_arg(3, instruction);
	}
	add_action(arg[0].u.string,
		   num_arg > 1 ? arg[1].u.string : 0,
		   num_arg > 2 ? arg[2].u.number : 0);
	pop_n_elems(num_arg-1);
	break;
    }
#ifdef F_ADD_VERB
    CASE(F_ADD_VERB);
	add_verb(sp->u.string,0);
	break;
#endif /* F_ADD_VERB */
#ifdef F_ADD_XVERB
    CASE(F_ADD_XVERB);
   	add_verb(sp->u.string,1);
	break;
#endif /* F_ADD_XVERB */
    CASE(F_ALLOCATE);
    {
	struct vector *v;

	v = allocate_array(sp->u.number);	/* Will have ref count == 1 */
	pop_stack();
	push_vector(v);
	v->ref--;
	break;
    }
	CASE(F_ALLOCATE_MAPPING); /* Truilkan@TMI - 92/02/10 */
	{
		struct mapping *m;

		m = allocate_mapping(sp->u.number);
		pop_stack();
		push_mapping(m);
		m->ref--;
		break;
	}
    CASE(F_ED);
	if (num_arg == 0) {
	    if (command_giver == 0 || command_giver->interactive == 0) {
			push_number(0);
			break;
	    }
	    ed_start(0, 0, 0);
	    push_number(1);
	    break;
	} else if (num_arg == 1) {
	    ed_start(sp->u.string, 0, 0);
	} else {
	    if (sp->type == T_STRING)
	        ed_start((sp-1)->u.string, sp->u.string, current_object);
	    else
		ed_start((sp-1)->u.string, 0 , 0);
	    pop_stack();
	}
	break;
    CASE(F_CRYPT);
    {
	char salt[2];
	char *res;
	char *choise =
	    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

	if (sp->type == T_STRING && strlen(sp->u.string) >= 2) {
	    salt[0] = sp->u.string[0];
	    salt[1] = sp->u.string[1];
	} else {
	    salt[0] = choise[random_number(strlen(choise))];
	    salt[1] = choise[random_number(strlen(choise))];
	}
#ifdef sun
	res = string_copy(_crypt((sp-1)->u.string, salt));
#else
	res = string_copy(crypt((sp-1)->u.string, salt));
#endif
	pop_n_elems(2);
	push_malloced_string(res);
	break;
    }
    CASE(F_DESTRUCT);

	if (~(sp->u.ob->flags & O_PRIVILEGED) ||
			(current_object->flags & O_PRIVILEGED)) {
		destruct_object(sp);
	}
	break;
    CASE(F_RANDOM);
	if (sp->u.number <= 0) {
	    sp->u.number = 0;
	    break;
	}
	sp->u.number = random_number(sp->u.number);
	break;
#ifdef F_SAY
    CASE(F_SAY);
    {
	static struct {
	    struct vector v;
	    struct svalue second_item[1];
	} vtmp = { { 2, 1,
#ifdef DEBUG
		     1,
#endif
		     (struct wiz_list *)NULL,
		 { { T_NUMBER } } }, { { T_OBJECT } }
	       };
	
	if (num_arg == 1) {
	    vtmp.v.item[0].type = T_NUMBER; /* this marks the place for the
					       command_giver
					       */
	    vtmp.v.item[1].type = T_NUMBER; /* will not match any object... */
	    say(sp, &vtmp.v);
	} else {
	    if ( sp->type == T_POINTER ) {
		if (sp->u.vec->ref > 1) {
		    struct vector *vtmpp =
			slice_array(sp->u.vec, 0, sp->u.vec->size-1);
		    say(sp-1, vtmpp);
		    free_vector(vtmpp);
		} else
	            say(sp-1, sp->u.vec);
	    } else {
	        vtmp.v.item[0].type = T_NUMBER;
		vtmp.v.item[1].type = T_OBJECT;
	        vtmp.v.item[1].u.ob = sp->u.ob;
		add_ref(sp->u.ob, "say");
	        say(sp-1, &vtmp.v);
	    }
	    pop_stack();
	}
	break;
    }
#endif /* F_SAY */
#ifdef F_TELL_ROOM
    CASE(F_TELL_ROOM);
    {
	extern struct vector null_vector;
	struct svalue *arg = sp- num_arg + 1;
	struct vector *avoid;

	if (arg[0].type == T_OBJECT)
	    ob = arg[0].u.ob;
	else {
	    ob = find_object(arg[0].u.string);
	    if (ob == 0)
		error("Object not found.\n");
	}
	if (num_arg == 2) {
	    avoid = &null_vector;
	    avoid->ref++;
	} else {
	    extern struct vector *order_alist PROT((struct vector *));
	    struct vector *vtmpp;
	    static struct vector vtmp = { 1, 1,
#ifdef DEBUG
		1,
#endif
		(struct wiz_list *)NULL,
		{ { T_POINTER } }
		};

	    if (arg[2].type != T_POINTER)
		bad_arg(3, instruction);
	    vtmp.item[0].u.vec = arg[2].u.vec;
	    if (vtmp.item[0].u.vec->ref > 1) {
		vtmp.item[0].u.vec->ref--;
		vtmp.item[0].u.vec = slice_array(
		  vtmp.item[0].u.vec, 0, vtmp.item[0].u.vec->size-1);
	    }
	    sp--;
	    vtmpp = order_alist(&vtmp);
	    avoid = vtmpp->item[0].u.vec;
	    vtmpp->item[0].u.vec = vtmp.item[0].u.vec;
	    free_vector(vtmpp);
	}
	tell_room(ob, sp, avoid);
	free_vector(avoid);
	pop_stack();
	break;
    }
#endif /* F_TELL_ROOM */
#ifdef F_SHOUT
    CASE(F_SHOUT);
	shout_string(sp->u.string);
	break;
#endif /* F_SHOUT */
    CASE(F_WHILE);
	fatal("F_WHILE should not appear.\n");
    CASE(F_DO);
	fatal("F_DO should not appear.\n");
    CASE(F_FOR);
	fatal("F_FOR should not appear.\n");
    CASE(F_SWITCH);
    {
	extern char* findstring PROT((char*));
	unsigned short offset,break_adr;
	int d,s,r;
	char *l,*end_tab;
	static short off_tab[] = { 0*6,1*6,3*6,7*6,15*6,31*6,63*6,127*6,255*6,
					511*6,1023*6,2047*6,4095*6,8191*6 };

	((char *)&offset)[0] = pc[1];
	((char *)&offset)[1] = pc[2];
	((char *)&break_adr)[0] = pc[3];
	((char *)&break_adr)[1] = pc[4];
	*--break_sp = break_adr;
	if ( ( i = EXTRACT_UCHAR(pc) >> 4 ) != 0xf ) {
	    if ( sp->type == T_NUMBER && !sp->u.number ) {
		/* special case: uninitalized string */
		s = (int)ZERO_AS_STR_CASE_LABEL;
	    } else if ( sp->type == T_STRING ) {
	        switch(sp->subtype) {
	        case STRING_SHARED:
	            s = (int)sp->u.string;
	            break;
		default:
		    s = (int)findstring(sp->u.string);
	            break;
	        }
	    } else {
		bad_arg(1, F_SWITCH);
	    }
	} else {
	    if (sp->type != T_NUMBER) bad_arg(1, F_SWITCH);
	    s = sp->u.number;
	    i = (int)pc[0] &0xf ;
	}
	pop_stack();
	end_tab = current_prog->program + break_adr;
	if ( i >= 14 )
	    if ( i == 14 ) {
		/* fastest switch format : lookup table */
    		l = current_prog->program + offset;
                ((char *)&d)[0] = end_tab[-4];
                ((char *)&d)[1] = end_tab[-3];
                ((char *)&d)[2] = end_tab[-2];
                ((char *)&d)[3] = end_tab[-1];
		if ( s >= d && l + (s=(s-d)*sizeof(short)) < end_tab - 4 ) {
		    ((char *)&offset)[0] = l[s];
		    ((char *)&offset)[1] = l[s+1];
		    if (offset) {
			pc = current_prog->program + offset;
			break;
		    }
		}
		/* default */
		((char *)&offset)[0] = pc[5];
		((char *)&offset)[1] = pc[6];
		pc = current_prog->program + offset;
		break;
	    } else
		fatal("unsupported switch table format.\n");
	l = current_prog->program + offset + off_tab[i];
	d = (off_tab[i]+6) >> 1;
	if (d == 3) d=0;
	for(;;) {
	    ((char *)&r)[0] = l[0];
	    ((char *)&r)[1] = l[1];
	    ((char *)&r)[2] = l[2];
	    ((char *)&r)[3] = l[3];
	    if (s < r)
                if (d < 6) {
                    if (!d) { /* test for range */
			((char *)&offset)[0] = l[-2];
			((char *)&offset)[1] = l[-1];

			/* F_BREAK is required to be > 1 */
			if (offset <= 1) {
		            ((char *)&r)[0] = l[-6];
		            ((char *)&r)[1] = l[-5];
		            ((char *)&r)[2] = l[-4];
		            ((char *)&r)[3] = l[-3];
			    if (s >= r) {
				/* s is in the range */
				if (!offset) {
				    /* range with lookup table */
                                    ((char *)&offset)[0] = l[4];
                                    ((char *)&offset)[1] = l[5];
				    l = current_prog->program + offset +
					(s-r) * sizeof(short);
                                    ((char *)&offset)[0] = l[0];
                                    ((char *)&offset)[1] = l[1];
				    break;
				}
				((char *)&offset)[0] = l[4];
				((char *)&offset)[1] = l[5];
				break;
			    }
			}
			/* use default address */
                        ((char *)&offset)[0] = pc[5];
                        ((char *)&offset)[1] = pc[6];
                        break;
                    } /* !d */
                    d = 0;
                } else {
		    /* d >= 6 */
                    l -= d;
                    d >>= 1;
		}
	    else if (s > r) {
                if (d < 6) {
                    if (!d) { /* test for range */
			((char *)&offset)[0] = l[4];
			((char *)&offset)[1] = l[5];
			if (offset <= 1) {
		            ((char *)&r)[0] = l[6];
		            ((char *)&r)[1] = l[7];
		            ((char *)&r)[2] = l[8];
		            ((char *)&r)[3] = l[9];
			    if (s <= r) {
				/* s is in the range */
				if (!offset) {
				    /* range with lookup table */
                                    ((char *)&offset)[0] = l[10];
                                    ((char *)&offset)[1] = l[11];
				    l = current_prog->program + offset +
					(s-r) * sizeof(short);
                                    ((char *)&offset)[0] = l[0];
                                    ((char *)&offset)[1] = l[1];
				    break;
				}
				((char *)&offset)[0] = l[10];
				((char *)&offset)[1] = l[11];
				break;
			    }
			}
			/* use default address */
                        ((char *)&offset)[0] = pc[5];
                        ((char *)&offset)[1] = pc[6];
                        break;
                    } /* !d */
                    d = 0;
                } else {
		    /* d >= 6 */
                    l += d;
                    while (l >= end_tab) {
                        d >>= 1;
                        if (d <= 3) {
                            if (!d) break;
                            d = 0;
                        }
                        l -= d;
                    }
		    d >>= 1;
		}
	    } else {
		/* s == r */
		((char *)&offset)[0] = l[4];
		((char *)&offset)[1] = l[5];
		if ( !l[-2] && !l[-1] ) {
		    /* end of range with lookup table */
		    ((char *)&r)[0] = l[-6];
		    ((char *)&r)[1] = l[-5];
		    ((char *)&r)[2] = l[-4];
		    ((char *)&r)[3] = l[-3];
		    l = current_prog->program + offset + (s-r)*sizeof(short);
                    ((char *)&offset)[0] = l[0];
                    ((char *)&offset)[1] = l[1];
		}
		if (offset <= 1) {
		    if (!offset) {
			/* start of range with lookup table */
                        ((char *)&offset)[0] = l[10];
                        ((char *)&offset)[1] = l[11];
			l = current_prog->program + offset;
                        ((char *)&offset)[0] = l[0];
                        ((char *)&offset)[1] = l[1];
		    } else {
                        ((char *)&offset)[0] = l[10];
                        ((char *)&offset)[1] = l[11];
		    }
		}
		break;
	    }
	}
	pc = current_prog->program + offset;
	break;
    }
    CASE(F_BREAK);
    {
	pc = current_prog->program + *break_sp++;
	break;
    }
    CASE(F_SUBSCRIPT);
	fatal("F_SUBSCRIPT should not appear.\n");
    CASE(F_STRLEN);
	i = strlen(sp->u.string);
	pop_stack();
	push_number(i);
	break;
    CASE(F_SIZEOF);
	if (sp->type == T_MAPPING)
		i = sp->u.map->size;
	else if (sp->type == T_POINTER)
		i = sp->u.vec->size;
	else
		i = 0;
	pop_stack();
	push_number(i);
	break;
    CASE(F_LOWER_CASE);
    {
	char *str = string_copy(sp->u.string);
	for (i = strlen(str)-1; i>=0; i--)
	    if (isalpha(str[i]))
		str[i] |= 'a' - 'A';
	pop_stack();
	push_malloced_string(str);
	break;
    }
    CASE(F_SET_HEART_BEAT);
	i = set_heart_beat(current_object, sp->u.number);
	sp->u.number = i;
	break;
    CASE(F_CAPITALIZE);
	if (islower(sp->u.string[0])) {
	    char *str;

	    str = string_copy(sp->u.string);
	    str[0] += 'A' - 'a';
	    pop_stack();
	    push_malloced_string(str);
	}
	break;
    CASE(F_PROCESS_STRING);
    {
	extern char
	    *process_string PROT((char *));

	char *str;

	str = process_string(sp->u.string);
	if (str != sp->u.string) {
	    pop_stack();
	    push_malloced_string(str);
	}
	break;
    }
    CASE(F_COMMAND);
    {
	i = command_for_object(sp->u.string, 0);
	pop_stack();
	push_number(i);
	break;
    }
    CASE(F_STAT);
    {
      struct stat buf;
      char *path;
      
      /*
	if a regular file we return:
	(file_size, file_modify_time, object_load_time)
	*/
      
      
      path = (sp-1)->u.string;
      if (*path == '/')
	path++;
      if (stat(path,&buf) != -1) {
	if (buf.st_mode & S_IFREG) { /* if a regular file */
	  struct vector *v;
	  
	  v = allocate_array(3);
	  v->item[0].type = T_NUMBER;
	  v->item[0].u.number = buf.st_size;
	  v->item[1].type = T_NUMBER;
	  v->item[1].u.number = buf.st_mtime;
	  v->item[2].type = T_NUMBER;
	  ob = find_object2(path);
	  if (ob)
	    v->item[2].u.number = ob->load_time;
	  else
	    v->item[2].u.number = 0;
	  pop_n_elems(2);
	  push_vector(v);
	  v->ref--;	/* Will now be 1. */
	  break; /* don't drop through in this case */
	}
      }
    }
	/*
	  drops through to get_dir since stat will supposedly eventually
	  replace get_dir.
	  */
    CASE(F_GET_DIR);
    {
	struct vector *v = get_dir((sp-1)->u.string, sp->u.number);
	
	pop_n_elems(2);
	if (v) {
	    push_vector(v);
	    v->ref--;	/* Will now be 1. */
	} else
	    push_number(0);
	break;
    }
    CASE(F_RM);
	i = remove_file(sp->u.string);
	pop_stack();
	push_number(i);
	break;
    CASE(F_CAT);
    {
	struct svalue *arg = sp- num_arg + 1;
	int start = 0, len = 0;

	if (num_arg > 1)
	    start = arg[1].u.number;
	if (num_arg == 3) {
	    if (arg[2].type != T_NUMBER)
		bad_arg(2, instruction);
	    len = arg[2].u.number;
	}
	i = print_file(arg[0].u.string, start, len);
	pop_n_elems(num_arg);
	push_number(i);
	break;
    }
    CASE(F_MKDIR);
    {
	char *path;

	path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
	/* pop_stack(); see comment above... */
	if (path == 0 || mkdir(path, 0770) == -1)
	    assign_svalue(sp, &const0);
	else
	    assign_svalue(sp, &const1);
	break;
    }
    CASE(F_RMDIR);
    {
	char *path;

	path = check_valid_path(sp->u.string, current_object, "rmdir", 1);
	/* pop_stack(); rw - what the heck ??? */
	if (path == 0 || rmdir(path) == -1)
	    assign_svalue(sp, &const0);
	else
	    assign_svalue(sp, &const1);
	break;
    }
    CASE(F_SET_PROMPT);
    {  /* not implemented yet */
        break;
    }
	CASE(F_KEYS);
    CASE(F_INDICES);
    {
        struct vector *v;
        v = mapping_indices(sp->u.map);
        pop_stack();
        push_vector(v);
        break;
    }
    CASE(F_INPUT_TO);
    {
	i = input_to((sp-1)->u.string, sp->u.number);
	pop_n_elems(2);
	push_number(i);
	break;
    }
    CASE(F_SET_LIVING_NAME);
	set_living_name(current_object, sp->u.string);
	break;
    CASE(F_PARSE_COMMAND);
    {
	struct svalue *arg;

	num_arg = EXTRACT_UCHAR(pc);
	pc++;
	arg = sp - num_arg + 1;
	if (arg[0].type != T_STRING)
	    bad_arg(1, F_PARSE_COMMAND);
	if (arg[1].type != T_OBJECT && arg[1].type != T_POINTER)
	    bad_arg(2, F_PARSE_COMMAND);
	if (arg[2].type != T_STRING)
	    bad_arg(3, F_PARSE_COMMAND);
	if (arg[1].type == T_POINTER)
	    check_for_destr(arg[1].u.vec);

	i = parse(arg[0].u.string, &arg[1], arg[2].u.string, &arg[3],
		  num_arg-3); 
	pop_n_elems(num_arg);	/* Get rid of all arguments */
	push_number(i);		/* Push the result value */
	break;
    }
    CASE(F_SSCANF);
	num_arg = EXTRACT_UCHAR(pc);
	pc++;
	i = inter_sscanf(num_arg);
	pop_n_elems(num_arg);
	push_number(i);
	break;
    CASE(F_ENABLE_COMMANDS);
	enable_commands(1);
	push_number(1);
	break;
    CASE(F_DISABLE_COMMANDS);
	enable_commands(0);
	push_number(0);
	break;
    CASE(F_PRESENT);
	{
	    struct svalue *arg = sp - num_arg + 1;
	    ob = object_present(arg, num_arg == 1 ? 0 : arg[1].u.ob);
	    pop_n_elems(num_arg);
	    if (ob)
		push_object(ob);
	    else
		push_number(0);
	}
	break;
#ifdef F_SET_LIGHT
    CASE(F_SET_LIGHT);
    {
	struct object *o1;

	add_light(current_object, sp->u.number);
	o1 = current_object;
	while(o1->super)
	    o1 = o1->super;
	sp->u.number = o1->total_light;
	break;
    }
#endif /* F_SET_LIGHT */
    CASE(F_CONST0);
	push_number(0);
	break;
    CASE(F_CONST1);
	push_number(1);
	break;
    CASE(F_NUMBER);
	((char *)&i)[0] = pc[0];
	((char *)&i)[1] = pc[1];
	((char *)&i)[2] = pc[2];
	((char *)&i)[3] = pc[3];
	pc += 4;
	push_number(i);
	break;
    CASE(F_ASSIGN);
#ifdef DEBUG
	if (sp[-1].type != T_LVALUE)
	    fatal("Bad argument to F_ASSIGN\n");
#endif
	assign_svalue((sp-1)->u.lvalue, sp);
	assign_svalue(sp-1, sp);
	pop_stack();
	break;
    CASE(F_CTIME);
    {
	char *cp;
	cp = string_copy(time_string(sp->u.number));
	pop_stack();
	push_malloced_string(cp);
	/* Now strip the newline. */
	cp = strchr(cp, '\n');
	if (cp)
	    *cp = '\0';
	break;
    }
    CASE(F_ADD_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_ADD_EQ);
	argp = sp[-1].u.lvalue;
	switch(argp->type) {
	case T_STRING:
	{
	    char *new_str;
	    if (sp->type == T_STRING) {
		int l = strlen(argp->u.string);
		new_str = xalloc(l + strlen(sp->u.string) + 1);
		strcpy(new_str, argp->u.string);
		strcpy(new_str+l, sp->u.string);
		pop_n_elems(2);
		push_malloced_string(new_str);
	    } else if (sp->type == T_NUMBER) {
		char buff[20];
		sprintf(buff, "%d", sp->u.number);
		new_str = xalloc(strlen(argp->u.string) + strlen(buff) + 1);
		strcpy(new_str, argp->u.string);
		strcat(new_str, buff);
		pop_n_elems(2);
		push_malloced_string(new_str);
	    } else {
		bad_arg(2, F_ADD_EQ);
	    }
	    break;
	}
	case T_NUMBER:
	    if (sp->type == T_NUMBER) {
	        i = argp->u.number + sp->u.number;
		pop_n_elems(2);
		push_number(i);
	    } else {
	        error("Bad type number to rhs +=.\n");
	    }
	    break;
        case T_POINTER:
	    if (sp->type != T_POINTER) {
              bad_arg(2, F_ADD_EQ);
	    } else {
	      struct vector *v;
	      check_for_destr(argp->u.vec);
	      check_for_destr(sp->u.vec);
	      v = add_array(argp->u.vec,sp->u.vec);
	      pop_n_elems(2);
	      push_vector(v); /* This will make ref count == 2 */
	      v->ref--;
	    }
	    break;	      
        case T_MAPPING:
            if (sp->type != T_MAPPING) {
               bad_arg(2, F_ADD_EQ);
            } else {
               struct mapping *m;

               m = add_mapping(argp->u.map, sp->u.map);
               pop_n_elems(2);
               push_mapping(m);
               m->ref--;
            }
            break;
	default:
            bad_arg(1, F_ADD_EQ);
	}
	assign_svalue(argp, sp);
	break;
    CASE(F_SUB_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_SUB_EQ);
	argp = sp[-1].u.lvalue;
	switch (argp->type) {
	case T_NUMBER:
	if (sp->type != T_NUMBER)
	        error("Bad right type to -=");
	    argp->u.number -= sp->u.number;
	    sp--;
            break;
	case T_POINTER:
	  {
	    struct vector *subtract_array PROT((struct vector*,struct vector*));
	    struct vector *v;

	    if (sp->type != T_POINTER)
	        error("Bad right type to -=");
	    v = sp->u.vec;
	    if (v->ref > 1) {
		v = slice_array(v, 0, v->size-1 );
		v->ref--;
            }
	    sp--;
	    v = subtract_array(argp->u.vec, v);
	    free_vector(argp->u.vec);
	    argp->u.vec = v;
	    break;
	  }
	default:
	    error("Bad left type to -=.\n");
	}
	assign_svalue_no_free(sp, argp);
	break;
    CASE(F_MULT_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_MULT_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != sp->type)
	    error("Mismatched types on *=.\n");
	if (sp->type == T_NUMBER) {
	   i = argp->u.number * sp->u.number;
	   pop_n_elems(2);
	   push_number(i);
        } else if (sp->type == T_MAPPING) {
           struct mapping *m;
           m = compose_mapping(argp->u.map, sp->u.map);
           pop_n_elems(2);
           push_mapping(m);
        } else
           bad_arg(2, F_MULT_EQ);
	assign_svalue(argp, sp);
	break;
    CASE(F_AND_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_AND_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to &=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to &=");
	i = argp->u.number & sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_OR_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_OR_EQ);
	argp = sp[-1].u.lvalue;
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_OR_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to |=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to |=");
	i = argp->u.number | sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_XOR_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_XOR_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to ^=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to ^=");
	i = argp->u.number ^ sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_LSH_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_LSH_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to <<=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to <<=");
	i = argp->u.number << sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_RSH_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_RSH_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to >>=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to >>=");
	i = argp->u.number >> sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_DIV_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_DIV_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to /=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to /=");
	if (sp->u.number == 0)
	    error("Division by 0\n");
	i = argp->u.number / sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_MOD_EQ);
	if (sp[-1].type != T_LVALUE)
	    bad_arg(1, F_MOD_EQ);
	argp = sp[-1].u.lvalue;
	if (argp->type != T_NUMBER)
	    error("Bad left type to %=.\n");
	if (sp->type != T_NUMBER)
	    error("Bad right type to %=");
	if (sp->u.number == 0)
	    error("Division by 0\n");
	i = argp->u.number % sp->u.number;
	pop_n_elems(2);
	push_number(i);
	assign_svalue(argp, sp);
	break;
    CASE(F_STRING);
    {
	unsigned short string_number;
	((char *)&string_number)[0] = pc[0];
	((char *)&string_number)[1] = pc[1];
	pc += 2;
	push_string(current_prog->strings[string_number],
		    STRING_CONSTANT);
	break;
    }
#ifdef F_RUSAGE
    CASE(F_RUSAGE);
    {
        char buff[500];

	struct rusage rus;
	long usertime, stime;
	int maxrss;

	if (getrusage(RUSAGE_SELF, &rus) < 0)
            buff[0] = 0;
	else {
	    usertime = rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
	    stime = rus.ru_stime.tv_sec * 1000 + rus.ru_stime.tv_usec / 1000;
	    maxrss = rus.ru_maxrss;
#ifdef sun
	    maxrss *= getpagesize() / 1024;
#endif
	    sprintf(buff, "%ld %ld %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		    usertime, stime, maxrss, rus.ru_ixrss, rus.ru_idrss,
		    rus.ru_isrss, rus.ru_minflt, rus.ru_majflt, rus.ru_nswap,
		    rus.ru_inblock, rus.ru_oublock, rus.ru_msgsnd, 
		    rus.ru_msgrcv, rus.ru_nsignals, rus.ru_nvcsw, 
		    rus.ru_nivcsw);
	  }
	push_string(buff, STRING_MALLOC);
	break;
    }
#endif
    CASE(F_CINDENT);
    {
	char *path;

	path = check_valid_path(sp->u.string, current_object, "cindent", 1);
	if (path) {
	    if (indent_program(path)) {
		assign_svalue(sp, &const1);
		break;
	    }
	} else {
	    add_message("Illegal attempt to indent\n");
	}
	assign_svalue(sp, &const0);
	break;
    }
    CASE(F_DESCRIBE);
    {
	char *str;
	int live;

	if (num_arg < 3) live = 0;
	else {
	    if (sp->type != T_NUMBER) bad_arg (3, F_DESCRIBE);
	    live = sp->u.number;
	    pop_stack ();
	}
	str = describe_items(sp-1, sp->u.string, live);
	pop_n_elems(2);
	if (str) push_malloced_string (string_copy (str));
	else     push_number(0);
	break;
    }
    CASE(F_UNIQUE_ARRAY); {
	extern struct vector
	    *make_unique PROT((struct vector *arr,char *func,
	    struct svalue *skipnum));
	struct vector *res;

	if (num_arg < 3) {
	    check_for_destr((sp-1)->u.vec);
	    res = make_unique((sp-1)->u.vec, sp->u.string, &const0);
	} else {
	    check_for_destr((sp-2)->u.vec);
	    res = make_unique((sp-2)->u.vec, (sp-1)->u.string, sp);
	    pop_stack ();
	}
	pop_n_elems(2);
	if (res) {
	    push_vector (res);	/* This will make ref count == 2 */
	    res->ref--;
	} else
	    push_number (0);
	break;
    }
    CASE(F_VERSION); {
      char buff[80]; /* expanded this a little */
      
      get_version(buff);
      push_string(buff, STRING_MALLOC);
      break;
    }
#ifdef F_RENAME
    CASE(F_RENAME); {
	i = do_rename((sp-1)->u.string, sp->u.string, F_RENAME);
	pop_n_elems(2);
	push_number(i);
	break;
    }
#endif /* F_RENAME */
#ifdef F_LINK
	CASE(F_LINK);
	{
		struct svalue *ret;

		push_string((sp-1)->u.string,STRING_CONSTANT);
		push_string(sp->u.string,STRING_CONSTANT);
        ret = apply_master_ob("valid_ln",2);
		if (!IS_ZERO(ret)) {
			i = do_rename((sp-1)->u.string, sp->u.string, F_LINK);
		}
		pop_n_elems(2);
		push_number(i);
		break;
	}
#endif
    CASE(F_MAP_ARRAY); {
	struct vector *res;
        struct mapping *map = (struct mapping *) 0;
	struct svalue *arg;

	arg = sp - num_arg + 1; ob = 0;

	if (arg[2].type == T_OBJECT)
	    ob = arg[2].u.ob;
	else if (arg[2].type == T_STRING) 
	    ob = find_object(arg[2].u.string);

	if (!ob)
	    bad_arg (3, F_MAP_ARRAY);

	if (arg[0].type == T_POINTER) {
	    check_for_destr(arg[0].u.vec);
	    res = map_array (arg[0].u.vec, arg[1].u.string, ob,
			     num_arg > 3 ? sp : (struct svalue *)0);
        } else if (arg[0].type == T_MAPPING) {
            map = map_mapping (arg[0].u.map, arg[1].u.string, ob,
                             num_arg > 3 ? sp : (struct svalue *) 0);
	} else {
	    res = 0;
	}
	pop_n_elems (num_arg);
        if (map) {
           push_mapping(map);
           map->ref--;
        } else if (res) {
	    push_vector (res);	/* This will make ref count == 2 */
	    res->ref--;
	} else
	    push_number (0);
	break;
    }
    CASE(F_SORT_ARRAY); {
	extern struct vector *sort_array
	  PROT((struct vector*,char *,struct object *));
	struct vector *res;
	struct svalue *arg;

	arg = sp - 2; ob = 0;

	if (arg[2].type == T_OBJECT)
	    ob = arg[2].u.ob;
	else if (arg[2].type == T_STRING) 
	    ob = find_object(arg[2].u.string);

	if (!ob)
	    bad_arg (3, F_SORT_ARRAY);

	if (arg[0].type == T_POINTER) {
	    /* sort_array already takes care of destructed objects */
	    res = sort_array (
	      slice_array(arg[0].u.vec, 0, arg[0].u.vec->size-1),
	      arg[1].u.string, ob);
	} else
	    res = 0;
	pop_n_elems (3);
	sp++;
	if (res) {
	    sp->type = T_POINTER;
	    sp->u.vec = res;
	}
	else     *sp = const0;
	break;
    }
#ifdef F_ASSOC
    CASE(F_ASSOC);
    {
	/* When the key list of an alist contains destructed objects
	   it is better not to free them till the next reordering by
	   order_alist to retain the alist property.
	 */
	int assoc PROT((struct svalue *key, struct vector *keys));
	struct svalue *args = sp -num_arg +1;
	struct vector *keys,*data;
	struct svalue *fail_val;
	int ix;

	if ( !args[1].u.vec->size ||
	  args[1].u.vec->item[0].type != T_POINTER ) {
	    keys = args[1].u.vec;
	    if (num_arg == 2) {
		data = (struct vector *)NULL;
	    } else {
		if (args[2].type != T_POINTER ||
		  args[2].u.vec->size != keys->size) {
		    bad_arg(3,F_ASSOC);
		}
		data = args[2].u.vec;
	    }
	    if (num_arg == 4) {
		fail_val = &args[3];
	    } else {
		fail_val = &const0;
	    }
	} else {
	    keys = args[1].u.vec->item[0].u.vec;
	    if (args[1].u.vec->size > 1) {
		if (args[1].u.vec->item[1].type != T_POINTER ||
		    args[1].u.vec->item[1].u.vec->size != keys->size) {
			bad_arg(2,F_ASSOC);
	        }
		data = args[1].u.vec->item[1].u.vec;
	    } else {
		data = (struct vector *)NULL;
	    }
	    if (num_arg == 3) fail_val = &args[2];
	    else if (num_arg == 2) fail_val = &const0;
	    else {
		error ("too many args to efun assoc");
	    }
	}
	ix = assoc(&args[0],keys);
	if (data == (struct vector *)NULL) {
	    pop_n_elems(num_arg);
	    push_number(ix);
	} else {
	    assign_svalue(args, ix==-1 ? fail_val : &data->item[ix]);
	    pop_n_elems(num_arg-1);
	}
        break;
    }
#endif /* F_ASSOC */
#ifdef F_DEBUG_INFO
    CASE(F_DEBUG_INFO);
    {
	struct svalue *arg = sp-num_arg+1;
	struct svalue res;

	switch ( arg[0].u.number ) {
	    case 0:
	    {
		int flags;
		struct object *obj2;

		if (num_arg != 2)
	    	    error("bad number of arguments to debug_info");
		if ( arg[1].type != T_OBJECT)
		    bad_arg(1,instruction);
		ob = arg[1].u.ob;
		flags = ob->flags;
		add_message("O_HEART_BEAT      : %s\n",
		  flags&O_HEART_BEAT      ?"TRUE":"FALSE");
		add_message("O_IS_WIZARD       : %s\n",
		  flags&O_IS_WIZARD       ?"TRUE":"FALSE");
		add_message("O_ENABLE_COMMANDS : %s\n",
		  flags&O_ENABLE_COMMANDS ?"TRUE":"FALSE");
		add_message("O_CLONE           : %s\n",
		  flags&O_CLONE           ?"TRUE":"FALSE");
		add_message("O_DESTRUCTED      : %s\n",
		  flags&O_DESTRUCTED      ?"TRUE":"FALSE");
		add_message("O_SWAPPED         : %s\n",
		  flags&O_SWAPPED          ?"TRUE":"FALSE");
		add_message("O_ONCE_INTERACTIVE: %s\n",
		  flags&O_ONCE_INTERACTIVE?"TRUE":"FALSE");
		add_message("O_PRIVILEGED      : %s\n",
		  flags&O_PRIVILEGED?"TRUE":"FALSE");
		add_message("O_RESET_STATE     : %s\n",
		  flags&O_RESET_STATE     ?"TRUE":"FALSE");
		add_message("O_WILL_CLEAN_UP   : %s\n",
		  flags&O_WILL_CLEAN_UP   ?"TRUE":"FALSE");
	        add_message("total light : %d\n", ob->total_light);
		add_message("next_reset  : %d\n", ob->next_reset);
		add_message("time_of_ref : %d\n", ob->time_of_ref);
		add_message("ref         : %d\n", ob->ref);
#ifdef DEBUG
		add_message("extra_ref   : %d\n", ob->extra_ref);
#endif
		add_message("swap_num    : %ld\n", ob->swap_num);
		add_message("name        : '%s'\n", ob->name);
		add_message("next_all    : OBJ(%s)\n",
		  ob->next_all?ob->next_all->name:"NULL");
		if (obj_list == ob) add_message(
		    "This object is the head of the object list.\n");
		for (obj2=obj_list,i=1; obj2; obj2=obj2->next_all,i++)
		    if (obj2->next_all == ob) {
			add_message(
			    "Previous object in object list: OBJ(%s)\n",
			    obj2->name);
			add_message("position in object list:%d\n",i);
		    }
		assign_svalue_no_free(&res,&const0);
		break;
	    }
	    case 1: {
		if (num_arg != 2)
	    	    error("bad number of arguments to debug_info");
		if ( arg[1].type != T_OBJECT)
		    bad_arg(1,instruction);
		ob = arg[1].u.ob;
		
		add_message("program ref's %d\n", ob->prog->ref);
		add_message("Name %s\n", ob->prog->name);
		add_message("program size %d\n",
		ob->prog->program_size);
		add_message("num func's %d (%d) \n", ob->prog->num_functions
		,ob->prog->num_functions * sizeof(struct function));
		add_message("num strings %d\n", ob->prog->num_strings);
		add_message("num vars %d (%d)\n", ob->prog->num_variables
		,ob->prog->num_variables * sizeof(struct variable));
		add_message("num inherits %d (%d)\n", ob->prog->num_inherited
		,ob->prog->num_inherited * sizeof(struct inherit));
		add_message("total size %d\n", ob->prog->total_size);
		assign_svalue_no_free(&res,&const0);
		break;
		}
	    default: bad_arg(1,instruction);
	}
	pop_n_elems(num_arg);
	sp++;
	*sp=res;
	break;
    }
#endif /* F_DEBUG_INFO */
    CASE(F_CP); {
        i = copy_file(sp[-1].u.string, sp[0].u.string);
        pop_n_elems(2);
        push_number(i);
        break;
    }
    CASE(F_GET_CHAR);
    {
	struct svalue *arg = sp - num_arg + 1;
	int flag = 1;
	
	if (num_arg == 1 || (sp->type == T_NUMBER && sp->u.number == 0))
	    flag = 0;
	i = get_char(arg[0].u.string, flag);
	pop_n_elems(num_arg);
	push_number(i);
	break;
    }
    CASE(F_MUD_NAME); {
      push_string(MUD_NAME, STRING_MALLOC);
      break;
    }
#ifdef F_SPRINTF
    CASE(F_SPRINTF);
    {
      char *s;

      /*
       * string_print_formatted() returns a pointer to it's internal
       * buffer, or to an internal constant...  Either way, it must
       * be copied before it's returned as a string.
       */

      s = string_print_formatted((sp-num_arg+1)->u.string,
                                 num_arg-1, sp-num_arg+2);
      pop_n_elems(num_arg);
      if (!s) push_number(0); else push_malloced_string(string_copy(s));
      break;
    }
#endif /* F_SPRINTF */
#ifdef F_PRINTF
    CASE(F_PRINTF);
      add_message("%s", string_print_formatted((sp-num_arg+1)->u.string,
                                               num_arg-1, sp-num_arg+2));
      pop_n_elems(num_arg-1);
      break;
#endif /* F_PRINTF */   
#if 0
/* not really close with this one yet */
    CASE(F_MUD_STATUS); {
      push_vector(mud_status());   
      sp->u.vec->ref--;
      break;
    }
#endif
   CASE(F_CHILDREN); {
	struct vector *vec;
   vec = children(sp->u.string);
	pop_stack();
	if (vec == 0) {
	    push_number(0);
	} else {
	    push_vector(vec); /* This will make ref count == 2 */
	    vec->ref--;
	}
	break;
    }
    CASE(F_MESSAGE);
    {
    int freeuse = 0,freeavoid = 0;
    struct vector *use,*avoid;
    extern struct vector null_vector;
    struct svalue *args;
    static struct vector vtmp1 = { 1,1,
#ifdef DEBUG
         1,
#endif
         (struct wiz_list *)NULL};
    static struct vector vtmp2 = { 1,1,
#ifdef DEBUG
         1,
#endif
         (struct wiz_list *)NULL};
	
   args = sp - num_arg + 1;
   if(args[2].type == T_OBJECT)
   {
      vtmp1.item[0].type = T_OBJECT;
      vtmp1.item[0].u.ob = args[2].u.ob;
      add_ref(args[2].u.ob,"message");
      use = &vtmp1;
   }
   else
   if(args[2].type == T_STRING)
   {
      vtmp1.item[0].type = T_STRING;
      vtmp1.item[0].u.string = make_shared_string(args[2].u.string);
      vtmp1.item[0].subtype = STRING_MALLOC;
      use = &vtmp1;
   }
   else
   {
      if(args[2].u.vec->ref > 1) {
         use = slice_array(args[2].u.vec,0,args[2].u.vec->size-1);
         freeuse = 1;
      }
      else
         use = args[2].u.vec;
   }

   if(num_arg == 4)
   {
      if(args[3].type == T_OBJECT)
      {
         vtmp2.item[0].type = T_OBJECT;
         vtmp2.item[0].u.ob = args[3].u.ob;
         add_ref(args[3].u.ob,"message");
         avoid = &vtmp2;
      }
      else
      {
         if(args[3].u.vec->ref > 1) {
            avoid = slice_array(args[3].u.vec,0,args[3].u.vec->size-1);
            freeavoid = 1;
         }
         else
            avoid = args[3].u.vec;
      }
   }
   else
      avoid = &null_vector;

   do_message(args[0].u.string,args[1].u.string,use,avoid);
   if(freeuse)
      free_vector(use);
   if(freeavoid)
      free_vector(avoid);
      pop_n_elems(num_arg);
      break;
    }
    }
#ifdef DEBUG
    if (expected_stack && expected_stack != sp ||
	sp < fp + csp->num_local_variables - 1)
    {
	fatal("Bad stack after evaluation. Instruction %d, num arg %d\n",
	      instruction, num_arg);
    }
#endif /* DEBUG */
    goto again;
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
 * manually !
 */

char debug_apply_fun[30]; /* For debugging */

static int apply_low(fun, ob, num_arg)
    char *fun;
    struct object *ob;
    int num_arg;
{
    static int cache_id[0x40] = {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    static char *cache_name[0x40];
    static struct function *cache_pr[0x40];
    static struct function *cache_pr_inherited[0x40];
    static struct program *cache_progp[0x40];
    static int cache_function_index_offset[0x40];
    static int cache_variable_index_offset[0x40];

    struct function *pr;
    struct program *progp;
    extern int num_error;
    struct control_stack *save_csp;
    int ix;

    ob->time_of_ref = current_time;	/* Used by the swapper */
    /*
     * This object will now be used, and is thus a target for
     * reset later on (when time due).
     */
    ob->flags &= ~O_RESET_STATE;
#ifdef DEBUG
    strncpy(debug_apply_fun, fun, sizeof debug_apply_fun);
    debug_apply_fun[sizeof debug_apply_fun - 1] = '\0';
#endif
    if (num_error > 0)
	goto failure;
    if (fun[0] == ':')
	error("Illegal function call\n");
    /*
     * If there is a chain of objects shadowing, start with the first
     * of these.
     */
#ifndef NO_SHADOWS /* LPCA */
    while (ob->shadowed && ob->shadowed != current_object)
	ob = ob->shadowed;
#endif
retry_for_shadow:
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    progp = ob->prog;
#ifdef DEBUG
    if (ob->flags & O_DESTRUCTED)
	fatal("apply() on destructed object\n");
#endif
    ix = ( progp->id_number ^ (int)fun ^ ( (int)fun >> 6 ) ) & 0x3f;
    if (cache_id[ix] == progp->id_number && !strcmp(cache_name[ix], fun) &&
	(!cache_progp[ix] || cache_progp[ix] == ob->prog)) {
        /* We have found a matching entry in the cache. The pointer to
	   the function name has to match, not only the contents.
	   This is because hashing the string in order to get a cache index
	   would be much more costly than hashing it's pointer.
	   If cache access would be costly, the cache would be useless.
	   */
	if (cache_progp[ix]) {
	    /* the cache will tell us in wich program the function is, and
	     * where
	     */
	    push_control_stack(cache_pr[ix]);
	    csp->num_local_variables = num_arg;
	    current_prog = cache_progp[ix];
	    pr = cache_pr_inherited[ix];
	    function_index_offset = cache_function_index_offset[ix];
	    variable_index_offset = cache_variable_index_offset[ix];
	    /* Remove excessive arguments */
	    while(csp->num_local_variables > pr->num_arg) {
		pop_stack();
		csp->num_local_variables--;
	    }
	    /* Correct number of arguments and local variables */
	    while(csp->num_local_variables < pr->num_arg + pr->num_local) {
		push_number(0);
		csp->num_local_variables++;
	    }
	    tracedepth++;
	    if (TRACEP(TRACE_CALL)) {
		do_trace_call(pr);
	    }
	    fp = sp - csp->num_local_variables + 1;
	    break_sp = (short*)(sp+1);
#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
	    /* Now, previous_object() is always set, even by
	     * call_other(this_object()). It should not break any
	     * compatibility.
	     */
	    if (current_object != ob)
#endif
		previous_ob = current_object;
	    current_object = ob;
	    save_csp = csp;
	    eval_instruction(current_prog->program + pr->offset);
#ifdef DEBUG
	    if (save_csp-1 != csp)
		fatal("Bad csp after execution in apply_low\n");
#endif
	    /*
	     * Arguments and local variables are now removed. One
	     * resulting value is always returned on the stack.
	     */
	    return 1;
	} /* when we come here, the cache has told us that the function isn't
	   * defined in the object
	   */
    } else {
	/* we have to search the function */
	if (!cache_progp[ix] && cache_id[ix]) {
	    /* The old cache entry was for an undefined function, so the
	       name had to be malloced */
	    FREE(cache_name[ix]);
	}
	cache_id[ix] = progp->id_number;
        for(pr=progp->functions; pr < progp->functions + progp->num_functions;
            pr++)
        {
            eval_cost++;
            if (pr->name == 0 ||
                pr->name[0] != fun[0] ||
                strcmp(pr->name, fun) != 0 ||
                (pr->type & TYPE_MOD_PRIVATE))
            {
                continue;
            }
            if (pr->flags & NAME_UNDEFINED)
                continue;
            /* Static functions may not be called from outside. */
			if ((pr->type & (TYPE_MOD_STATIC|TYPE_MOD_PRIVATE)) &&
				current_object != ob) /* fix from 3.1.1 */
			{
                continue;
			}
	    /* The searched function is found */
	    cache_pr[ix] = pr;
	    cache_name[ix] = pr->name;
	    push_control_stack(pr);
	    csp->num_local_variables = num_arg;
	    current_prog = progp;
	    pr = setup_new_frame(pr);
	    cache_pr_inherited[ix] = pr;
	    cache_progp[ix] = current_prog;
	    cache_variable_index_offset[ix] = variable_index_offset;
	    cache_function_index_offset[ix] = function_index_offset;
#ifdef OLD_PREVIOUS_OBJECT_BEHAVIOUR
            if (current_object != ob)
#endif
                previous_ob = current_object;
            current_object = ob;
            save_csp = csp;
            eval_instruction(current_prog->program + pr->offset);
#ifdef DEBUG
            if (save_csp-1 != csp)
                fatal("Bad csp after execution in apply_low\n");
#endif
            /*
             * Arguments and local variables are now removed. One
             * resulting value is always returned on the stack.
             */
            return 1;
	}
	/* We have to mark a function not to be in the object */
	cache_name[ix] = string_copy(fun);
	cache_progp[ix] = (struct program *)0;
    }
#ifndef NO_SHADOWS /* LPCA */
    if (ob->shadowing) {
	/*
	 * This is an object shadowing another. The function was not found,
	 * but can maybe be found in the object we are shadowing.
	 */
	ob = ob->shadowing;
	goto retry_for_shadow;
    }
#endif
failure:
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

static struct svalue *sapply(fun, ob, num_arg)
    char *fun;
    struct object *ob;
    int num_arg;
{
#ifdef DEBUG
    struct svalue *expected_sp;
#endif
    static struct svalue ret_value = { T_NUMBER };

    if (TRACEP(TRACE_APPLY)) {
	do_trace("Apply", "", "\n");
    }
#ifdef DEBUG
    expected_sp = sp - num_arg;
#endif
    if (apply_low(fun, ob, num_arg) == 0)
	return 0;
    assign_svalue(&ret_value, sp);
    pop_stack();
#ifdef DEBUG
    if (expected_sp != sp)
	fatal("Corrupt stack pointer.\n");
#endif
    return &ret_value;
}

struct svalue *apply(fun, ob, num_arg)
    char *fun;
    struct object *ob;
    int num_arg;
{
    tracedepth = 0;
    return sapply(fun, ob, num_arg);
}

/*
 * Call a function in all objects in a vector.
 */
int call_all_other(v, func, numargs)
    struct vector *v;
    char *func;
    int numargs;
{
    int idx;
    for (idx = 0; idx < v->size; idx++) {
        struct object *ob;
        int i;
        if (v->item[idx].type != T_OBJECT) continue;
        ob = v->item[idx].u.ob;
        if (ob->flags & O_DESTRUCTED) continue;
        for (i = numargs; i--; )
           push_svalue(sp - i + 1);
        if (apply_low(func, ob, numargs))
			pop_stack();
    }
	pop_n_elems(numargs);
    return 0;  /* for now */
}

/*
 * This function is similar to apply(), except that it will not
 * call the function, only return object name if the function exists,
 * or 0 otherwise.
 */
char *function_exists(fun, ob)
    char *fun;
    struct object *ob;
{
    struct function *pr;

#ifdef DEBUG
    if (ob->flags & O_DESTRUCTED)
	fatal("function_exists() on destructed object\n");
#endif
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    pr = ob->prog->functions;
    for(; pr < ob->prog->functions + ob->prog->num_functions; pr++) {
	struct program *progp;

	if (pr->name[0] != fun[0] || strcmp(pr->name, fun) != 0)
	    continue;
	/* Static functions may not be called from outside. */
	if ((pr->type & TYPE_MOD_STATIC) && current_object != ob)
	    continue;
	if (pr->flags & NAME_UNDEFINED)
	    return 0;
	for (progp = ob->prog; pr->flags & NAME_INHERITED;) {
	    progp = progp->inherit[pr->offset].prog;
	    pr = &progp->functions[pr->function_index_offset];
	}
	return progp->name;
    }
    return 0;
}

/*
 * Call a specific function address in an object. This is done with no
 * frame set up. It is expected that there are no arguments. Returned
 * values are removed.
 */

void call_function(progp, pr)
    struct program *progp;
    struct function *pr;
{
    if (pr->flags & NAME_UNDEFINED)
	return;
    push_control_stack(pr);
#ifdef DEBUG
    if (csp != control_stack)
	fatal("call_function with bad csp\n");
#endif
    csp->num_local_variables = 0;
    current_prog = progp;
    pr = setup_new_frame(pr);
    previous_ob = current_object;
    tracedepth = 0;
    eval_instruction(current_prog->program + pr->offset);
    pop_stack();	/* Throw away the returned result */
}

/*
 * This can be done much more efficiently, but the fix has
 * low priority.
 */
static int get_line_number(p, progp)
    char *p;
    struct program *progp;
{
    int offset;
    int i;
    if (progp == 0)
	return 0;
    offset = p - progp->program;
#ifdef DEBUG
    if (offset > progp->program_size)
	fatal("Illegal offset %d in object %s\n", offset, progp->name);
#endif
    for (i=0; offset > progp->line_numbers[i]; i++)
	;
    return i + 1;
}
    
/*
 * Write out a trace. If there is an heart_beat(), then return the
 * object that had that heart beat.
 */
char *dump_trace(how)
    int how;
{
    struct control_stack *p;
    char *ret = 0;
#ifdef DEBUG
    int last_instructions PROT((void));
#endif

    if (current_prog == 0)
	return 0;
    if (csp < &control_stack[0]) {
	(void)printf("No trace.\n");
	debug_message("No trace.\n");
	return 0;
    }
#ifdef DEBUG
#ifdef TRACE_CODE
    if (how)
	(void)last_instructions();
#endif
#endif
    for (p = &control_stack[0]; p < csp; p++) {
	(void)printf("'%15s' in '%20s' ('%20s')line %d\n",
		     p[0].funp ? p[0].funp->name : "CATCH",
		     p[1].prog->name, p[1].ob->name,
		     get_line_number(p[1].pc, p[1].prog));
	debug_message("'%15s' in '%20s' ('%20s')line %d\n",
		     p[0].funp ? p[0].funp->name : "CATCH",
		     p[1].prog->name, p[1].ob->name,
		     get_line_number(p[1].pc, p[1].prog));
	if (p->funp && strcmp(p->funp->name, "heart_beat") == 0)
	    ret = p->ob?p->ob->name:0; /*crash unliked gc*/
    }
    (void)printf("'%15s' in '%20s' ('%20s')line %d\n",
		 p[0].funp ? p[0].funp->name : "CATCH",
		 current_prog->name, current_object->name,
		 get_line_number(pc, current_prog));
    debug_message("'%15s' in '%20s' ('%20s')line %d\n",
		 p[0].funp ? p[0].funp->name : "CATCH",
		 current_prog->name, current_object->name,
		 get_line_number(pc, current_prog));
    return ret;
}

int get_line_number_if_any() {
    if (current_prog)
	return get_line_number(pc, current_prog);
    return 0;
}

static char *
find_percent(str)
char *str;
{
    while (1) {
		str = strchr(str, '%');
		if (str == 0)
			return 0;
		if (str[1] != '%')
			return str;
		str++;
    }
}

static int
inter_sscanf(num_arg)
int num_arg;
{
    char *fmt;		/* Format description */
    char *in_string;	/* The string to be parsed. */
    int number_of_matches;
    char *cp;
    int skipme = 0;    /* LPCA: Encountered a '*' ? */
    struct svalue *arg = sp - num_arg + 1;

    /*
     * First get the string to be parsed.
     */
    if (arg[0].type != T_STRING)
		bad_arg(1, F_SSCANF);
    in_string = arg[0].u.string;
	if (in_string == 0)
		return 0;
    /*
     * Now get the format description.
     */
    if (arg[1].type != T_STRING)
		bad_arg(2, F_SSCANF);
    fmt = arg[1].u.string;
    /*
     * First, skip and match leading text.
     */
    for (cp=find_percent(fmt); fmt != cp; fmt++, in_string++) {
	if (in_string[0] == '\0' || fmt[0] != in_string[0])
	    return 0;
    }
    /*
     * Loop for every % or substring in the format. Update num_arg and the
     * arg pointer continuosly. Assigning is done manually, for speed.
     */
    num_arg -= 2;
    arg += 2;
	for (number_of_matches=0; num_arg > 0;
		number_of_matches++, num_arg--, arg++) {
		int i, type;

	if (fmt[0] == '\0') {
	    /*
	     * We have reached end of the format string.
	     * If there are any chars left in the in_string,
	     * then we put them in the last variable (if any).
	     */
	    if (in_string[0]) {
		free_svalue(arg->u.lvalue);
		arg->u.lvalue->type = T_STRING;
		arg->u.lvalue->u.string = string_copy(in_string);
		arg->u.lvalue->subtype = STRING_MALLOC;
		number_of_matches++;
	    }
	    break;
	}
#ifdef DEBUG
	if (fmt[0] != '%')
	    fatal("In sscanf, should be a %% now !\n");
#endif
	type = T_STRING;
	if (fmt[1] == '*') { /* handles %*s and %*d as in C's sscanf() - LPCA */
		skipme++;
		fmt++;
	}
	if (fmt[1] == 'd')
	    type = T_NUMBER;
	else if (fmt[1] != 's')
	    error("Bad type : '%%%c' in sscanf fmt string.", fmt[1]);
	if (skipme) {
		num_arg++; /* we didn't use an arg this time around */
		arg--;
	}
	fmt += 2;
	/*
	 * Parsing a number is the easy case. Just use strtol() to
	 * find the end of the number.
	 */
	if (type == T_NUMBER) {
	    char *tmp = in_string;
	    int tmp_num;

	    tmp_num = (int) strtol(in_string, &in_string, 10);
		if (!skipme) {
			if (tmp == in_string) {
				/* No match */
				break;
			}
			free_svalue(arg->u.lvalue);
			arg->u.lvalue->type = T_NUMBER;
			arg->u.lvalue->u.number = tmp_num;
		}
	    while(fmt[0] && fmt[0] == in_string[0]) {
			fmt++;
			in_string++;
		}
		skipme = 0;  /* just in case it was ++'d */
	    if (fmt[0] != '%') {
			number_of_matches++;
			break;
		}
	    continue;
	}
	/*
	 * Now we have the string case.
	 */
	cp = find_percent(fmt);
	if (cp == fmt)
	    error("Illegal to have 2 adjacent %'s in fmt string in sscanf.");
	if (cp == 0)
		cp = fmt + strlen(fmt);
	/*
	 * First case: There were no extra characters to match.
	 * Then this is the last match.
	 */
	if (cp == fmt) {
	    number_of_matches++;
		if (skipme)
			break;
	    free_svalue(arg->u.lvalue);
	    arg->u.lvalue->type = T_STRING;
	    arg->u.lvalue->u.string = string_copy(in_string);
	    arg->u.lvalue->subtype = STRING_MALLOC;
	    break;
	}
	for (i=0; in_string[i]; i++) {
	    if (strncmp(in_string+i, fmt, cp - fmt) == 0) {
			char *match;
			/*
			 * Found a match !
			 */
			if (skipme) { /* LPCA */
				in_string += (i + cp - fmt);
				number_of_matches++;
				fmt = cp; /* advance fmt to next % */
				skipme = 0;
			}
			else {
				match = xalloc(i+1);
				(void)strncpy(match, in_string, i);
				in_string += i + cp - fmt;
				match[i] = '\0';
				free_svalue(arg->u.lvalue);
				arg->u.lvalue->type = T_STRING;
				arg->u.lvalue->u.string = match;
				arg->u.lvalue->subtype = STRING_MALLOC;
				fmt = cp;	/* Advance fmt to next % */
			}
			break;
		}
	}
	if (fmt == cp)	/* If match, then do continue. */
	    continue;
	/*
	 * No match was found. Then we stop here, and return
	 * the result so far !
	 */
	break;
    }
    return number_of_matches;
}

/* test stuff ... -- LA */
#ifdef OPCPROF
void opcdump()
{
    int i;

    for(i = 0; i < MAXOPC; i++)
	if (opcount[i]) printf("%d: %d\n", i, opcount[i]);
}
#endif

/*
 * Reset the virtual stack machine.
 */
void reset_machine(first)
    int first;
{
    csp = control_stack - 1;
    if (first)
	sp = start_of_stack - 1;
    else
	pop_n_elems(sp - start_of_stack + 1);
}

#ifdef TRACE_CODE

static char *get_arg(a, b)
    int a, b;
{
    static char buff[10];
    char *from, *to;

    from = previous_pc[a]; to = previous_pc[b];
    if (to - from < 2)
	return "";
    if (to - from == 2) {
	sprintf(buff, "%d", from[1]);
	return buff;
    }
    if (to - from == 3) {
	short arg;
	((char *)&arg)[0] = from[1];
	((char *)&arg)[1] = from[2];
	sprintf(buff, "%d", arg);
	return buff;
    }
    if (to - from == 5) {
	int arg;
	((char *)&arg)[0] = from[1];
	((char *)&arg)[1] = from[2];
	((char *)&arg)[2] = from[3];
	((char *)&arg)[3] = from[4];
	sprintf(buff, "%d", arg);
	return buff;
    }
    return "";
}

int last_instructions() {
    int i;
    i = last;
    do {
	if (previous_instruction[i] != 0)
	    printf("%6x: %3d %8s %-25s (%d)\n", previous_pc[i],
		   previous_instruction[i],
		   get_arg(i, (i+1) %
			   (sizeof previous_instruction / sizeof (int))),
		   get_f_name(previous_instruction[i]),
		   stack_size[i] + 1);
	i = (i + 1) % (sizeof previous_instruction / sizeof (int));
    } while (i != last);
    return last;
}

#endif /* TRACE_CODE */


#ifdef DEBUG

static void count_inherits(progp, search_prog)
    struct program *progp, *search_prog;
{
    int i;

    /* Clones will not add to the ref count of inherited progs */
    if (progp->extra_ref != 1) return; /* marion */
    for (i=0; i< progp->num_inherited; i++) {
	progp->inherit[i].prog->extra_ref++;
	if (progp->inherit[i].prog == search_prog)
	    printf("Found prog, inherited by %s\n", progp->name);
	count_inherits(progp->inherit[i].prog, search_prog);
    }
}

static void count_ref_in_vector(svp, num)
    struct svalue *svp;
    int num;
{
    struct svalue *p;

    for (p = svp; p < svp+num; p++) {
	switch(p->type) {
	case T_OBJECT:
	    p->u.ob->extra_ref++;
	    continue;
	case T_POINTER:
	    count_ref_in_vector(&p->u.vec->item[0], p->u.vec->size);
	    p->u.vec->extra_ref++;
	    continue;
	}
    }
}

/*
 * Clear the extra debug ref count for vectors
 */
void clear_vector_refs(svp, num)
    struct svalue *svp;
    int num;
{
    struct svalue *p;

    for (p = svp; p < svp+num; p++) {
	switch(p->type) {
	case T_POINTER:
	    clear_vector_refs(&p->u.vec->item[0], p->u.vec->size);
	    p->u.vec->extra_ref = 0;
	    continue;
	}
    }
}

/*
 * Loop through every object and variable in the game and check
 * all reference counts. This will surely take some time, and should
 * only be used for debugging.
 */
void check_a_lot_ref_counts(search_prog)
    struct program *search_prog;
{
    extern struct object *master_ob;
    struct object *ob;

    /*
     * Pass 1: clear the ref counts.
     */
    for (ob=obj_list; ob; ob = ob->next_all) {
	ob->extra_ref = 0;
	ob->prog->extra_ref = 0;
	clear_vector_refs(ob->variables, ob->prog->num_variables);
    }
    clear_vector_refs(start_of_stack, sp - start_of_stack + 1);

    /*
     * Pass 2: Compute the ref counts.
     */

    /*
     * List of all objects.
     */
    for (ob=obj_list; ob; ob = ob->next_all) {
	ob->extra_ref++;
	count_ref_in_vector(ob->variables, ob->prog->num_variables);
	ob->prog->extra_ref++;
	if (ob->prog == search_prog)
	    printf("Found program for object %s\n", ob->name);
	/* Clones will not add to the ref count of inherited progs */
	if (ob->prog->extra_ref == 1)
	    count_inherits(ob->prog, search_prog);
    }

    /*
     * The current stack.
     */
    count_ref_in_vector(start_of_stack, sp - start_of_stack + 1);
    update_ref_counts_for_players();
    count_ref_from_call_outs();
    if (master_ob) master_ob->extra_ref++; /* marion */

    if (search_prog)
	return;

    /*
     * Pass 3: Check the ref counts.
     */
    for (ob=obj_list; ob; ob = ob->next_all) {
	if (ob->ref != ob->extra_ref)
 	    fatal("Bad ref count in object %s, %d - %d\n", ob->name,
		  ob->ref, ob->extra_ref);
	if (ob->prog->ref != ob->prog->extra_ref) {
	    check_a_lot_ref_counts(ob->prog);
	    fatal("Bad ref count in prog %s, %d - %d\n", ob->prog->name,
		  ob->prog->ref, ob->prog->extra_ref);
	}
    }
}

#endif /* DEBUG */

/* Generate a debug message to the player */
static void
do_trace(msg, fname, post)
char *msg, *fname, *post;
{
    char buf[10000];
    char *objname;

    if (!TRACEHB)
	return;
    objname = TRACETST(TRACE_OBJNAME) ? (current_object && current_object->name ? current_object->name : "??")  : "";
    sprintf(buf, "*** %d %*s %s %s %s%s", tracedepth, tracedepth, "", msg, objname, fname, post);
    add_message(buf);
}

struct svalue *apply_master_ob(fun, num_arg)
    char *fun;
    int num_arg;
{
    extern struct object *master_ob;

    assert_master_ob_loaded(fun);
    /*
     * Maybe apply() should be called instead ?
     */
    return sapply(fun, master_ob, num_arg);
}

void assert_master_ob_loaded(fail_reason)
char *fail_reason;
{
    extern struct object *master_ob;
    static int inside = 0;
    struct svalue *ret;

    if (master_ob == 0 || master_ob->flags & O_DESTRUCTED) {
	/*
	 * The master object has been destructed. Free our reference,
	 * and load a new one.
	 *
	 * This test is needed because the master object is called from
	 * yyparse() at an error to find the wizard name. However, and error
	 * when loading the master object will cause a recursive call to this
	 * point.
	 *
	 * The best solution would be if the yyparse() did not have to call
	 * the master object to find the name of the wizard.
	 */
	if (inside) {
	    fprintf(stderr, "Failed to load master object.\n");
	    add_message("Failed to load master file (%s)!\n",
                        fail_reason ? fail_reason : "unknown reason");
	    exit(1);
	}
	fprintf(stderr, "assert_master_ob_loaded: Reloading master.c\n");
	if (master_ob)
	    free_object(master_ob, "assert_master_ob_loaded");
	/*
	 * Clear the pointer, in case the load failed.
	 */
	master_ob = 0;
	inside = 1;
	master_ob = load_object(master_file_name, 0); /* LPCA */

	ret = apply_master_ob("get_root_uid", 0);
	if (ret == 0 || ret->type != T_STRING) {
           fatal ("get_root_uid() in master object does not work\n");
	}
	master_ob->user = add_name(ret->u.string);
	master_ob->eff_user = master_ob->user;
	master_ob->flags |= O_PRIVILEGED; /* LPCA */
	inside = 0;
	add_ref(master_ob, "assert_master_ob_loaded");
	fprintf(stderr, "Reloading done.\n");
    }
}

/*
 * When an object is destructed, all references to it must be removed
 * from the stack.
 */
void remove_object_from_stack(ob)
    struct object *ob;
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

static int
strpref(p, s)
char *p, *s;
{
    while (*p)
	if (*p++ != *s++)
	    return 0;
    return 1;
}
