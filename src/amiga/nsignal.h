/* hosts/amiga/nsignal.h */

#ifndef AMIGA_NSIGNAL_H
#define AMIGA_NSIGNAL_H

#include <exec/types.h>
#include "ccdep.h"

/* LPMud signals aren't raised by other programs, but by external events,
** so the normal signal()/raise() mechanism provided by DICE isn't enough
** since it doesn't uses task-exception handlers.
*/

#define signal(a,b) new_signal(a,b)

/* New signals */

#define SIGALRM (NSIG-1)
#define SIGUSR1 (NSIG-2)

#define SIGHUP  29

/* Used standard task signals */

#define EXT_SIGHUP  SIGBREAKF_CTRL_C	/* Ctrl-C: hang-up */
#define EXT_SIGINT  SIGBREAKF_CTRL_E	/* Ctrl-E: interrupt select() */
#define EXT_SIGUSR  SIGBREAKF_CTRL_F	/* Ctrl-F: update master */

/* Prototypes */

extern __stkargs unsigned int alarm(unsigned int);
extern __stkargs __sigfunc new_signal(int, __sigfunc);
extern __stkargs int start_timer(struct timeval *, struct timerequest *);
extern __stkargs int setup_timer(LONG, struct timerequest **);
extern __stkargs void cleanup_timer(struct timerequest **);
extern __stkargs ULONG check_signals(void);

#endif
