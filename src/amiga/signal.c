/* hosts/amiga/signal.c
**
** Adapt the default-signal handling for inclusion of timer-
** and user-generated signals. In detail this means setting the
** task_exception_code hook on a stub which 'raise()s' the appropriate
** signals.
** This is closely coupled with the timer functions so they are also
** implemented here.
**
** LPMud uses the signals SIGHUP (driver process abortion by the user),
** SIGALRM (alarm timed out) and an interrupt signal for select().
** Amylaar additionally uses SIGUSR1 (update of the master by the user).
** These three signals are not caused by other program parts (as the standard
** implementation assumes), but instead are caused by timer time-outs or
** keypresses. Therefore these are implemented explicitely by using
** the Amiga's internal task signal system, which allows the setting of
** an exception handler which is called whenever a task signal is raised.
** The other signals are passed through to the standard clib signal().
**
** For easier compilation, the catch_exception() which needs registerized
** args is put in a separate file signal_rr.c .
**
** This code is based on the UnixLib by Erik van Roode.
**
**   18-Oct-92 [lars]  Done for DICE 2.06.40
**   24-Feb-93 [lars]  Small fix to support compilation for OS 1.3
**   28-Feb-93 [lars]  Moved to DICE 2.07.53
**   09-Feb-93 [lars]  Added check_signals() and default break handling.
*/

#include <sys/types.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <devices/timer.h>
#ifdef INCLUDE_VERSION
#include <dos/dos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#else
#include <libraries/dos.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include "nsignal.h"
#undef signal			/* This macro is implemented here! */

/*-----------------------------------------------------------------------*/

extern __regargs __geta4 ULONG catch_exception(__D0 ULONG);

ULONG sys_signal_alarm = 0,	/* The system-signal-masks */
      sys_signal_hup = 0, sys_signal_usr = 0;

void (*handler_hup) (void), (*handler_alarm) (void), (*handler_usr) (void);

static struct timerequest *treq = NULL;	/* the alarm()-timer */

/*-----------------------------------------------------------------------
** int start_timer (struct timeval *tv, struct timerequest *tr)
**
**   Start a timer <tr> with given timeval <tv>.
*/

__stkargs int start_timer(struct timeval * tv, struct timerequest * tr)
{
    if (!tr) {
	printf("no request structure\n");
	return 0;
    }
    if (tv->tv_secs == 0L && tv->tv_micro < 2L)
	tv->tv_micro = 2L;	/* minimal delay */

    tr->tr_time = *tv;
    tr->tr_node.io_Command = TR_ADDREQUEST;

    SendIO((struct IORequest *) tr);
    return 1;
}

/*-----------------------------------------------------------------------
** int setup_timer (LONG unit, struct timerequest **tr)
**
**   Setup a timer counting in <unit>, and store it in <tr>.
*/

__stkargs int setup_timer(LONG unit, struct timerequest ** tr)
{
    struct MsgPort *timerport;
    struct timerequest *req;

    if (*tr)
	return 1;

    if (!(timerport = (struct MsgPort *) CreatePort(0L, 0L))) {
	*tr = NULL;
	printf("setup_timer: could not create port\n");
	return 0;
    }
    if (!(req = (struct timerequest *) CreateExtIO(timerport
						,sizeof(struct timerequest))
	)) {
	DeletePort(timerport);
	*tr = NULL;
	printf("setup_timer: could not get request\n");
	return 0;
    }
    if (OpenDevice(TIMERNAME, unit, (struct IORequest *) req, 0L)) {
	CloseDevice((struct IORequest *) req);
	DeleteExtIO((struct IORequest *) req);
	DeletePort(timerport);
	printf("setup_timer: could not open timer\n");
	*tr = NULL;
	return 0;
    }
    *tr = req;
    return 1;
}

/*-----------------------------------------------------------------------
** void cleanup_timer (struct timerequest **tr)
**
**   Cleanup given timer <tr>.
*/

__stkargs void cleanup_timer(struct timerequest ** tr)
{
    struct MsgPort *tp;
    struct timerequest *tmp;
    UBYTE pFlags;

    if (*tr) {
	tmp = *tr;
	tp = tmp->tr_node.io_Message.mn_ReplyPort;
	if (tp) {
	    /* abort the current request */
	    pFlags = tp->mp_Flags;	/* still needed for DeletePort */
	    tp->mp_Flags = PA_IGNORE;
	    AbortIO((struct IORequest *) tmp);
	    WaitIO((struct IORequest *) tmp);
	    while (GetMsg(tp));
	    Forbid();
	    tp->mp_Flags = pFlags;
	    DeletePort(tp);
	    Permit();
	}
	CloseDevice((struct IORequest *) tmp);
	DeleteExtIO((struct IORequest *) tmp);
    }
    *tr = NULL;
}

/*-----------------------------------------------------------------------
** __stkargs int alarm (int seconds)
**
**   Start a timer which raises SIGALRM after <seconds>.
**   Also cleans up the mess made by a previous alarm.
**   Specifying a zero time count just cleans up.
*/

__stkargs unsigned int alarm(unsigned int seconds)
{
    static struct timeval tv;
    static first = 1;

    if (!treq) {
	printf("No handler installed !\n");
	if (seconds > 0)
	    return 0;		/* Heartbeat won't work :+( */
    }
    tv.tv_secs = seconds;
    tv.tv_micro = 0;

    if (seconds > 0) {
	/* first call of alarm() : WaitIO on unsent request ..... */
	if (!first) {
	    treq->tr_node.io_Message.mn_ReplyPort->mp_Flags = PA_IGNORE;
	    AbortIO((struct IORequest *) treq);
	    WaitIO((struct IORequest *) treq);
	    treq->tr_node.io_Message.mn_ReplyPort->mp_Flags = PA_SIGNAL;
	}
	first = 0;
	start_timer(&tv, treq);
    } else {
	/*
	 * if I don't use this code, AbortIO will generate a signal, which
	 * will trigger catch_alarm. catch_alarm will then generate CTRL-E.
	 * This can be resolved by preventing the signal to occur :+)
	 */
	treq->tr_node.io_Message.mn_ReplyPort->mp_Flags = PA_IGNORE;
	AbortIO((struct IORequest *) treq);
	WaitIO((struct IORequest *) treq);
	cleanup_timer(&treq);
	first = 1;
    }
    return 0;
}


/*-----------------------------------------------------------------------
** __sigfunc new_signal (int signo, __sigfunc handler)
**
**   Set the <handler> for <signo>.
**   The signals SIGHUP, SIGUSR1 and SIGALRM are treated manually to
**   allow the system-signals call the handlers via external exceptions.
*/

__stkargs __sigfunc new_signal(int signo, __sigfunc handler)
{
    register struct Task *this_task;

    this_task = (struct Task *) FindTask(NULL);

    switch (signo) {
    case SIGALRM:{
	    ULONG sigalrm;

	    sigalrm = sys_signal_alarm;
	    if ((__sigfunc) handler == SIG_IGN) {	/* remove SIGALRM
							 * handler */
		SetExcept(0L, sigalrm);	/* Only sigalrm !! */
		sys_signal_alarm = 0;
		handler_alarm = NULL;
		cleanup_timer(&treq);
	    } else {		/* install handler */
		if (!setup_timer(UNIT_VBLANK, &treq)) {
		    printf("Could not setup_timer\n");
		    break;	/* What else ?? */
		}
		sigalrm = 1L << (treq->tr_node.io_Message.mn_ReplyPort->mp_SigBit);

		this_task->tc_ExceptCode = (APTR) catch_exception;
		sys_signal_alarm = sigalrm;
		handler_alarm = (void (*) ()) handler;
		SetExcept(sigalrm, sigalrm);
		/* If we start treq, handler will be called */
	    }
	    break;
	}
    case SIGHUP:{
	    ULONG sighup;

	    sighup = (((__sigfunc) handler == SIG_IGN) || ((__sigfunc) handler == SIG_DFL))
		? 0 : EXT_SIGHUP;

	    this_task->tc_ExceptCode = (APTR) catch_exception;
	    sys_signal_hup = sighup;
	    handler_hup = (void (*) ()) handler;
	    SetExcept(sighup, EXT_SIGHUP);
	    break;
	}
    case SIGUSR1:{
	    ULONG sigusr;

	    sigusr = (((__sigfunc) handler == SIG_IGN) || ((__sigfunc) handler == SIG_DFL))
		? 0 : EXT_SIGUSR;

	    this_task->tc_ExceptCode = (APTR) catch_exception;
	    sys_signal_usr = sigusr;
	    handler_usr = (void (*) ()) handler;
	    SetExcept(sigusr, EXT_SIGUSR);
	    break;
	}
    default:
	signal(signo, handler);
	break;
    }
    return handler;
}

/*-----------------------------------------------------------------------
** ULONG check_signals (void)
**
**   Check the tasks external signals and call the associated handler
**   (if any).
**   Result is the signal mask.
*/

static int _ChkSignalLockout = 0;	/* simple semaphore for
					 * check_signals() */

ULONG check_signals(void)
{
    ULONG mask;

    if (_ChkSignalLockout)
	return 0L;
    ++_ChkSignalLockout;

    mask = ((struct Task *) FindTask(NULL))->tc_SigRecvd;

    /* Default Ctrl-C handling */
    if (!sys_signal_hup && (mask & SIGBREAKF_CTRL_C)) {
	SetSignal(0L, SIGBREAKF_CTRL_C);
	write(2, "*** Break.\n", 11);
	exit(EXIT_FAILURE);
    }
    /* Handle our special exceptions */
    if (mask & sys_signal_alarm) {
	(*handler_alarm) ();
	SetSignal(0L, sys_signal_alarm);
    }
    if (mask & sys_signal_hup) {
	(*handler_hup) ();
	SetSignal(0L, sys_signal_hup);
    }
    if (mask & sys_signal_usr) {
	(*handler_usr) ();
	SetSignal(0L, sys_signal_usr);
    }
    --_ChkSignalLockout;
    return mask;
}

/*************************************************************************/
