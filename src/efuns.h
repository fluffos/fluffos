/* efuns.h:  this file should be included by any .c file that wants to
   define f_* efuns to be called by eval_instruction() in interpret.c
*/

#include "config.h" /* must be included before the #ifdef TIMES */
#include <sys/types.h>
#ifdef TIMES
#include <sys/times.h>
#endif
#include <sys/stat.h>
#include <time.h>
#ifdef sun
#include <sys/time.h>
#endif /* sun */
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <memory.h>
#include <setjmp.h>

#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "exec.h"
#include "efun_protos.h"
#include "comm.h"
#include "localtime.h"
#include "socket_efuns.h"
#include "socket_errors.h"
#include "lang.tab.h"
#include "switch.h"
#include "sent.h"
#include "debug.h" /* needed by f_set_debug_level() */

/* Needed these from interpret.c. But what for? -SH */
#define TRACE_CALL_OTHER 2
#define TRACE_RETURN 4
#define TRACE_ARGS 8
#define TRACE_HEART_BEAT 32
#define TRACETST(b) (command_giver->interactive->trace_level & (b))
#define TRACEP(b) \
    (command_giver && command_giver->interactive && TRACETST(b) && \
     (command_giver->interactive->trace_prefix == 0 || \
      (current_object && strpref(command_giver->interactive->trace_prefix, \
	      current_object->name))) )
#define TRACEHB (current_heart_beat == 0 \
	 || (command_giver->interactive->trace_level & TRACE_HEART_BEAT))

extern int max_string_length;
extern int d_flag, boot_time;
extern char *pc;
extern int tracedepth;
extern int current_time;
extern char *last_verb;
extern struct svalue *fp;	  /* Pointer to first argument. */
extern int function_index_offset; /* Needed for inheritance */
extern int variable_index_offset; /* Needed for inheritance */
extern struct object *previous_ob;
extern struct object *master_ob;
extern struct svalue *expected_stack;
extern userid_t *backbone_uid;
extern struct svalue const0, const1, const0u, const0n;
extern struct object *current_heart_beat, *current_interactive;
extern struct svalue catch_value;	/* Used to throw an error to a catch */
extern short *break_sp;		/* Points to address to branch to
				 * at next F_BREAK			*/
extern struct control_stack *csp;	/* Points to last element pushed */

extern struct svalue *sp;
extern int num_hidden;
extern int eval_cost;

#ifdef CACHE_STATS
extern unsigned long int apply_low_call_others;
extern unsigned long int apply_low_cache_hits;
extern unsigned long int apply_low_slots_used;
extern unsigned long int apply_low_collisions;
#endif

static int i;
static struct svalue *argp;
static struct object *ob;
