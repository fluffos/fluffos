/* efuns.h:  this file should be included by any .c file that wants to
   define f_* efuns to be called by eval_instruction() in interpret.c
*/

#ifndef _EFUNS_H_
#define _EFUNS_H_

#include "config.h"		/* must be included before the #ifdef TIMES */

#include <sys/types.h>
#ifdef GET_PROCESS_STATS
#include <sys/procstats.h>
#endif
#if defined(__bsdi__) || defined(epix)
#include <sys/time.h>
#endif
#ifdef TIMES
#include <sys/times.h>
#endif
#include <sys/stat.h>
#if !defined(hp68k)
#include <time.h>
#endif				/* !hp68k */
#if !defined(LATTICE) && (defined(sun) || defined(apollo) || defined(__386BSD__) || defined(hp68k) || defined(__STDC__))
#include <sys/time.h>
#endif				/* sun, etc */
#if !defined(LATTICE) && !defined(_M_UNIX)
#include <sys/resource.h>
#endif
#ifdef SunOS_5
#include <sys/rusage.h>
#include <crypt.h>
#endif
#if !defined(LATTICE) && !defined(OS2)
#include <sys/ioctl.h>
#include <netdb.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifndef LATTICE
#include <memory.h>
#endif
#include <setjmp.h>

#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "buffer.h"
#include "object.h"
#include "exec.h"
#include "efun_protos.h"
#include "comm.h"
#include "include/localtime.h"
#include "socket_efuns.h"
#include "include/socket_err.h"
#include "opcodes.h"
#include "switch.h"
#include "sent.h"
#include "debug.h"		/* needed by f_set_debug_level() */

#ifdef LPC_TO_C
#include "virtual_architecture.h"
#endif

/* Needed these from interpret.c. But what for? -SH */
#define TRACE_CALL_OTHER 2
#define TRACE_RETURN 4
#define TRACE_ARGS 8
#define TRACE_HEART_BEAT 32

#ifdef LPC_TO_C
#define TRACE_COMPILED 256
#define TRACE_LPC_EXEC 512
#endif

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
extern struct svalue *fp;	/* Pointer to first argument. */
extern int function_index_offset;	/* Needed for inheritance */
extern int variable_index_offset;	/* Needed for inheritance */
extern struct object *previous_ob;
extern struct object *master_ob;
extern userid_t *backbone_uid;
extern struct svalue const0, const1, const0u, const0n;
extern struct object *current_heart_beat, *current_interactive;
extern struct svalue catch_value;	/* Used to throw an error to a catch */
extern short *break_sp;		/* Points to address to branch to at next
				 * F_BREAK			 */
extern struct control_stack *csp;	/* Points to last element pushed */

#ifdef LPC_TO_C
extern struct control_stack control_stack[MAX_TRACE];

#endif

extern struct svalue *sp;
extern int num_hidden;
extern int eval_cost;
extern short int caller_type;

#ifdef CACHE_STATS
extern unsigned int apply_low_call_others;
extern unsigned int apply_low_cache_hits;
extern unsigned int apply_low_slots_used;
extern unsigned int apply_low_collisions;

#endif

#endif
