#include "std.h"
#include "lpc_incl.h"
#include "backend.h"
#include "comm.h"
#include "call_out.h"
#include "eoperators.h"

/*
 * This file implements delayed calls of functions.
 * Static functions can not be called this way.
 *
 * Allocate the structures several in one chunk, to get rid of malloc
 * overhead.
 */

#define CHUNK_SIZE	20

struct call {
    int delta;
    union string_or_func function;
    struct object *ob;
    struct svalue v;
    struct vector *vs;
    struct call *next;
#ifdef THIS_PLAYER_IN_CALL_OUT
    struct object *command_giver;
#endif
};

static struct call *call_list, *call_list_free;
static int num_call;

static void free_call PROT((struct call *));
void remove_all_call_out PROT((struct object *));

/*
 * Free a call out structure.
 */
static void free_call P1(struct call *, cop)
{
    if (cop->vs) {
	free_vector(cop->vs);
    }
    free_svalue(&cop->v, "free_call");
    cop->v = const0n;
    cop->next = call_list_free;
    if (cop->ob) {
	free_string(cop->function.s);
	free_object(cop->ob, "free_call");
    } else {
	free_funp(cop->function.f);
    }
    cop->function.s = 0;
#ifdef THIS_PLAYER_IN_CALL_OUT
    if (cop->command_giver)
	free_object(cop->command_giver, "free_call");
#endif
    cop->ob = 0;
    call_list_free = cop;
}

/*
 * Setup a new call out.
 */
void new_call_out P5(struct object *, ob, struct svalue *, fun, int, delay, int, num_args, struct svalue *, arg)
{
    struct call *cop, **copp;

    if (delay < 1)
	delay = 1;
    if (!call_list_free) {
	int i;

	call_list_free = CALLOCATE(CHUNK_SIZE, struct call,
				   TAG_CALL_OUT, "new_call_out: call_list_free");
	for (i = 0; i < CHUNK_SIZE - 1; i++)
	    call_list_free[i].next = &call_list_free[i + 1];
	call_list_free[CHUNK_SIZE - 1].next = 0;
	num_call += CHUNK_SIZE;
    }
    cop = call_list_free;
    call_list_free = call_list_free->next;

    if (fun->type == T_STRING) {
	cop->function.s = make_shared_string(fun->u.string);
	cop->ob = ob;
	add_ref(ob, "call_out");
    } else {
	cop->function.f = fun->u.fp;
	fun->u.fp->ref++;
	cop->ob = 0;
    }
#ifdef THIS_PLAYER_IN_CALL_OUT
    cop->command_giver = command_giver;	/* save current user context */
    if (command_giver)
	add_ref(command_giver, "new_call_out");	/* Bump its ref */
#endif
    cop->v.type = T_NUMBER;
    cop->v.subtype = 0;
    cop->v.u.number = 0;
    cop->vs = NULL;
    if (arg) {
	assign_svalue(&cop->v, arg);
    }
    if (num_args > 0) {
	int j;

	cop->vs = allocate_empty_array(num_args);
	for (j = 0; j < num_args; j++) {
	    assign_svalue_no_free(&cop->vs->item[j], &arg[j + 1]);
	}
    }
    for (copp = &call_list; *copp; copp = &(*copp)->next) {
	if ((*copp)->delta >= delay) {
	    (*copp)->delta -= delay;
	    cop->delta = delay;
	    cop->next = *copp;
	    *copp = cop;
	    return;
	}
	delay -= (*copp)->delta;
    }
    *copp = cop;
    cop->delta = delay;
    cop->next = 0;
}

/*
 * See if there are any call outs to be called. Set the 'command_giver'
 * if it is a living object. Check for shadowing objects, which may also
 * be living objects.
 */
void call_out()
{
    struct call *cop;
    jmp_buf save_error_recovery_context;
    int save_rec_exists;
    struct object *save_command_giver;
    static int last_time;

    if (call_list == 0) {
	last_time = current_time;
	return;
    }
    if (last_time == 0)
	last_time = current_time;
    save_command_giver = command_giver;
    current_interactive = 0;
    call_list->delta -= current_time - last_time;
    last_time = current_time;
    memcpy((char *) save_error_recovery_context,
	   (char *) error_recovery_context, sizeof error_recovery_context);
    save_rec_exists = error_recovery_context_exists;
    error_recovery_context_exists = NORMAL_ERROR_CONTEXT;
    while (call_list && call_list->delta <= 0) {
	/*
	 * Move the first call_out out of the chain.
	 */
	cop = call_list;
	call_list = call_list->next;
	/*
	 * A special case: If a lot of time has passed, so that current call
	 * out was missed, then it will have a negative delta. This negative
	 * delta implies that the next call out in the list has to be
	 * adjusted.
	 */
	if (call_list && cop->delta < 0)
	    call_list->delta += cop->delta;
	if (!(cop->ob && cop->ob->flags & O_DESTRUCTED)) {
	    if (SETJMP(error_recovery_context)) {
		clear_state();
		debug_message("Error in call out.\n");
	    } else {
		struct svalue v;
		struct object *ob;

		ob = cop->ob;
#ifndef NO_SHADOWS
		while (ob->shadowing)
		    ob = ob->shadowing;
#endif				/* NO_SHADOWS */
		command_giver = 0;
#ifdef THIS_PLAYER_IN_CALL_OUT
		if (cop->command_giver &&
		    !(cop->command_giver->flags & O_DESTRUCTED)) {
		    command_giver = cop->command_giver;
		} else if (ob->flags & O_LISTENER) {
		    command_giver = ob;
		}
#endif
		v.type = cop->v.type;
		v.subtype = 0;
		v.u = cop->v.u;
		v.subtype = cop->v.subtype;	/* Not always used */
		if (v.type == T_OBJECT && (v.u.ob->flags & O_DESTRUCTED)) {
		    v.type = T_NUMBER;
		    v.subtype = 0;
		    v.u.number = 0;
		}
		/* current object no longer set */
		push_svalue(&v);
		if (cop->vs) {
		    int j;

		    check_for_destr(cop->vs);
		    for (j = 0; j < cop->vs->size; j++) {
			push_svalue(&cop->vs->item[j]);
		    }
		}
		if (cop->ob) {
		    (void) apply(cop->function.s, cop->ob, 
				 1 + (cop->vs ? cop->vs->size : 0),
				 ORIGIN_CALL_OUT);
		} else {
		    (void) call_function_pointer(cop->function.f, 1 + (cop->vs ? cop->vs->size : 0));
		}
	    }
	}
	free_call(cop);
    }
    memcpy((char *) error_recovery_context,
	   (char *) save_error_recovery_context,
	   sizeof error_recovery_context);
    error_recovery_context_exists = save_rec_exists;
    command_giver = save_command_giver;
}

/*
 * Throw away a call out. First call to this function is discarded.
 * The time left until execution is returned.
 * -1 is returned if no call out pending.
 */
int remove_call_out P2(struct object *, ob, char *, fun)
{
    struct call **copp, *cop;
    int delay = 0;

    if (!ob) return -1;
    for (copp = &call_list; *copp; copp = &(*copp)->next) {
	delay += (*copp)->delta;
	if ((*copp)->ob == ob && strcmp((*copp)->function.s, fun) == 0) {
	    cop = *copp;
	    if (cop->next)
		cop->next->delta += cop->delta;
	    *copp = cop->next;
	    free_call(cop);
	    return delay;
	}
    }
    return -1;
}

int find_call_out P2(struct object *, ob, char *, fun)
{
    struct call **copp;
    int delay = 0;

    if (!ob) return -1;
    for (copp = &call_list; *copp; copp = &(*copp)->next) {
	delay += (*copp)->delta;
	if ((*copp)->ob == ob && strcmp((*copp)->function.s, fun) == 0) {
	    return delay;
	}
    }
    return -1;
}

int print_call_out_usage P1(int, verbose)
{
    int i;
    struct call *cop;

    for (i = 0, cop = call_list; cop; cop = cop->next)
	i++;

    if (verbose == 1) {
	add_message("Call out information:\n");
	add_message("---------------------\n");
	add_vmessage("Number of allocated call outs: %8d, %8d bytes\n",
		    num_call, num_call * sizeof(struct call));
	add_vmessage("Current length: %d\n", i);
    } else {
	if (verbose != -1)
	    add_vmessage("call out:\t\t\t%8d %8d (current length %d)\n", num_call,
			num_call * sizeof(struct call), i);
    }
    return (int) (num_call * sizeof(struct call));
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_call_outs()
{
    struct call *cop;

    for (cop = call_list; cop; cop = cop->next) {
	mark_svalue(&cop->v);
	if (cop->ob) {
	    cop->ob->extra_ref++;
	    EXTRA_REF(BLOCK(cop->function.s))++;
	}
#ifdef THIS_PLAYER_IN_CALL_OUT
    if (cop->command_giver)
	cop->command_giver->extra_ref++;
#endif
    }
}
#endif

/*
 * Construct an array of all pending call_outs. Every item in the array
 * consists of 4 items (but only if the object not is destructed):
 * 0:	The object.
 * 1:	The function (string).
 * 2:	The delay.
 * 3:	The argument.
 */
struct vector *get_all_call_outs()
{
    int i, next_time;
    struct call *cop;
    struct vector *v;

/* Zap all of the dested ones from the array... */
    remove_all_call_out((struct object *) NULL);
    for (i = 0, cop = call_list; cop; i++, cop = cop->next)
	;
    v = allocate_empty_array(i);
    next_time = 0;

    for (i = 0, cop = call_list; cop; i++, cop = cop->next) {
	struct vector *vv;

	next_time += cop->delta;
	if (cop->ob && cop->ob->flags & O_DESTRUCTED)
	    continue;  /* This should be smarter.  -Beek */
	vv = allocate_empty_array(4);
#ifdef NEW_FUNCTIONS
	if (cop->ob) {
#endif
	    vv->item[0].type = T_OBJECT;
	    vv->item[0].u.ob = cop->ob;
	    add_ref(cop->ob, "get_all_call_outs");
	    vv->item[1].type = T_STRING;
	    vv->item[1].subtype = STRING_SHARED;
	    vv->item[1].u.string = make_shared_string(cop->function.s);
#ifdef NEW_FUNCTIONS
	} else {
	    vv->item[0].type = T_OBJECT;
	    vv->item[0].u.ob = cop->function.f->owner;
	    add_ref(cop->function.f->owner, "get_all_call_outs");
	    vv->item[1].type = T_STRING;
	    vv->item[1].subtype = STRING_SHARED;
	    vv->item[1].u.string = make_shared_string("<function>");
	}
#endif
	vv->item[2].u.number = next_time;
	vv->item[2].type = T_NUMBER;
	assign_svalue_no_free(&vv->item[3], &cop->v);

	v->item[i].type = T_POINTER;
	v->item[i].u.vec = vv;	/* Ref count is already 1 */
    }
    return v;
}

void
remove_all_call_out P1(struct object *, obj)
{
    struct call **copp, *cop;

    copp = &call_list;
    while (*copp) {
	if (((*copp)->ob == obj) || ((*copp)->ob->flags & O_DESTRUCTED)) {
	    cop = *copp;
	    if (cop->next)
		cop->next->delta += cop->delta;
	    *copp = cop->next;
	    free_call(cop);
	} else
	    copp = &(*copp)->next;
    }
}
