/* hosts/amiga/signal_rr.c
**
** Just the exception catcher -- it needs registerized args (-mRR) which
** are not used elsewhere.
**
**   28-Feb-93 [lars]  Done for DICE 2.07.53
*/

#include <exec/types.h>
#ifdef INCLUDE_VERSION
#include <dos/dos.h>
#else
#include <libraries/dos.h>
#endif
#include "nsignal.h"
extern __stkargs ULONG SetSignal(unsigned long newSignals, unsigned long signalSet);


/*-----------------------------------------------------------------------*/

extern ULONG sys_signal_alarm,	/* The system-signal-masks */
      sys_signal_hup, sys_signal_usr;

extern void (*handler_hup) (void), (*handler_alarm) (void), (*handler_usr) (void);

/*-----------------------------------------------------------------------
** ULONG catch_exception (ULONG mask)
**
**   Called by the OS if the task gets an exception, this dispatch to
**   the appropriate signal-function.
**   Note that the std-raise() also removes the handler, so our functions
**   are called manually.
*/

__regargs __geta4 ULONG catch_exception(__D0 ULONG mask)
{
    /* Handle our special exceptions */
    if (mask & sys_signal_alarm)
	(*handler_alarm) ();
    if (mask & sys_signal_hup)
	(*handler_hup) ();
    if (mask & sys_signal_usr)
	(*handler_usr) ();
    /* Stop select() anyway */
    SetSignal(EXT_SIGINT, EXT_SIGINT);
    return mask;
}

/*************************************************************************/
