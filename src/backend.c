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

#if defined(OS2)
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORE
#include <os2.h>

extern HEV mudos_event_sem;
#endif

jmp_buf error_recovery_context;
int error_recovery_context_exists = NULL_ERROR_CONTEXT;

/*
 * The 'current_time' is updated at every heart beat.
 */
int current_time;

int heart_beat_flag = 0;

struct object *current_heart_beat;
static void look_for_objects_to_swap PROT((void));
static void call_heart_beat PROT((void));
INLINE static void cycle_hb_list PROT((void));

#ifdef DEBUG
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
    command_giver = 0;
    current_interactive = 0;
    previous_ob = 0;
    current_prog = 0;
    caller_type = 0;
    error_recovery_context_exists = NORMAL_ERROR_CONTEXT;
    reset_machine(0);		/* Pop down the stack. */
}				/* clear_state() */

#ifdef DEBUG
static void report_holes() {
    if (current_object)
	fprintf(stderr, "current_object is %s\n", current_object->name);
    if (command_giver)
	fprintf(stderr, "command_giver is %s\n", command_giver->name);
    if (current_interactive)
	fprintf(stderr, "current_interactive is %s\n", current_interactive->name);
}
#endif

void logon P1(struct object *, ob)
{
    struct svalue *ret;

    /* current_object no longer set */
    ret = apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
    if (ret == 0) {
	add_vmessage("prog %s:\n", ob->name);
	fatal("Could not find logon on the user %s\n", ob->name);
    }
}				/* logon() */

#ifndef NO_ADD_ACTION
/*
 * Take a user command and parse it.
 * The command can also come from a NPC.
 * Beware that 'str' can be modified and extended !
 */
int parse_command P2(char *, str, struct object *, ob)
{
    struct object *save = command_giver;
    int res;

    /* disallow users to issue commands containing ansi escape codes */
#ifdef NO_ANSI
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

#define NUM_COMMANDS MAX_USERS

/*
 * This is the backend. We will stay here for ever (almost).
 */
int eval_cost;

void backend()
{
    struct timeval timeout;
    int nb;
    int i;

    fprintf(stderr, "Setting up IPC on port %d.\n", (int) PORTNO);
    init_user_conn();		/* initialize user connection socket */
#ifdef SOCKET_EFUNS
    init_sockets();		/* initialize efun sockets           */
#endif
    signal(SIGHUP, startshutdownMudOS);
    if (!t_flag)
	call_heart_beat();
    SETJMP(error_recovery_context);
    clear_state();

#ifdef OS2
    do {
	long rc_wait;

	remove_destructed_objects();
	eval_cost = 0;
	if (MudOS_is_being_shut_down) {
	    shutdownMudOS(0);
	}
	if (slow_shut_down_to_do) {
	    int tmp = slow_shut_down_to_do;

	    slow_shut_down_to_do = 0;
	    slow_shut_down(tmp);
	}
	rc_wait = DosWaitEventSem(mudos_event_sem, -1);
	DosResetEventSem(mudos_event_sem, &i);
	process_io();
	if (heart_beat_flag) {
	    call_heart_beat();
	}			/* endif */
	/*
	 * process user commands.
	 */
	for (i = 0; process_user_command() && i < NUM_COMMANDS; i++);
    } while (1);
#else
    while (1) {
	/*
	 * The call of clear_state() should not really have to be done once
	 * every loop. However, there seem to be holes where the state is not
	 * consistent. If these holes are removed, then the call of
	 * clear_state() can be moved to just before the while() - statement.
	 * *sigh* /Lars
	 */
        /* Well, let's do it and see what happens - Sym */
	/* What happens is random crashes at odd times.  It's too close
	   to v21 to deal with this ... -Beek */
	clear_state();
	eval_cost = max_cost;

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
	     * drop through.
	     */
	    timeout.tv_sec = 60;
	    timeout.tv_usec = 0;
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
	for (i = 0; process_user_command() && i < NUM_COMMANDS; i++)
	    ;

	/*
	 * call heartbeat if appropriate.
	 */
	if (heart_beat_flag) {
	    debug(512, ("backend: HEARTBEAT\n"));
	    call_heart_beat();
	}
    }
#endif
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
    struct object *ob;
    struct object *next_ob;
    jmp_buf save_error_recovery_context;
    int save_rec_exists;
    struct object *save;

    if (current_time < next_time)
	return;			/* Not time to look yet */
    next_time = current_time + 15 * 60;	/* Next time is in 15 minutes */
    memcpy((char *) save_error_recovery_context,
	   (char *) error_recovery_context, sizeof error_recovery_context);
    save_rec_exists = error_recovery_context_exists;

    /*
     * prevent error messages from objects going to whoever happens to be
     * this_user() (usually this isn't the user of the object).
     */
    save = command_giver;
    command_giver = (struct object *) 0;
    /*
     * Objects object can be destructed, which means that next object to
     * investigate is saved in next_ob. If very unlucky, that object can be
     * destructed too. In that case, the loop is simply restarted.
     */
    for (ob = obj_list; ob; ob = next_ob) {
	int ready_for_swap;

	eval_cost = max_cost;

	if (ob->flags & O_DESTRUCTED)
	    ob = obj_list;	/* restart */
	next_ob = ob->next_all;
	if (SETJMP(error_recovery_context)) {	/* amylaar */
	    clear_state();
	    debug_message("Error in " APPLY_CLEAN_UP "() or " APPLY_RESET "().\n");
	    continue;
	}
	/*
	 * Check reference time before reset() is called.
	 */
	if (current_time < ob->time_of_ref + time_to_swap)
	    ready_for_swap = 0;
	else
	    ready_for_swap = 1;
#ifndef LAZY_RESETS
	/*
	 * Should this object have reset(1) called ?
	 */
	if ((ob->flags & O_WILL_RESET) && (ob->next_reset < current_time)
	    && !(ob->flags & O_RESET_STATE)) {
#ifdef DEBUG
	    if (d_flag) {
		fprintf(stderr, "RESET %s\n", ob->name);
	    }
#endif
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

	    if (current_time - ob->time_of_ref > time_to_clean_up
		&& (ob->flags & O_WILL_CLEAN_UP)) {
		int save_reset_state = ob->flags & O_RESET_STATE;
		struct svalue *svp;

#ifdef DEBUG
		if (d_flag)
		    fprintf(stderr, "clean up %s\n", ob->name);
#endif
		/*
		 * Supply a flag to the object that says if this program is
		 * inherited by other objects. Cloned objects might as well
		 * believe they are not inherited. Swapped objects will not
		 * have a ref count > 1 (and will have an invalid ob->prog
		 * pointer).
		 */

		push_number(ob->flags & (O_CLONE | O_SWAPPED) ? 0 : ob->prog->p.i.ref);
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

	    if (ob->prog && ob->prog->p.i.line_info)
		swap_line_numbers(ob->prog);
	    if (ob->flags & O_SWAPPED || !ready_for_swap)
		continue;
	    if (ob->flags & O_HEART_BEAT)
		continue;
#ifdef DEBUG
	    if (d_flag)
		fprintf(stderr, "swap %s\n", ob->name);
#endif
	    swap(ob);		/* See if it is possible to swap out to disk */
	}
    }
    /* restore command_giver to its previous value */
    command_giver = save;
    memcpy((char *) error_recovery_context, (char *) save_error_recovery_context,
	   sizeof error_recovery_context);
    error_recovery_context_exists = save_rec_exists;
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

static struct object *hb_list = 0;	/* head */
static struct object *hb_tail = 0;	/* for sane wrap around */

static int num_hb_objs = 0;	/* so we know when to stop! */
static int num_hb_calls = 0;	/* starts */
static float perc_hb_probes = 100.0;	/* decaying avge of how many complete */

#ifdef OS2
void alarm_loop()
{
    while (1) {
	DosSleep(HEARTBEAT_INTERVAL / 1000);
	heart_beat_flag = 1;
	DosPostEventSem(mudos_event_sem);
    }
}				/* alarm_loop() */
#endif

static void call_heart_beat()
{
    struct object *ob;
    struct object *save_current_object = current_object;
    struct object *save_command_giver = command_giver;
    int num_done = 0;

#ifdef OS2
    static TID bing = 0;

#endif

    heart_beat_flag = 0;

    signal(SIGALRM, sigalrm_handler);
#if !defined(OS2)
#ifdef HAS_UALARM
    ualarm(HEARTBEAT_INTERVAL, 0);
#else
    alarm(SYSV_HEARTBEAT_INTERVAL);	/* defined in config.h */
#endif
#else
    if (!bing) {
	DosCreateThread(&bing, alarm_loop, 0, 0, 100);
    }
#endif				/* OS2 */

    debug(256, ("."));

    current_time = get_current_time();
    current_interactive = 0;

    if (hb_list) {
	num_hb_calls++;
	while (hb_list && !heart_beat_flag && (num_done < num_hb_objs)) {
	    num_done++;
	    cycle_hb_list();
	    ob = hb_tail;	/* now at end */
	    if (!(ob->flags & O_HEART_BEAT))
		fatal("Heartbeat not set in object on heart beat list!");
	    if (ob->flags & O_SWAPPED)
		fatal("Heart beat in swapped object.\n");
	    /* is it time to do a heartbeat ? */
	    ob->heart_beat_ticks--;
	    /* move ob to end of list, do ob */
	    if (ob->prog->p.i.heart_beat == -1)
		continue;
	    if (ob->heart_beat_ticks < 1) {
		ob->heart_beat_ticks = ob->time_to_heart_beat;
		current_prog = ob->prog;
		current_object = ob;
		current_heart_beat = ob;
		command_giver = ob;
#ifndef NO_SHADOWS
		while (command_giver->shadowing)
		    command_giver = command_giver->shadowing;
#endif				/* NO_SHADOWS */
#ifndef NO_ADD_ACTION
		if (!(command_giver->flags & O_ENABLE_COMMANDS))
		    command_giver = 0;
#endif
#ifndef NO_MUDLIB_STATS
		add_heart_beats(&ob->stats, 1);
#endif
		eval_cost = max_cost;
		call_function(ob->prog,
			&ob->prog->p.i.functions[ob->prog->p.i.heart_beat]);
	    }
	}
	if (num_hb_objs)
	    perc_hb_probes = 100 * (float) num_done / num_hb_objs;
	else
	    perc_hb_probes = 100.0;
    }
    current_object = save_current_object;
    current_prog = 0;
    current_heart_beat = 0;
    look_for_objects_to_swap();
    call_out();
#ifndef NO_MUDLIB_STATS
    mudlib_stats_decay();
#endif
    command_giver = save_command_giver;
}				/* call_heart_beat() */

/* Take the first object off the heart beat list, place it at the end
 */
INLINE static void cycle_hb_list()
{
    struct object *ob;

    if (!hb_list)
	fatal("Cycle heart beat list with empty list!");
    if (hb_list == hb_tail)
	return;			/* 1 object on list */
    ob = hb_list;
    hb_list = hb_list->next_heart_beat;
    hb_tail->next_heart_beat = ob;
    hb_tail = ob;
    ob->next_heart_beat = 0;
}				/* cycle_hb_list() */

int
query_heart_beat P1(struct object *, ob)
{
    if (!(ob->flags & O_HEART_BEAT)) {
	return 0;
    } else {
	return ob->time_to_heart_beat;
    }
}				/* query_heart_beat() */

/* add or remove an object from the heart beat list; does the major check...
 * If an object removes something from the list from within a heart beat,
 * various pointers in call_heart_beat could be stuffed, so we must
 * check current_heart_beat and adjust pointers.  */

int set_heart_beat P2(struct object *, ob, int, to)
{
    struct object *o = hb_list;
    struct object *oprev = 0;

    if (ob->flags & O_DESTRUCTED)
	return (0);
#ifdef OLD_HEARTBEAT
    if (to)
	to = 1;
#endif
    while (o && o != ob) {
	if (!(o->flags & O_HEART_BEAT))
	    fatal("Found disabled object in the active heart beat list!\n");
	oprev = o;
	o = o->next_heart_beat;
    }
    if (!o && (ob->flags & O_HEART_BEAT))
	fatal("Couldn't find enabled object in heart beat list!");
    if (!to && !(ob->flags & O_HEART_BEAT))	/* if set_heart_beat(0) and
						 * O_HEART_BEAT is 0 */
	return (0);
    if (to && (ob->flags & O_HEART_BEAT)) {	/* change to intervals for
						 * heart_beat */
	if (to < 0)
	    return 0;
	ob->time_to_heart_beat = to;
	ob->heart_beat_ticks = to;
	return to;
    }
    if (to) {
	ob->flags |= O_HEART_BEAT;
	if (ob->next_heart_beat)
	    fatal("Dangling pointer to next_heart_beat in object!");
	ob->next_heart_beat = hb_list;
	hb_list = ob;
	if (!hb_tail)
	    hb_tail = ob;
	num_hb_objs++;
	ob->time_to_heart_beat = to;
	ob->heart_beat_ticks = to;
	cycle_hb_list();	/* Added by Linus. 911104 */
    } else {			/* remove all refs */
	ob->flags &= ~O_HEART_BEAT;
	if (hb_list == ob)
	    hb_list = ob->next_heart_beat;
	if (hb_tail == ob)
	    hb_tail = oprev;
	if (oprev)
	    oprev->next_heart_beat = ob->next_heart_beat;
	ob->next_heart_beat = 0;
	num_hb_objs--;
    }
    return (1);
}				/* set_heart_beat() */

int heart_beat_status P1(int, verbose)
{
    char buf[20];

    if (verbose == 1) {
	add_message("Heart beat information:\n");
	add_message("-----------------------\n");
	add_vmessage("Number of objects with heart beat: %d, starts: %d\n",
		    num_hb_objs, num_hb_calls);
	sprintf(buf, "%.2f", perc_hb_probes);
	add_vmessage("Percentage of HB calls completed last time: %s\n", buf);
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
    struct vector *prefiles;
    struct svalue *ret;
    VOLATILE int ix;

    if (SETJMP(error_recovery_context)) {
	clear_state();
	debug_message("error in epilog() in the master object.\n");
	error_recovery_context_exists = NULL_ERROR_CONTEXT;
	return;
    }
    error_recovery_context_exists = NORMAL_ERROR_CONTEXT;
    push_number(eflag);
    ret = apply_master_ob(APPLY_EPILOG, 1);
    if ((ret == 0) || (ret == (svalue *)-1) || (ret->type != T_POINTER))
	return;
    else
	prefiles = ret->u.vec;
    if ((prefiles == 0) || (prefiles->size < 1))
	return;
    prefiles->ref++;
    ix = -1;
    if (SETJMP(error_recovery_context)) {
	clear_state();
	add_message("error in preload() in the master object.\n");
    }
    while (++ix < prefiles->size) {
	if (prefiles->item[ix].type != T_STRING)
	    continue;

	eval_cost = max_cost;

	push_string(prefiles->item[ix].u.string, STRING_MALLOC);
	(void) apply_master_ob(APPLY_PRELOAD, 1);
    }
    free_vector(prefiles);
    error_recovery_context_exists = NULL_ERROR_CONTEXT;
}				/* preload_objects() */

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */

INLINE void remove_destructed_objects()
{
    struct object *ob, *next;

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
