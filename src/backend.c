/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "std.h"
#include "lpc_incl.h"
#include "backend.h"
#include "comm.h"
#include "replace_program.h"
#include "socket_efuns.h"
#include "swap.h"
#include "port.h"
#include "lint.h"
#include "master.h"

error_context_t *current_error_context = 0;

/*
 * The 'current_time' is updated at every pulse.
 */
int current_time;
static struct timeval next_pulse, pulse_timeout, *pulse_tv;

static void generate_pulse PROT((int));

#if 0
static void report_holes PROT((void));
#endif

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
    set_command_giver(0);
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

/*
 * This is the backend. We will stay here for ever (almost).
 */
int eval_cost;

void backend()
{
    struct timeval current;
    int i, nb;
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

    if (!t_flag) {
	generate_pulse(1);
	pulse_tv = ((PULSE_INTERVAL > 0) ? &pulse_timeout : 0);
    }

    if (!there_is_a_port)
	debug_message("No external ports specified.\n");

    init_user_conn();		/* initialize user connection socket */
#ifdef SIGHUP
    signal(SIGHUP, startshutdownMudOS);
#endif
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

	gettimeofday(&current, 0);
	pulse_timeout.tv_usec  = (next_pulse.tv_usec + 1000000) - current.tv_usec;
	pulse_timeout.tv_sec   = ((next_pulse.tv_sec - 1) - current.tv_sec) + (pulse_timeout.tv_usec / 1000000);
	pulse_timeout.tv_usec %= 1000000;

	if (pulse_timeout.tv_sec < 0)
	    pulse_timeout.tv_sec = pulse_timeout.tv_usec = 0;

#ifndef hpux
	nb = select(FD_SETSIZE, &readmask, &writemask, (fd_set *) 0, pulse_tv);
#else
	nb = select(FD_SETSIZE, (int *) &readmask, (int *) &writemask, (int *) 0, &pulse_tv);
#endif
	/*
	 * process I/O if necessary.
	 */
	if (nb > 0) {
	    process_io();
	    while (process_user_command());
	}
	/*
	 * generate a pulse
	 */
	if (!t_flag)
	    generate_pulse(0);
    }
}				/* backend() */

/*
 * Despite the name, this routine takes care of several things.
 * It will run once every 15 minutes.
 *
 * . It will attempt to reconnect to the address server if the connection has
 *   been lost.
 * . It will loop through all objects.
 *
 *   . If an object is found in a state of not having done reset, and the
 *     delay to next reset has passed, then reset() will be done.
 *
 *   . If the object has a existed more than the time limit given for swapping,
 *     then 'clean_up' will first be called in the object, after which it will
 *     be swapped out if it still exists.
 *
 * There are some problems if the object self-destructs in clean_up, so
 * special care has to be taken of how the linked list is used.
*/
static void look_for_objects_to_swap()
{
    static int next_time;
    extern int no_ip_daemon;
    static int next_server_time;
    object_t *ob;
    VOLATILE object_t *next_ob;
    error_context_t econ;

    /* Attempt to reconnect to the address server if the connection has been
     * severed (or never existed).
     */
    if (!next_server_time) {
	if (ADDR_SERVER_RECONNECT > 0)
	    next_server_time = current_time + ADDR_SERVER_RECONNECT;
	else
	    next_server_time = -1;
    }

    if (next_server_time >= 0 && current_time >= next_server_time) {
	/* initialize the address server.  if it is already initialized, then
	 * this is a nop.  this will cause the driver to reattempt connecting
	 * to the address server once every 15 minutes in the event that it
	 * has gone down.
	 */
	if (!no_ip_daemon)
	    init_addr_server(ADDR_SERVER_IP, ADDR_SERVER_PORT);
	next_server_time = current_time + ADDR_SERVER_RECONNECT;
    }

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
	int ready_for_swap = 0;
	int ready_for_clean_up = 0;

	eval_cost = max_cost;

	if (ob->flags & O_DESTRUCTED)
	    ob = obj_list;	/* restart */
	next_ob = ob->next_all;

	/*
	 * Check reference time before reset() is called.
	 */
	if (current_time - ob->time_of_ref > time_to_swap)
	    ready_for_swap = 1;
	if (current_time - ob->time_of_ref > time_to_clean_up)
	    ready_for_clean_up = 1;
#if !defined(NO_RESETS) && !defined(LAZY_RESETS)
	/*
	 * Should this object have reset(1) called ?
	 */
	if ((ob->flags & O_WILL_RESET) && (ob->next_reset < current_time)
	    && !(ob->flags & O_RESET_STATE)) {
	    debug(d_flag, ("RESET /%s\n", ob->name));
	    reset_object(ob);
	}
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

	    if (ready_for_clean_up && (ob->flags & O_WILL_CLEAN_UP)) {
		int save_reset_state = ob->flags & O_RESET_STATE;
		svalue_t *svp;

		debug(d_flag, ("clean up /%s\n", ob->name));

		/*
		 * Supply a flag to the object that says if this program is
		 * inherited by other objects. Cloned objects might as well
		 * believe they are not inherited. Swapped objects will not
		 * have a ref count > 1 (and will have an invalid ob->prog
		 * pointer).
		 *
		 * Note that if it is in the apply_low cache, it will also
		 * get a flag of 1, which may cause the mudlib not to clean
		 * up the object.  This isn't bad because:
		 * (1) one expects it is rare for objects that have untouched
		 * long enough to clean_up to still be in the cache, especially
		 * on busy MUDs.
		 * (2) the ones that are are the more heavily used ones, so
		 * keeping them around seems justified.
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
	     * out.  Always swap out line number information.  If already
	     * swapped or not time yet, don't swap.
	     */

	    if (ob->prog && ob->prog->line_info)
		swap_line_numbers(ob->prog);
	    if (ob->flags & O_SWAPPED || !ready_for_swap)
		continue;

	    debug(d_flag, ("swap /%s\n", ob->name));
	    swap(ob);		/* See if it is possible to swap out to disk */
	}
    }
    pop_context(&econ);
}				/* look_for_objects_to_swap() */

/*
 * Call the pulse() function in the master object.  Also call the next reset.
 */
static void generate_pulse P1(int, setup)
{
    if (!setup) {
	/* first check to see if it's time to run */
	if (next_pulse.tv_sec || next_pulse.tv_usec) {
	    struct timeval current;

	    gettimeofday(&current, 0);
	    if (current.tv_sec < next_pulse.tv_sec ||
		(current.tv_sec == next_pulse.tv_sec && current.tv_usec < next_pulse.tv_usec))
	    {
		/* it's not time yet */
		return;
	    }
	}

	/* generate the pulse, but only if enabled */
	if (PULSE_INTERVAL > 0)
	    safe_apply_master_ob(APPLY_PULSE, 0);

	look_for_objects_to_swap();
	current_time = get_current_time();
    }

    /* setup for the next pulse */
    gettimeofday(&next_pulse, 0);
    next_pulse.tv_sec += ((next_pulse.tv_usec + PULSE_INTERVAL) / 1000000);
    next_pulse.tv_usec = ((next_pulse.tv_usec + PULSE_INTERVAL) % 1000000);
}				/* generate_pulse() */

/* New version used when not in -o mode. The epilog() in master.c is
 * supposed to return an array of files (castles in 2.4.5) to load. The array
 * returned by apply() will be freed at next call of apply(), which means that
 * the ref count has to be incremented to protect against deallocation.
 *
 * The master object is asked to do the actual loading.
 */
void preload_objects P1(int, eflag)
{
    VOLATILE array_t *prefiles;
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

	push_svalue(((array_t *)prefiles)->item + ix);
	(void) apply_master_ob(APPLY_PRELOAD, 1);
    }
    free_array((array_t *)prefiles);
    pop_context(&econ);
}				/* preload_objects() */

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */

INLINE void remove_destructed_objects()
{
    int n;
    object_t *ob, *next;

    if (obj_list_replace)
	replace_programs();
    if (obj_list_destruct) {
	for (ob = obj_list_destruct; ob; ob = next) {
	    next = ob->next_all;
	    destruct2(ob);
	}
	obj_list_destruct = 0;
    }
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

void set_pulse_interval P1(int, seconds)
{
    PULSE_INTERVAL = seconds;
    if (PULSE_INTERVAL > 0) {
	gettimeofday(&next_pulse, 0);
	next_pulse.tv_sec += ((next_pulse.tv_usec + PULSE_INTERVAL) / 1000000);
	next_pulse.tv_usec = ((next_pulse.tv_usec + PULSE_INTERVAL) % 1000000);
	pulse_tv = &pulse_timeout;
    } else {
	/* pulse disabled */
	pulse_tv = 0;
    }
}
