/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "backend.h"
#include "comm.h"
#include "replace_program.h"
#include "debug.h"
#include "socket_efuns.h"
#include "swap.h"
#include "call_out.h"
#include "port.h"
#include "lint.h"

#ifdef WIN32
#include <process.h>
void CDECL alarm_loop PROT((void *));
#endif

error_context_t *current_error_context = 0;

/*
 * The 'current_time' is updated at every heart beat.
 */
int current_time;

int heart_beat_flag = 0;

object_t *current_heart_beat;
static void look_for_objects_to_swap PROT((void));
static void call_heart_beat PROT((void));

/*
 * There are global variables that must be zeroed before any execution.
 * In case of errors, there will be a LONGJMP(), and the variables will
 * have to be cleared explicitely. They are normally maintained by the
 * code that use them.
 *
 * This routine must only be called from top level, not from inside
 * stack machine execution (as stack will be cleared).
 */
void clear_state()
{
    current_object = 0;
    command_giver = 0;
    current_interactive = 0;
    previous_ob = 0;
    current_prog = 0;
    caller_type = 0;
    reset_machine(0);		/* Pop down the stack. */
}				/* clear_state() */

#if 0
static void report_holes() {
    if (current_object && current_object->name)
	debug_message("current_object is /%s\n", current_object->name);
    if (command_giver && command_giver->name)
	debug_message("command_giver is /%s\n", command_giver->name);
    if (current_interactive && current_interactive->name)
	debug_message("current_interactive is /%s\n", current_interactive->name);
    if (previous_ob && previous_ob->name)
	debug_message("previous_ob is /%s\n", previous_ob->name);
    if (current_prog && current_prog->name)
	debug_message("current_prog is /%s\n", current_prog->name);
    if (caller_type)
	debug_message("caller_type is %s\n", caller_type);
}
#endif

void logon P1(object_t *, ob)
{
    /* current_object no longer set */
    apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
    /* function not existing is no longer fatal */
}

#ifndef NO_ADD_ACTION
/*
 * Take a user command and parse it.
 * The command can also come from a NPC.
 * Beware that 'str' can be modified and extended !
 */
int parse_command P2(char *, str, object_t *, ob)
{
    object_t *save = command_giver;
    int res;

    /* disallow users to issue commands containing ansi escape codes */
#if defined(NO_ANSI) && !defined(STRIP_BEFORE_PROCESS_INPUT)
    char *c;

    for (c = str; *c; c++) {
	if (*c == 27) {
	    *c = ' ';		/* replace ESC with ' ' */
	}
    }
#endif
    command_giver = ob;
    res = user_parser(str);
    command_giver = save;
    return (res);
}				/* parse_command() */
#endif

/*
 * This is the backend. We will stay here for ever (almost).
 */
int eval_cost;

void backend()
{
    struct timeval timeout;
    int nb;
    int i;
    int there_is_a_port = 0;
    error_context_t econ;

    debug_message("Initializations complete.\n\n");
    for (i = 0; i < 5; i++) {
	if (external_port[i].port) {
	    debug_message("Accepting connections on port %d.\n",
		    external_port[i].port);
	    there_is_a_port = 1;
	}
    }

    if (!there_is_a_port)
	debug_message("No external ports specified.\n");

    init_user_conn();		/* initialize user connection socket */
#ifdef SIGHUP
    signal(SIGHUP, startshutdownMudOS);
#endif
    if (!t_flag)
	call_heart_beat();
    clear_state();
    save_context(&econ);
    if (SETJMP(econ.context))
	restore_context(&econ);

    while (1) {	
	/* Has to be cleared if we jumped out of process_user_command() */
	current_interactive = 0;
	eval_cost = max_cost;

	if (obj_list_replace || obj_list_destruct)
	    remove_destructed_objects();

	/*
	 * shut down MudOS if MudOS_is_being_shut_down is set.
	 */
	if (MudOS_is_being_shut_down)
	    shutdownMudOS(0);
	if (slow_shut_down_to_do) {
	    int tmp = slow_shut_down_to_do;

	    slow_shut_down_to_do = 0;
	    slow_shut_down(tmp);
	}
	/*
	 * select
	 */
	make_selectmasks();
	if (heart_beat_flag) {	/* use zero timeout if a heartbeat is
				 * pending. */
	    timeout.tv_sec = 0;	/* this should avoid problems with longjmp's
				 * too */
	    timeout.tv_usec = 0;
	} else {
	    /*
	     * not using infinite timeout so that we'll have insurance in the
	     * unlikely event a heartbeat happens between now and the
	     * select(). Note that SIGALRMs (for heartbeats) do make select()
	     * drop through. (Except on Windows)
	     */
#ifdef WIN32
	    timeout.tv_sec = HEARTBEAT_INTERVAL/1000000;
	    timeout.tv_usec = HEARTBEAT_INTERVAL%1000000;
#else
	    timeout.tv_sec = 60;
	    timeout.tv_usec = 0;
#endif
	}
#ifndef hpux
	nb = select(FD_SETSIZE, &readmask, &writemask, (fd_set *) 0, &timeout);
#else
	nb = select(FD_SETSIZE, (int *) &readmask, (int *) &writemask,
		    (int *) 0, &timeout);
#endif
	/*
	 * process I/O if necessary.
	 */
	if (nb > 0) {
	    process_io();
	}
	/*
	 * process user commands.
	 */
	for (i = 0; process_user_command() && i < max_users; i++)
	    ;

	/*
	 * call heartbeat if appropriate.
	 */
	if (heart_beat_flag) {
	    debug(512, ("backend: HEARTBEAT\n"));
	    call_heart_beat();
	}
    }
}				/* backend() */

/*
 * Despite the name, this routine takes care of several things.
 * It will loop through all objects once every 10 minutes.
 *
 * If an object is found in a state of not having done reset, and the
 * delay to next reset has passed, then reset() will be done.
 *
 * If the object has a existed more than the time limit given for swapping,
 * then 'clean_up' will first be called in the object, after which it will
 * be swapped out if it still exists.
 *
 * There are some problems if the object self-destructs in clean_up, so
 * special care has to be taken of how the linked list is used.
*/
static void look_for_objects_to_swap()
{
    static int next_time;
    object_t *ob;
    VOLATILE object_t *next_ob;
    error_context_t econ;

    if (current_time < next_time)
	return;			/* Not time to look yet */
    next_time = current_time + 15 * 60;	/* Next time is in 15 minutes */

    /*
     * Objects object can be destructed, which means that next object to
     * investigate is saved in next_ob. If very unlucky, that object can be
     * destructed too. In that case, the loop is simply restarted.
     */
    next_ob = obj_list;
    save_context(&econ);
    if (SETJMP(econ.context))
	restore_context(&econ);
    
    while ((ob = (object_t *)next_ob)) {
	int ready_for_swap;

	eval_cost = max_cost;

	if (ob->flags & O_DESTRUCTED)
	    ob = obj_list;	/* restart */
	next_ob = ob->next_all;

	/*
	 * Check reference time before reset() is called.
	 *
	 * FIXME: clean_up has the same problem. -Beek
	 */
	if (current_time < ob->time_of_ref + time_to_swap)
	    ready_for_swap = 0;
	else
	    ready_for_swap = 1;
#ifndef NO_RESETS
#ifndef LAZY_RESETS
	/*
	 * Should this object have reset(1) called ?
	 */
	if ((ob->flags & O_WILL_RESET) && (ob->next_reset < current_time)
	    && !(ob->flags & O_RESET_STATE)) {
#ifdef DEBUG
	    if (d_flag) {
		debug_message("RESET /%s\n", ob->name);
	    }
#endif
	    reset_object(ob);
	}
#endif
#endif
	if (time_to_clean_up > 0) {
	    /*
	     * Has enough time passed, to give the object a chance to
	     * self-destruct ? Save the O_RESET_STATE, which will be cleared.
	     * 
	     * Only call clean_up in objects that has defined such a function.
	     * 
	     * Only if the clean_up returns a non-zero value, will it be called
	     * again.
	     */

	    if (current_time - ob->time_of_ref > time_to_clean_up
		&& (ob->flags & O_WILL_CLEAN_UP)) {
		int save_reset_state = ob->flags & O_RESET_STATE;
		svalue_t *svp;

#ifdef DEBUG
		if (d_flag)
		    debug_message("clean up /%s\n", ob->name);
#endif
		/*
		 * Supply a flag to the object that says if this program is
		 * inherited by other objects. Cloned objects might as well
		 * believe they are not inherited. Swapped objects will not
		 * have a ref count > 1 (and will have an invalid ob->prog
		 * pointer).
		 */

		push_number(ob->flags & (O_CLONE | O_SWAPPED) ? 0 : ob->prog->ref);
		svp = apply(APPLY_CLEAN_UP, ob, 1, ORIGIN_DRIVER);
		if (ob->flags & O_DESTRUCTED)
		    continue;
		if (!svp || (svp->type == T_NUMBER && svp->u.number == 0))
		    ob->flags &= ~O_WILL_CLEAN_UP;
		ob->flags |= save_reset_state;
	    }
	}
	if (time_to_swap > 0) {
	    /*
	     * At last, there is a possibility that the object can be swapped
	     * out.
	     */

	    if (ob->prog && ob->prog->line_info)
		swap_line_numbers(ob->prog);
	    if (ob->flags & O_SWAPPED || !ready_for_swap)
		continue;
	    if (ob->flags & O_HEART_BEAT)
		continue;
#ifdef DEBUG
	    if (d_flag)
		debug_message("swap /%s\n", ob->name);
#endif
	    swap(ob);		/* See if it is possible to swap out to disk */
	}
    }
    pop_context(&econ);
}				/* look_for_objects_to_swap() */

/* Call all heart_beat() functions in all objects.  Also call the next reset,
 * and the call out.
 * We do heart beats by moving each object done to the end of the heart beat
 * list before we call its function, and always using the item at the head
 * of the list as our function to call.  We keep calling heart beats until
 * a timeout or we have done num_heart_objs calls.  It is done this way so
 * that objects can delete heart beating objects from the list from within
 * their heart beat without truncating the current round of heart beats.
 *
 * Set command_giver to current_object if it is a living object. If the object
 * is shadowed, check the shadowed object if living. There is no need to save
 * the value of the command_giver, as the caller resets it to 0 anyway.  */

typedef struct {
    object_t *ob;
    short heart_beat_ticks;
    short time_to_heart_beat;
} heart_beat_t;

static heart_beat_t *heart_beats = 0;
static int max_heart_beats = 0;
static int heart_beat_index = 0;
static int num_hb_objs = 0;
static int num_hb_to_do = 0;

static int num_hb_calls = 0;	/* starts */
static float perc_hb_probes = 100.0;	/* decaying avge of how many complete */

#ifdef WIN32
void CDECL alarm_loop P1(void *, ignore)
{
    while (1) {
	Sleep(HEARTBEAT_INTERVAL / 1000);
	heart_beat_flag = 1;
    }
}				/* alarm_loop() */
#endif

static void call_heart_beat()
{
    object_t *ob;
    heart_beat_t *curr_hb;

#ifdef WIN32
    static long Win32Thread = -1;
#endif

    heart_beat_flag = 0;
#ifdef SIGALRM
    signal(SIGALRM, sigalrm_handler);
#endif

#ifdef HAS_UALARM
    ualarm(HEARTBEAT_INTERVAL, 0);
#else
#  ifdef WIN32
    if (Win32Thread == -1) Win32Thread = _beginthread(alarm_loop, 256, 0);
#  else
    alarm(SYSV_HEARTBEAT_INTERVAL);	/* defined in config.h */
#  endif
#endif

    debug(256, ("."));

    current_time = get_current_time();
    current_interactive = 0;

    if ((num_hb_to_do = num_hb_objs)) {
	num_hb_calls++;
	heart_beat_index = 0;
	while (!heart_beat_flag) {
	    ob = (curr_hb = &heart_beats[heart_beat_index])->ob;
	    DEBUG_CHECK(!(ob->flags & O_HEART_BEAT),
			"Heartbeat not set in object on heartbeat list!");
	    DEBUG_CHECK(ob->flags & O_SWAPPED,
			"Heartbeat in swapped object.\n");
	    /* is it time to do a heart beat ? */
	    curr_hb->heart_beat_ticks--;

	    if (ob->prog->heart_beat != -1) {
		if (curr_hb->heart_beat_ticks < 1) {
		    curr_hb->heart_beat_ticks = curr_hb->time_to_heart_beat;
		    current_heart_beat = ob;
		    command_giver = ob;
#ifndef NO_SHADOWS
		    while (command_giver->shadowing)
			command_giver = command_giver->shadowing;
#endif
#ifndef NO_ADD_ACTION
		    if (!(command_giver->flags & O_ENABLE_COMMANDS))
			command_giver = 0;
#endif
#ifdef PACKAGE_MUDLIB_STATS
		    add_heart_beats(&ob->stats, 1);
#endif
		    eval_cost = max_cost;
		    /* this should be looked at ... */
		    call_function(ob->prog, ob->prog->heart_beat);
		    command_giver = 0;
		    current_object = 0;
		}
	    }
	    if (++heart_beat_index == num_hb_to_do)
		break;
	}
	if (heart_beat_index < num_hb_to_do)
	    perc_hb_probes = 100 * (float) heart_beat_index / num_hb_to_do;
	else
	    perc_hb_probes = 100.0;
	heart_beat_index = num_hb_to_do = 0;
    }
    current_prog = 0;
    current_heart_beat = 0;
    look_for_objects_to_swap();
    call_out();
#ifdef PACKAGE_MUDLIB_STATS
    mudlib_stats_decay();
#endif
}				/* call_heart_beat() */

int
query_heart_beat P1(object_t *, ob)
{
    int index;
    
    if (!(ob->flags & O_HEART_BEAT))  return 0;
    index = num_hb_objs;
    while (index--) {
	if (heart_beats[index].ob == ob) 
	    return heart_beats[index].time_to_heart_beat;
    }
    return 0;
}				/* query_heart_beat() */

/* add or remove an object from the heart beat list; does the major check...
 * If an object removes something from the list from within a heart beat,
 * various pointers in call_heart_beat could be stuffed, so we must
 * check current_heart_beat and adjust pointers.  */

int set_heart_beat P2(object_t *, ob, int, to)
{
    int index;
    
    if (ob->flags & O_DESTRUCTED) return 0;

    if (!to) {
	int num;
	
	index = num_hb_objs;
	while (index--) {
	    if (heart_beats[index].ob == ob) break;
	}
	if (index < 0) return 0;

	if (num_hb_to_do) {
	    if (index <= heart_beat_index)
		heart_beat_index--;
	    if (index < num_hb_to_do)
		num_hb_to_do--;
	}
	
	if ((num = (num_hb_objs - (index + 1))))
	    memmove(heart_beats + index, heart_beats + (index + 1), num * sizeof(heart_beat_t));
	
	num_hb_objs--;
	ob->flags &= ~O_HEART_BEAT;
	return 1;
    }
    
    if (ob->flags & O_HEART_BEAT) {
	if (to < 0) return 0;
	
	index = num_hb_objs;
	while (index--) {
	    if (heart_beats[index].ob == ob) {
		heart_beats[index].time_to_heart_beat = heart_beats[index].heart_beat_ticks = to;
		break;
	    }
	}
	DEBUG_CHECK(index < 0, "Couldn't find enabled object in heart_beat list!\n");
    } else {
	heart_beat_t *hb;
	
	if (!max_heart_beats)
	    heart_beats = CALLOCATE(max_heart_beats = HEART_BEAT_CHUNK,
				    heart_beat_t, TAG_HEART_BEAT,
				    "set_heart_beat: 1");
	else if (num_hb_objs == max_heart_beats) {
	    max_heart_beats += HEART_BEAT_CHUNK;
	    heart_beats = RESIZE(heart_beats, max_heart_beats,
				 heart_beat_t, TAG_HEART_BEAT,
				 "set_heart_beat: 1");
	}
	
	hb = &heart_beats[num_hb_objs++];
	hb->ob = ob;
	if (to < 0) to = 1;
	hb->time_to_heart_beat = to;
	hb->heart_beat_ticks = to;
	ob->flags |= O_HEART_BEAT;
    }
    
    return 1;
}

int heart_beat_status P2(outbuffer_t *, ob, int, verbose)
{
    char buf[20];

    if (verbose == 1) {
	outbuf_add(ob, "Heart beat information:\n");
	outbuf_add(ob, "-----------------------\n");
	outbuf_addv(ob, "Number of objects with heart beat: %d, starts: %d\n",
		    num_hb_objs, num_hb_calls);
	
	/* passing floats to varargs isn't highly portable so let sprintf
	   handle it */
	sprintf(buf, "%.2f", perc_hb_probes);
	outbuf_addv(ob, "Percentage of HB calls completed last time: %s\n", buf);
    }
    return (0);
}				/* heart_beat_status() */

/* New version used when not in -o mode. The epilog() in master.c is
 * supposed to return an array of files (castles in 2.4.5) to load. The array
 * returned by apply() will be freed at next call of apply(), which means that
 * the ref count has to be incremented to protect against deallocation.
 *
 * The master object is asked to do the actual loading.
 */
void preload_objects P1(int, eflag)
{
    array_t *prefiles;
    svalue_t *ret;
    VOLATILE int ix;
    error_context_t econ;

    save_context(&econ);
    if (SETJMP(econ.context)) {
	restore_context(&econ);
	pop_context(&econ);
	return;
    }
    push_number(eflag);
    ret = apply_master_ob(APPLY_EPILOG, 1);
    pop_context(&econ);
    if ((ret == 0) || (ret == (svalue_t *)-1) || (ret->type != T_ARRAY))
	return;
    else
	prefiles = ret->u.arr;
    if ((prefiles == 0) || (prefiles->size < 1))
	return;

    debug_message("\nLoading preloaded files ...\n");
    prefiles->ref++;
    ix = 0;
    /* in case of an error, effectively do a 'continue' */
    save_context(&econ);
    if (SETJMP(econ.context)) {
	restore_context(&econ);
	ix++;
    }
    for ( ; ix < prefiles->size; ix++) {
	if (prefiles->item[ix].type != T_STRING)
	    continue;

	eval_cost = max_cost;

	push_svalue(prefiles->item + ix);
	(void) apply_master_ob(APPLY_PRELOAD, 1);
    }
    free_array(prefiles);
    pop_context(&econ);
}				/* preload_objects() */

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */

INLINE void remove_destructed_objects()
{
    object_t *ob, *next;

    if (obj_list_replace)
	replace_programs();
    for (ob = obj_list_destruct; ob; ob = next) {
	next = ob->next_all;
	destruct2(ob);
    }
    obj_list_destruct = 0;
}				/* remove_destructed_objects() */

static double load_av = 0.0;

void update_load_av()
{
    static int last_time;
    int n;
    double c;
    static int acc = 0;

    acc++;
    if (current_time == last_time)
	return;
    n = current_time - last_time;
    if (n < NUM_CONSTS)
	c = consts[n];
    else
	c = exp(-n / 900.0);
    load_av = c * load_av + acc * (1 - c) / n;
    last_time = current_time;
    acc = 0;
}				/* update_load_av() */

static double compile_av = 0.0;

void
update_compile_av P1(int, lines)
{
    static int last_time;
    int n;
    double c;
    static int acc = 0;

    acc += lines;
    if (current_time == last_time)
	return;
    n = current_time - last_time;
    if (n < NUM_CONSTS)
	c = consts[n];
    else
	c = exp(-n / 900.0);
    compile_av = c * compile_av + acc * (1 - c) / n;
    last_time = current_time;
    acc = 0;
}				/* update_compile_av() */

char *query_load_av()
{
    static char buff[100];

    sprintf(buff, "%.2f cmds/s, %.2f comp lines/s", load_av, compile_av);
    return (buff);
}				/* query_load_av() */

#ifdef F_HEART_BEATS
array_t *get_heart_beats() {
    int n = num_hb_objs;
    heart_beat_t *hb = heart_beats;
    array_t *arr;
    
    arr = allocate_empty_array(n);
    while (n--) {
	arr->item[n].type = T_OBJECT;
	arr->item[n].u.ob = hb->ob;
	add_ref(hb->ob, "get_heart_beats");
	hb++;
    }
    return arr;
}
#endif
