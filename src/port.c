#include "std.h"
#include "port.h"
#include "lint.h"
#include "file_incl.h"
#include "network_incl.h"

#if defined(WIN32) || defined(LATTICE)
int dos_style_link P2(char *, x, char *, y) {
    char link_cmd[100];
    sprintf(link_cmd, "copy %s %s", x, y);
    return system(link_cmd);
}
#endif

/* for get_cpu_times() */
#ifdef GET_PROCESS_STATS
#include <sys/procstats.h>
#endif
#ifdef RUSAGE
#include <sys/resource.h>
#endif

#ifdef sun
time_t time PROT((time_t *));
#endif

/*
 * Return a pseudo-random number in the range 0 .. n-1
 */
int random_number P1(int, n)
{
#ifdef RAND
    static char called = 0;

    if (!called) {
	time_t tim;

	time(&tim);
	srand(tim);
	called = 1;
    }				/* endif */
    return rand() % n;
#else
#ifdef RANDOM
    return random() % n;
#else				/* RANDOM */
#ifdef DRAND48
    return (int) (drand48() * n);
#else				/* DRAND48 */
    return current_time % n;	/* You really don't want to use this method */
#endif				/* DRAND48 */
#endif				/* RANDOM */
#endif				/* RAND */
}

/*
 * The function time() can't really be trusted to return an integer.
 * But MudOS uses the 'current_time', which is an integer number
 * of seconds. To make this more portable, the following functions
 * should be defined in such a way as to return the number of seconds since
 * some chosen year. The old behaviour of time(), is to return the number
 * of seconds since 1970.
 */

int get_current_time()
{
    return (int) time(0l);	/* Just use the old time() for now */
}

char *time_string P1(int, t)
{
    return (char *) ctime((time_t *) & t);
}

/*
 * Initialize the microsecond clock.
 */
void init_usec_clock()
{
#ifdef _SEQUENT_
    usclk_init();
#endif
}

/*
 * Get a microsecond clock sample.
 */
void
get_usec_clock P2(long *, sec, long *, usec)
{
#ifdef HAS_GETTIMEOFDAY
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *sec = tv.tv_sec;
    *usec = tv.tv_usec;
#else
#ifdef _SEQUENT_
           *sec = 0;
    *usec = GETUSCLK();
#else
#ifdef LATTICE
    unsigned int clock[2];

    if (timer(clock)) {
	*sec = time(0);
	*usec = 0;
    } else {
	*sec = clock[0];
	*usec = clock[1];
    }
#else
    *sec = time(0);
    *usec = 0;
#endif
#endif
#endif
}

#ifdef USE_POSIX_SIGNALS
int
port_sigblock P1(sigset_t, mask)
{
    sigset_t omask;

    sigprocmask(SIG_BLOCK, &mask, &omask);
    return (omask);
}

int
port_sigmask P1(int, sig)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, sig);
    return (set);
}

void
     (*port_signal(sig, func)) ()
    int sig;
    void (*func) ();
{
    struct sigaction act, oact;

    act.sa_handler = func;
    act.sa_mask = 0;
    act.sa_flags = 0;
    if (sigaction(sig, &act, &oact) == -1)
	return ((void (*) ()) -1);
    return (oact.sa_handler);
}

int
port_sigsetmask P1(sigset_t, mask)
{
    sigset_t omask;

    sigprocmask(SIG_SETMASK, &mask, &omask);
    return (omask);
}
#endif

int
get_cpu_times P2(unsigned long *, secs, unsigned long *, usecs)
{
#ifdef RUSAGE
    struct rusage rus;
#endif
#if defined(TIMES) && !defined(RUSAGE)
    struct tms t;
    unsigned long total;
#endif
#ifdef GET_PROCESS_STATS
    struct process_stats ps;
#endif

#ifdef RUSAGE			/* start RUSAGE */
    if (getrusage(RUSAGE_SELF, &rus) < 0) {
	return 0;
    }
    *secs = rus.ru_utime.tv_sec + rus.ru_stime.tv_sec;
    *usecs = rus.ru_utime.tv_usec + rus.ru_stime.tv_usec;
    return 1;
#else				/* end then RUSAGE */

#ifdef GET_PROCESS_STATS	/* start GET_PROCESS_STATS */
    if (get_process_stats(NULL, PS_SELF, &ps, NULL) == -1) {
	return 0;
    }
    *secs = ps.ps_utime.tv_sec + ps.ps_stime.tv_sec;
    *usecs = ps.ps_utime.tv_usec + ps.ps_stime.tv_usec;
    return 1;
#else				/* end then GET_PROCESS_STATS */

#ifdef TIMES			/* start TIMES */
    times(&t);
    *secs = (total = t.tms_utime + t.tms_stime) / CLK_TCK;
    *usecs = ((total - (*secs * CLK_TCK)) * 1000000) / CLK_TCK;
    return 1;
#else				/* end then TIMES */

#ifdef LATTICE			/* start LATTICE */
    unsigned int clock[2];

    if (timer(clock))
	return 0;

    *secs = clock[0];
    *usecs = clock[1];
    return 1;
#else
    return 0;
#endif				/* end LATTICE */
#endif				/* end TIMES */
#endif				/* end else GET_PROCESS_STATS */
#endif				/* end else RUSAGE */
}

/* return the current working directory */
char *
     get_current_dir P2(char *, buf, int, limit)
{
#ifdef HAS_GETCWD
    return getcwd(buf, limit);	/* POSIX */
#else
    return getwd(buf);		/* BSD */
#endif
}

#ifndef HAS_STRERROR
/* for those systems without strerror() but with sys_errlist, sys_nerr */
/* Warning: Sun has a prototype for strerror, but no definition for it,
   so we can't use that name */
extern char *sys_errlist[];
extern int sys_nerr;

char *port_strerror P1(int, which)
{
    if ((which < 0) || (which >= sys_nerr)) {
	return "unknown error";
    } else {
	return sys_errlist[which];
    }
}
#endif				/* STRERROR */

#ifdef MEMMOVE_MISSING
/* for those without memmove() and a working bcopy() that can handle overlaps */
INLINE char *memmove P3(register char *, b, register char *, a, register int, s)
{
    char *r = b;

    if (b < a) {
	while (s--)
	    *(b++) = *(a++);
    } else if (b > a) {
	b += s;
	a += s;
	while (s--)
	    *(--b) = *(--a);
    }
    return r;
}
#endif

#if 0
#ifdef WIN32
char *crypt P2(char *, str, char *, salt)
{
    static char buf[50];

    strncpy(buf, str, 50);
    buf[50] = 0;
    return buf;
}				/* crypt() */
#endif
#endif

#ifdef WIN32
char *WinStrError(int err) {
    static char buf[30];
    if (errno < 10000) return strerror(err);
    sprintf(buf, "error #%d", err);
    return buf;
}
#endif
