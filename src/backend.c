/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/times.h>
#include <math.h>
#include <memory.h>

#include "lint.h"
#include "config.h"
#include "interpret.h"
#include "object.h"
#include "wiz_list.h"
#include "exec.h"
#include "comm.h"

jmp_buf	error_recovery_context;
int		error_recovery_context_exists = 0;

/*
 * The 'current_time' is updated at every heart beat. */

int current_time;

static void cycle_hb_list PROT((void));
extern struct object *command_giver, *current_interactive, *obj_list_destruct;
extern struct object *previous_ob, *master_ob;
extern int num_player, d_flag;

struct object *current_heart_beat;

void call_heart_beat(), catch_alarm();
void prepare_ipc(),
    shutdowngame(), ed_cmd PROT((char *)),
    print_prompt(), call_out(),
    destruct2 PROT((struct object *));

extern int get_message PROT((char *, int)), player_parser PROT((char *)),
    call_function_interactive PROT((struct interactive *, char *)),
    resort_free_list(), swap PROT((struct object *));

extern void flush_all_player_mess();

extern int t_flag;
int time_to_call_heart_beat;
int comm_time_to_call_heart_beat = 0; /* this is set by interrupt, */
	/* comm sets time_to_call_heart_beat sometime after */

/* There are global variables that must be zeroed before any execution.
 * In case of errors, there will be a longjmp(), and the variables will
 * have to be cleared explicitely. They are normally maintained by the
 * code that use them.
 *
 * This routine must only be called from top level, not from inside
 * stack machine execution (as stack will be cleared).  */

void clear_state()
{
	extern struct object *previous_ob;
	current_object = 0;
	command_giver = 0;
	current_interactive = 0;
	previous_ob = 0;
	current_prog = 0;
	error_recovery_context_exists = 1;
	reset_machine(0);	/* Pop down the stack. */
}

void logon(ob)
	struct object *ob;
{
	struct svalue *ret;
	struct object *save = current_object;

	/* current_object must be set here, so that the static "logon" in
	 * player.c can be called.  */
	current_object = ob;
	ret = apply("logon", ob, 0);
	if (ret == 0)
	{	add_message("prog %s:\n", ob->name);
		fatal("Could not find logon on the player %s\n", ob->name);
	}
	current_object = save;
}

/* Take a player command and parse it.
 * The command can also come from a NPC.
 * Beware that 'str' can be modified and extended ! */

int parse_command(str, ob)
    char *str;
    struct object *ob;
{
    struct object *save = command_giver;
    int res;

    command_giver = ob;
    res = player_parser(str);
    command_giver = save;
    return res;
}

/* This is the backend. We will stay here for ever (almost).
 */

int eval_cost;
void backend()
{
    char buff[COMMAND_BUF_SIZE];
    extern int game_is_being_shut_down;
    extern int slow_shut_down_to_do;

    (void)printf("Setting up IPC.\n");
    fflush(stdout);
    prepare_ipc();
    (void)signal(SIGHUP, startshutdowngame);
    if (!t_flag)
		call_heart_beat();
    setjmp(error_recovery_context);
    while(1)
	{	/* The call of clear_state() should not really have to be done
		 * once every loop. However, there seem to be holes where the
		 * state is not consistent. If these holes are removed,
		 * then the call of clear_state() can be moved to just before the
		 * while() - statment. *sigh* /Lars */
		clear_state();
		eval_cost = 0;
		remove_destructed_objects(); /* marion - before ref checks! */
#ifdef DEBUG
		if (d_flag > 1)
			check_a_lot_ref_counts(0);
#endif
		if (game_is_being_shut_down)
			shutdowngame();
		if (slow_shut_down_to_do)
		{	int tmp = slow_shut_down_to_do;
			slow_shut_down_to_do = 0;
			slow_shut_down(tmp);
		}
		if (get_message(buff, sizeof buff))
		{	void update_load_av PROT((void));

			update_load_av();
			/* Now we have a string from the player. This string can go to
			 * one of several places. If it is prepended with a '!', then
			 * it is an escape from the 'ed' editor, so we send it
			 * as a command to the parser.
			 * If any object function is waiting for an input string, then
			 * send it there.
			 * Otherwise, send the string to the parser.
			 * The player_parser() will find that current_object is 0, and
			 * then set current_object to point to the object that defines
			 * the command. This will enable such functions to be static.  */
			current_object = 0;
			current_interactive = command_giver;

#ifdef DEBUG
			if (!command_giver->interactive)
				fatal("Non interactive player in main loop !\n");
#endif
			if (buff[0] == '!'
				&& (command_giver->interactive->ed_buffer
				|| command_giver->interactive->input_to))
			{	struct svalue * ret;

				push_constant_string(buff+1); /* not malloced */
				ret = apply ("process_input",command_giver,1);
				if (ret && (ret->type == T_STRING) && ret->u.string)
					strcpy (buff+1,ret->u.string);
				else
					command_giver->interactive->has_process_input = 0;
				parse_command(buff+1, command_giver);
			}
			else if (command_giver->interactive->ed_buffer)
				ed_cmd(buff);
			else if (call_function_interactive(command_giver->interactive,buff))
			{
				/* Do nothing ! */
			}
			else
			{	/* send a copy of user input back to player object to
				 * provide support for things like command history and
				 * mud shell programming languages */ 
				if (command_giver->interactive->has_process_input)
				{	struct svalue * ret;

					push_constant_string(buff); /* not malloced */
					ret = apply ("process_input",command_giver,1);
					if (ret && (ret->type == T_STRING) && ret->u.string)
						strcpy (buff,ret->u.string);
					else
						command_giver->interactive->has_process_input = 0;
				}
				parse_command(buff, command_giver);
			}
			/* Print a prompt if player is still here.
			*/
			if (command_giver->interactive)
				print_prompt();
		}
		if (time_to_call_heart_beat)
			call_heart_beat();
		command_giver = 0;
	}
}

/* Despite the name, this routine takes care of several things.
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
 * special care has to be taken of how the linked list is used.  */

static void look_for_objects_to_swap()
{
    extern long time_to_swap; /* marion - for invocation parameter */
    static int next_time;
    struct object *ob;
    struct object *next_ob;
    jmp_buf save_error_recovery_context;
    int save_rec_exists;
	struct object *save;

    if (current_time < next_time)
		return;				/* Not time to look yet */
    next_time = current_time + 15 * 60;	/* Next time is in 15 minutes */
    memcpy((char *) save_error_recovery_context,
	   (char *) error_recovery_context, sizeof error_recovery_context);
    save_rec_exists = error_recovery_context_exists;

	/* prevent error messages from objects going to whoever happens to
	 * be this_player() (usually this isn't the user of the object).  */
	save = command_giver;
	command_giver = (struct object *)0;
    /* Objects object can be destructed, which means that
     * next object to investigate is saved in next_ob. If very unlucky,
     * that object can be destructed too. In that case, the loop is simply
     * restarted.  */
    for (ob = obj_list; ob; ob = next_ob)
	{
		int ready_for_swap;

		if (ob->flags & O_DESTRUCTED)
			ob = obj_list; /* restart */
		next_ob = ob->next_all;
        if (setjmp(error_recovery_context))
		{	/* amylaar */
            extern void clear_state();
            clear_state();
            debug_message("Error in look_for_objects_to_swap.\n");
			continue;
        }
		/* Check reference time before reset() is called.
		 */
		if (current_time < ob->time_of_ref + time_to_swap)
			ready_for_swap = 0;
		else
			ready_for_swap = 1;
		/* Should this object have reset(1) called ?
		 */
		if (ob->next_reset < current_time && !(ob->flags & O_RESET_STATE))
		{
			if (d_flag)
				fprintf(stderr, "RESET %s\n", ob->name);
			reset_object(ob, 1);
		}
		if (TIME_TO_CLEAN_UP > 0)
		{
			/* Has enough time passed, to give the object a chance
			 * to self-destruct ? Save the O_RESET_STATE, which will be cleared.
			 *
			 * Only call clean_up in objects that has defined such a function.
			 *
			 * Only if the clean_up returns a non-zero value, will it be called
			 * again.  */

			if (current_time - ob->time_of_ref > TIME_TO_CLEAN_UP
				&& (ob->flags & O_WILL_CLEAN_UP))
			{
				int save_reset_state = ob->flags & O_RESET_STATE;
				struct svalue *svp;
	      
				if (d_flag)
					fprintf(stderr, "clean up %s\n", ob->name);
				/* Supply a flag to the object that says if this program
				 * is inherited by other objects. Cloned objects might as well
				 * believe they are not inherited. Swapped objects will not
				 * have a ref count > 1 (and will have an invalid ob->prog
				 * pointer).  */

				push_number(ob->flags &
					(O_CLONE|O_SWAPPED) ? 0 : ob->prog->ref);
				svp = apply("clean_up", ob, 1);
				if (ob->flags & O_DESTRUCTED)
					continue;
				if (!svp || (svp->type == T_NUMBER && svp->u.number == 0))
					ob->flags &= ~O_WILL_CLEAN_UP;
				ob->flags |= save_reset_state;
			}
		}
		if (TIME_TO_SWAP > 0)
		{	/* At last, there is a possibility that the object can be swapped
			 * out.  */

			if (ob->flags & O_SWAPPED || !ready_for_swap)
				continue;
			if (ob->flags & O_HEART_BEAT)
				continue;
			if (d_flag)
				fprintf(stderr, "swap %s\n", ob->name);
			swap(ob);	/* See if it is possible to swap out to disk */
		}
	}
	/* restore command_giver to its previous value */
	command_giver = save;
    memcpy((char *) error_recovery_context,
	   (char *) save_error_recovery_context,
	   sizeof error_recovery_context);
    error_recovery_context_exists = save_rec_exists;
}

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

static struct object *hb_list = 0; /* head */
static struct object *hb_tail = 0; /* for sane wrap around */

static int num_hb_objs = 0;  /* so we know when to stop! */
static int num_hb_calls = 0; /* stats */
static float perc_hb_probes = 100.0; /* decaying avge of how many complete */

void call_heart_beat()
{
    struct object *ob, *hide_current = current_object;
    int num_done = 0;
    
    time_to_call_heart_beat = 0; /* interrupt loop if we take too long */
    comm_time_to_call_heart_beat = 0;
#ifndef MSDOS
    (void)signal(SIGALRM, catch_alarm);
    alarm(2);
#else
    start_timer(2);
#endif
    current_time = get_current_time();
    current_interactive = 0;

    if ((num_player > 0) && hb_list)
	{
        num_hb_calls++;
		while (hb_list &&
#ifndef MSDOS
	       !comm_time_to_call_heart_beat
#else
	       !timer_expired()
#endif
	       && (num_done < num_hb_objs))
	   {
			num_done++;
			cycle_hb_list();
			ob = hb_tail; /* now at end */
			if (!(ob->flags & O_HEART_BEAT))
				fatal("Heart beat not set in object on heart beat list!");
			if (ob->flags & O_SWAPPED)
				fatal("Heart beat in swapped object.\n");
			/* move ob to end of list, do ob */
			if (ob->prog->heart_beat == -1)
				continue;
			current_prog = ob->prog;
			current_object = ob;
			current_heart_beat = ob;
			command_giver = ob;
#ifndef NO_SHADOWS /* MudOS */
			while(command_giver->shadowing)
				command_giver = command_giver->shadowing;
#endif NO_SHADOWS
			if (!(command_giver->flags & O_ENABLE_COMMANDS))
				command_giver = 0;
			if (ob->user)
				ob->user->heart_beats++;
			eval_cost = 0;
			call_function(ob->prog,
				&ob->prog->functions[ob->prog->heart_beat]);
		}
		if (num_hb_objs)
			perc_hb_probes = 100 * (float) num_done / num_hb_objs;
		else
			perc_hb_probes = 100.0;
    }
    current_object = hide_current;
    current_heart_beat = 0;
    look_for_objects_to_swap();
    call_out();	/* some things depend on this, even without players! */
    flush_all_player_mess();
    wiz_decay();
#ifdef MUDWHO
    sendmudwhoinfo();
#endif
}

/* Take the first object off the heart beat list, place it at the end
 */
static void cycle_hb_list()
{
    struct object *ob;

    if (!hb_list)
		fatal("Cycle heart beat list with empty list!");
    if (hb_list == hb_tail)
		return; /* 1 object on list */
    ob = hb_list;
    hb_list = hb_list->next_heart_beat;
    hb_tail->next_heart_beat = ob;
    hb_tail = ob;
    ob->next_heart_beat = 0;
}

/* add or remove an object from the heart beat list; does the major check...
 * If an object removes something from the list from within a heart beat,
 * various pointers in call_heart_beat could be stuffed, so we must
 * check current_heart_beat and adjust pointers.  */

int set_heart_beat(ob, to)
    struct object *ob;
    int to;
{
    struct object *o = hb_list;
    struct object *oprev = 0;

    if (ob->flags & O_DESTRUCTED)
		return 0;
    if (to)
		to = 1;
    while (o && o != ob)
	{
		if (!(o->flags & O_HEART_BEAT))
			fatal("Found disabled object in the active heart beat list!\n");
		oprev = o;
		o = o->next_heart_beat;
	}
    if (!o && (ob->flags & O_HEART_BEAT))
		fatal("Couldn't find enabled object in heart beat list!");
    if (to == (ob->flags & O_HEART_BEAT))
		return(0);
    if (to)
	{
		ob->flags |= O_HEART_BEAT;
		if (ob->next_heart_beat)
			fatal("Dangling pointer to next_heart_beat in object!");
		ob->next_heart_beat = hb_list;
		hb_list = ob;
		if (!hb_tail)
			hb_tail = ob;
		num_hb_objs++;
		cycle_hb_list();     /* Added by Linus. 911104 */
    }
    else
	{ /* remove all refs */
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
    return(1);
}
/* sigh.  Another status function.  */

int heart_beat_status(verbose)
    int verbose;
{
    char buf[20];

    if (verbose)
	{
		add_message("\nHeart beat information:\n");
		add_message("-----------------------\n");
		add_message("Number of objects with heart beat: %d, starts: %d\n",
			num_hb_objs, num_hb_calls);
		sprintf(buf, "%.2f", perc_hb_probes);
		add_message("Percentage of HB calls completed last time: %s\n", buf);
    }
    return 0;
}

/* New version used when not in -o mode. The epilog() in master.c is
 * supposed to return an array of files (castles in 2.4.5) to load. The array
 * returned by apply() will be freed at next call of apply(), which means that
 * the ref count has to be incremented to protect against deallocation.
 *
 * The master object is asked to do the actual loading.
 */
void preload_objects(eflag)
    int eflag;
{
    struct vector *prefiles;
    struct svalue *ret;
    int ix;

    push_number(eflag);
    ret = apply_master_ob("epilog", 1);
    if ((ret == 0) || (ret->type != T_POINTER))
		return;
    else
		prefiles = ret->u.vec;
    if ((prefiles == 0) || (prefiles->size < 1))
		return;
    prefiles->ref++;
    ix = -1;
    if (setjmp(error_recovery_context))
	{
		clear_state();
		add_message("Anomaly in the fabric of world space.\n");
	}
    error_recovery_context_exists = 1;
    while (++ix < prefiles->size)
	{
		if (prefiles->item[ix].type != T_STRING)
			continue;
		eval_cost = 0;
		push_string(prefiles->item[ix].u.string, STRING_MALLOC);
		(void)apply_master_ob("preload", 1);
    }
    free_vector(prefiles);
    error_recovery_context_exists = 0;
}

/* catch alarm, set flag for comms code and heart_beat to catch.
 * comms code sets time_to_call_heart_beat for the backend when
 * it has completed the current round of player commands.  */

void catch_alarm()
{	comm_time_to_call_heart_beat = 1;
}

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */

void remove_destructed_objects()
{
    struct object *ob, *next;
    for (ob=obj_list_destruct; ob; ob = next)
	{
		next = ob->next_all;
		destruct2(ob);
    }
    obj_list_destruct = 0;
}

static double load_av = 0.0;

void update_load_av()
{
    extern double consts[5];
    static int last_time;
    int n;
    double c;
    static int acc = 0;

    acc++;
    if (current_time == last_time)
		return;
    n = current_time - last_time;
    if (n < sizeof consts / sizeof consts[0])
		c = consts[n];
    else
		c = exp(- n / 900.0);
    load_av = c * load_av + acc * (1 - c) / n;
    last_time = current_time;
    acc = 0;
}

static double compile_av = 0.0;

void update_compile_av(lines)
    int lines;
{
    extern double consts[5];
    static int last_time;
    int n;
    double c;
    static int acc = 0;

    acc += lines;
    if (current_time == last_time)
		return;
    n = current_time - last_time;
    if (n < sizeof consts / sizeof consts[0])
		c = consts[n];
    else
		c = exp(- n / 900.0);
    compile_av = c * compile_av + acc * (1 - c) / n;
    last_time = current_time;
    acc = 0;
}

char *query_load_av()
{
    static char buff[100];

    sprintf(buff, "%.2f cmds/s, %.2f comp lines/s", load_av, compile_av);
    return buff;
}

/* EOF */
