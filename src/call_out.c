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
    array_t *vs;
    struct pending_call_s *next;
#ifdef THIS_PLAYER_IN_CALL_OUT
    object_t *command_giver;
#endif
#ifdef CALLOUT_HANDLES
    int handle;
#endif
} pending_call_t;

static pending_call_t *call_list[CALLOUT_CYCLE_SIZE];
static pending_call_t *call_list_free;
static int num_call, call_out_time = 0;
#ifdef CALLOUT_HANDLES
static int unique = 0;
#endif

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

INLINE_STATIC void free_call P1(pending_call_t *, cop)
{
    if (cop->vs)
	free_array(cop->vs);
    free_called_call(cop);
}

/*
 * Setup a new call out.
 */
#ifdef CALLOUT_HANDLES
int
#else
void
#endif
new_call_out P5(object_t *, ob, svalue_t *, fun, int, delay, 
		int, num_args, svalue_t *, arg)
{
    pending_call_t *cop, **copp;
    int tm;

    if (delay < 1)
	delay = 1;
    /* Needs to be initialized here in case of very early call_outs */
    if (!call_out_time)
	call_out_time = current_time;
    
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
    if (num_args > 0) {
	cop->vs = allocate_empty_array(num_args);
	memcpy(cop->vs->item, arg, sizeof(svalue_t) * num_args);
    } else
	cop->vs = 0;

    /* Find out which slot this one fits in */
    tm = (delay + current_time) & (CALLOUT_CYCLE_SIZE - 1);
    delay = (1 + (delay + current_time - call_out_time - 1) / CALLOUT_CYCLE_SIZE);

    for (copp = &call_list[tm]; *copp; copp = &(*copp)->next) {
	if ((*copp)->delta >= delay) {
	    (*copp)->delta -= delay;
	    cop->delta = delay;
	    cop->next = *copp;
	    *copp = cop;
#ifdef CALLOUT_HANDLES
	    tm += CALLOUT_CYCLE_SIZE * ++unique;
	    cop->handle = tm;
	    return tm;
#else
	    return;
#endif
	}
	delay -= (*copp)->delta;
    }
    *copp = cop;
    cop->delta = delay;
    cop->next = 0;
#ifdef CALLOUT_HANDLES
    tm += CALLOUT_CYCLE_SIZE * ++unique;
    cop->handle = tm;
    return tm;
#endif
}

/*
 * See if there are any call outs to be called. Set the 'command_giver'
 * if it is a living object. Check for shadowing objects, which may also
 * be living objects.
 */
void call_out()
{
    int extra;
    static pending_call_t *cop = 0;
    object_t *save_command_giver = command_giver;
    error_context_t econ;
    int tm;
    
    current_interactive = 0;

    /* could be still allocated if an error occured during a call_out */
    if (cop) {
	free_called_call(cop);
	cop = 0;
    }
    if (!call_out_time)
	call_out_time = current_time;
    save_context(&econ);
    while (call_out_time < current_time) {
	/* we increment at the end in case we are interrupted by errors,
	   but we need to use call_out_time + 1 here. */
	tm = (call_out_time+1) & (CALLOUT_CYCLE_SIZE - 1);
	if (call_list[tm] && --call_list[tm]->delta == 0)
	    do {
		/*
		 * Move the first call_out out of the chain.
		 */
		cop = call_list[tm];
		call_list[tm] = call_list[tm]->next;
		if (cop->ob && (cop->ob->flags & O_DESTRUCTED)) {
		    free_call(cop);
		    cop = 0;
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
#endif
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
			    extra = cop->vs->size;
			    transfer_push_some_svalues(cop->vs->item, extra);
			    free_empty_array(cop->vs);
			} else
			    extra = 0;
			
			if (cop->ob) {
			    if (cop->function.s[0] == APPLY___INIT_SPECIAL_CHAR)
				error("Illegal function name\n");
    
			    (void) apply(cop->function.s, cop->ob, extra,
					 ORIGIN_CALL_OUT);
			} else {
			    (void) call_function_pointer(cop->function.f, extra);
			}
		    }
		    free_called_call(cop);
		    cop = 0;
		}
	    } while (call_list[tm] && call_list[tm]->delta == 0);
	call_out_time++;
    }
    pop_context(&econ);
    command_giver = save_command_giver;
}

static int time_left P2(int, slot, int, delay) {
    int current_slot = call_out_time & (CALLOUT_CYCLE_SIZE - 1);
    if (slot > current_slot) {
	return (delay-1) * CALLOUT_CYCLE_SIZE + (slot - current_slot) + call_out_time - current_time;
    } else {
	return delay * CALLOUT_CYCLE_SIZE + (slot - current_slot) + call_out_time - current_time;
    }
}

/*
 * Throw away a call out. First call to this function is discarded.
 * The time left until execution is returned.
 * -1 is returned if no call out pending.
 */
int remove_call_out P2(object_t *, ob, char *, fun)
{
    pending_call_t **copp, *cop;
    int delay;
    int i;
    
    if (!ob) return -1;
    for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
	delay = 0;
	for (copp = &call_list[i]; *copp; copp = &(*copp)->next) {
	    delay += (*copp)->delta;
	    if ((*copp)->ob == ob && strcmp((*copp)->function.s, fun) == 0) {
		cop = *copp;
		if (cop->next)
		    cop->next->delta += cop->delta;
		*copp = cop->next;
		free_call(cop);
		return time_left(i, delay);
	    }
	}
    }
    return -1;
}

#ifdef CALLOUT_HANDLES
int remove_call_out_by_handle P1(int, handle)
{
    pending_call_t **copp, *cop;
    int delay = 0;
    
    for (copp = &call_list[handle & (CALLOUT_CYCLE_SIZE - 1)]; *copp; copp = &(*copp)->next) {
	delay += (*copp)->delta;
	if ((*copp)->handle == handle) {
	    cop = *copp;
	    if (cop->next)
		cop->next->delta += cop->delta;
	    *copp = cop->next;
	    free_call(cop);
	    return time_left(handle & (CALLOUT_CYCLE_SIZE - 1), delay);
	}
    }
    return -1;
}

int find_call_out_by_handle P1(int, handle) 
{
    pending_call_t *cop;
    int delay = 0;
    
    for (cop = call_list[handle & (CALLOUT_CYCLE_SIZE - 1)]; cop; cop = cop->next) {
	delay += cop->delta;
	if (cop->handle == handle) 
	    return time_left(handle & (CALLOUT_CYCLE_SIZE - 1), delay);
    }
    return -1;
}
#endif
	
int find_call_out P2(object_t *, ob, char *, fun)
{
    pending_call_t *cop;
    int delay;
    int i;
    
    if (!ob) return -1;
    for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
	delay = 0;
	for (cop = call_list[i]; cop; cop = cop->next) {
	    delay += cop->delta;
	    if (cop->ob == ob && strcmp(cop->function.s, fun) == 0) 
		return time_left(i, delay);
	}
    }
    return -1;
}

int print_call_out_usage P2(outbuffer_t *, ob, int, verbose)
{
    int i, j;
    pending_call_t *cop;

    for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++)
	for (cop = call_list[j]; cop; cop = cop->next)
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
    int i;
    
    for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
	for (cop = call_list[i]; cop; cop = cop->next) {
	    if (cop->vs)
		cop->vs->extra_ref++;
	    if (cop->ob) {
		cop->ob->extra_ref++;
		EXTRA_REF(BLOCK(cop->function.s))++;
	    } else {
		cop->function.f->hdr.extra_ref++;
	    }
#ifdef THIS_PLAYER_IN_CALL_OUT
	    if (cop->command_giver)
		cop->command_giver->extra_ref++;
#endif
	}
    }
}
#endif

/*
 * Construct an array of all pending call_outs. Every item in the array
 * consists of 3 items (but only if the object not is destructed):
 * 0:	The object.
 * 1:	The function (string).
 * 2:	The delay.
 */
array_t *get_all_call_outs()
{
    int i, j, delay, tm;
    pending_call_t *cop;
    array_t *v;

    for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++)
	for (cop = call_list[j]; cop; cop = cop->next)
	    if (!cop->ob || !(cop->ob->flags & O_DESTRUCTED))
		i++;

    v = allocate_empty_array(i);
    tm = call_out_time & (CALLOUT_CYCLE_SIZE-1);

    for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++) {
	delay = 0;
	for (cop = call_list[j]; cop; cop = cop->next) {
	    array_t *vv;

	    delay += cop->delta;
	    if (cop->ob && (cop->ob->flags & O_DESTRUCTED))
		continue;
	    vv = allocate_empty_array(3);
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
	    vv->item[2].type = T_NUMBER;
	    if (j > tm) {
		vv->item[2].u.number = (delay-1) * CALLOUT_CYCLE_SIZE + (j - tm) + call_out_time - current_time;
	    } else {
		vv->item[2].u.number = delay * CALLOUT_CYCLE_SIZE + (j - tm) + call_out_time - current_time;
	    }
	    
	    v->item[i].type = T_ARRAY;
	    v->item[i++].u.arr = vv;	/* Ref count is already 1 */
	}
    }
    return v;
}

void
remove_all_call_out P1(object_t *, obj)
{
    pending_call_t **copp, *cop;
    int i;
    
    for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
	copp = &call_list[i];
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
}
