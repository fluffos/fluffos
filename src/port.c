#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#include "config.h"
#include "lint.h"

#ifdef sun
time_t time PROT((time_t *));
#endif


/*
 * This file defines things that may have to be changem when porting
 * LPmud to new environments. Hopefully, there are #ifdef's that will take
 * care of everything.
 */

/*
 * Return a pseudo-random number in the range 0 .. n-1
 */
int random_number(n)
    int n;
{
#ifdef RANDOM
    return random() % n;
#else /* RANDOM */
#ifdef DRAND48
    return (int)(drand48() * n);
#else /* DRAND48 */
    extern int current_time;

    return current_time % n; /* You really don't want to use this method */
#endif /* DRAND48 */
#endif /* RANDOM */
}

/*
 * The function time() can't really be trusted to return an integer.
 * But MudOS uses the 'current_time', which is an integer number
 * of seconds. To make this more portable, the following functions
 * should be defined in such a way as to return the number of seconds since
 * some chosen year. The old behaviour of time(), is to return the number
 * of seconds since 1970.
 */

int get_current_time() {
    return (int)time(0l);	/* Just use the old time() for now */
}

char *time_string(t)
    int t;
{
    return (char *)ctime((time_t *)&t);
}

#ifdef USE_POSIX_SIGNALS
int
port_sigblock(mask)
	sigset_t mask;
{
	sigset_t omask;
	sigprocmask(SIG_BLOCK, &mask, &omask);
	return (omask);
}

int
port_sigmask(sig)
	int sig;
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, sig);
	return (set);
}

void
(*port_signal(sig, func))()
	int sig;
	void (*func)();
{
	struct sigaction act, oact;
	act.sa_handler = func;
	act.sa_mask = 0;
	act.sa_flags = 0;
	if (sigaction(sig, &act, &oact) == -1)
		return ((void (*)())-1);
	return (oact.sa_handler);
}

int
port_sigsetmask(mask)
	sigset_t mask;
{
	sigset_t omask;
	sigprocmask(SIG_SETMASK, &mask, &omask);
	return (omask);
}
#endif
