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

typedef struct pending_call_s {
    int delta;
    union string_or_func function;
    object_t *ob;
    svalue_t v;
    array_t *vs;
    struct pending_call_s *next;
#ifdef THIS_PLAYER_IN_CALL_OUT
    object_t *command_giver;
#endif
} pending_call_t;

static pending_call_t *call_list, *call_list_free;
static int num_call;

static void free_call PROT((pending_call_t *));
static void free_called_call PROT((pending_call_t *));
void remove_all_call_out PROT((object_t *));

/*
 * Free a call out structure.
 */
static void free_called_call P1(pending_call_t *, cop)
{
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

static void free_call P1(pending_call_t *, cop)
{
    if (cop->vs) {
	free_array(cop->vs);
    }
    free_svalue(&cop->v, "free_call");
    free_called_call(cop);
}

/*
 * Setup a new call out.
 */
void new_call_out P5(object_t *, ob, svalue_t *, fun, int, delay, int, num_args, svalue_t *, arg)
{
    pending_call_t *cop, **copp;

    if (delay < 1)
	delay = 1;
    if (!call_list_free) {
	int i;

	call_list_free = CALLOCATE(CHUNK_SIZE, pending_call_t,
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
	fun->u.fp->hdr.ref++;
	cop->ob = 0;
    }
#ifdef THIS_PLAYER_IN_CALL_OUT
    cop->command_giver = command_giver;	/* save current user context */
    if (command_giver)
	add_ref(command_giver, "new_call_out");	/* Bump its ref */
#endif
    if (arg) {
	assign_svalue_no_free(&cop->v, arg);
    } else {
	cop->v = const0;
    }
    if (num_args > 0) {
	int j;

	cop->vs = allocate_empty_array(num_args);
	for (j = 0; j < num_args; j++) {
	    assign_svalue_no_free(&cop->vs->item[j], &arg[j + 1]);
	}
    } else
	cop->vs = 0;
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
    pending_call_t *cop;
    static int last_time;
    object_t *save_command_giver = command_giver;
    error_context_t econ;

    current_interactive = 0;
    if (call_list == 0) {
	last_time = current_time;
	return;
    }
    if (last_time == 0)
	last_time = current_time;

    call_list->delta -= current_time - last_time;
    last_time = current_time;
    save_context(&econ);
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
	if (cop->ob && (cop->ob->flags & O_DESTRUCTED)) {
	    free_call(cop);
	} else {
	    if (SETJMP(econ.context)) {
		restore_context(&econ);
	    } else {
		object_t *ob;

		ob = cop->ob;
#ifndef NO_SHADOWS
		if (ob)
		    while (ob->shadowing)
			ob = ob->shadowing;
#endif				/* NO_SHADOWS */
		command_giver = 0;
#ifdef THIS_PLAYER_IN_CALL_OUT
		if (cop->command_giver &&
		    !(cop->command_giver->flags & O_DESTRUCTED)) {
		    command_giver = cop->command_giver;
		} else if (ob && (ob->flags & O_LISTENER)) {
		    command_giver = ob;
		}
#endif
		/* current object no longer set */

		*(++sp) = cop->v;
		if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
		    put_number(0);
		}

		if (cop->vs) {
		    array_t *vec = cop->vs;
		    svalue_t *svp = vec->item + vec->size;

		    while (svp-- > vec->item) {
			if (svp->type == T_OBJECT && 
			    (svp->u.ob->flags & O_DESTRUCTED)) {
			    free_object(svp->u.ob, "call_out");
			    *svp = const0;
			}
		    }
		    /* cop->vs is ref one */
		    transfer_push_some_svalues(cop->vs->item, cop->vs->size);
		}
		if (cop->ob) {
		    (void) apply(cop->function.s, cop->ob, 
				 1 + (cop->vs ? cop->vs->size : 0),
				 ORIGIN_CALL_OUT);
		} else {
		    (void) call_function_pointer(cop->function.f, 1 + (cop->vs ? cop->vs->size : 0));
		}
	    }
	    free_called_call(cop);
	}
    }
    pop_context(&econ);
    command_giver = save_command_giver;
}

/*
 * Throw away a call out. First call to this function is discarded.
 * The time left until execution is returned.
 * -1 is returned if no call out pending.
 */
int remove_call_out P2(object_t *, ob, char *, fun)
{
    pending_call_t **copp, *cop;
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

int find_call_out P2(object_t *, ob, char *, fun)
{
    pending_call_t **copp;
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

int print_call_out_usage P2(outbuffer_t *, ob, int, verbose)
{
    int i;
    pending_call_t *cop;

    for (i = 0, cop = call_list; cop; cop = cop->next)
	i++;

    if (verbose == 1) {
	outbuf_add(ob, "Call out information:\n");
	outbuf_add(ob, "---------------------\n");
	outbuf_addv(ob, "Number of allocated call outs: %8d, %8d bytes\n",
		    num_call, num_call * sizeof(pending_call_t));
	outbuf_addv(ob, "Current length: %d\n", i);
    } else {
	if (verbose != -1)
	    outbuf_addv(ob, "call out:\t\t\t%8d %8d (current length %d)\n", num_call,
			num_call * sizeof(pending_call_t), i);
    }
    return (int) (num_call * sizeof(pending_call_t));
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_call_outs()
{
    pending_call_t *cop;

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
array_t *get_all_call_outs()
{
    int i, next_time;
    pending_call_t *cop;
    array_t *v;

    for (i = 0, cop = call_list; cop; cop = cop->next)
	if (!cop->ob || !(cop->ob->flags & O_DESTRUCTED))
	    i++;

    v = allocate_empty_array(i);
    next_time = 0;

    for (i = 0, cop = call_list; cop; cop = cop->next) {
	array_t *vv;

	next_time += cop->delta;
	if (cop->ob && (cop->ob->flags & O_DESTRUCTED))
	    continue;
	vv = allocate_empty_array(4);
	if (cop->ob) {
	    vv->item[0].type = T_OBJECT;
	    vv->item[0].u.ob = cop->ob;
	    add_ref(cop->ob, "get_all_call_outs");
	    vv->item[1].type = T_STRING;
	    vv->item[1].subtype = STRING_SHARED;
	    vv->item[1].u.string = make_shared_string(cop->function.s);
	} else {
	    vv->item[0].type = T_OBJECT;
	    vv->item[0].u.ob = cop->function.f->hdr.owner;
	    add_ref(cop->function.f->hdr.owner, "get_all_call_outs");
	    vv->item[1].type = T_STRING;
	    vv->item[1].subtype = STRING_SHARED;
	    vv->item[1].u.string = make_shared_string("<function>");
	}
	vv->item[2].u.number = next_time;
	vv->item[2].type = T_NUMBER;
	assign_svalue_no_free(&vv->item[3], &cop->v);

	v->item[i].type = T_ARRAY;
	v->item[i++].u.arr = vv;	/* Ref count is already 1 */
    }
    return v;
}

void
remove_all_call_out P1(object_t *, obj)
{
    pending_call_t **copp, *cop;

    copp = &call_list;
    while (*copp) {
	if ( ((*copp)->ob &&
	      (((*copp)->ob == obj) || ((*copp)->ob->flags & O_DESTRUCTED))) ||
	    (!(*copp)->ob &&
	     ((*copp)->function.f->hdr.owner == obj ||
	      (*copp)->function.f->hdr.owner->flags & O_DESTRUCTED)) )
	  {
	    cop = *copp;
	    if (cop->next)
		cop->next->delta += cop->delta;
	    *copp = cop->next;
	    free_call(cop);
	} else
	    copp = &(*copp)->next;
    }
}
