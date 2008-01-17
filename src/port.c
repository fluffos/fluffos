#include "std.h"
#include "port.h"
#include "lint.h"
#include "file_incl.h"
#include "network_incl.h"
#include <unistd.h>
#ifndef MINGW
#include <sys/mman.h>
#endif
#if defined(WIN32) 
int dos_style_link (char * x, char * y) {
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
time_t time (time_t *);
#endif

/*
 * Return a pseudo-random number in the range 0 .. n-1
 */
long random_number (long n)
{
#if defined(RAND) || defined(DRAND48)
    static char called = 0;

    if (!called) {
	time_t tim;

	time(&tim);
#  ifdef RAND
	srand(tim);
#  else
	srand48(tim);
#  endif
	called = 1;
    }				/* endif */
#  ifdef RAND
#    ifdef MINGW
    return rand() % n;
#    else
    return 1 + (long) ((float)n * rand() / (RAND_MAX+1.0);
#endif
#  else
    return (long)(drand48() * n);
#  endif
#else
#  ifdef RANDOM
    return random() % n;
#  else				/* RANDOM */
    return current_time % n;	/* You really don't want to use this method */
#  endif			/* RANDOM */
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

long get_current_time()
{
    return time(0l);	/* Just use the old time() for now */
}

char *time_string (time_t t)
{
    char *res = ctime(&t);
    if(!res)
      res = "ctime failed";
    return res;
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
get_usec_clock (long * sec, long * usec)
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
    *sec = time(0);
    *usec = 0;
#endif
#endif
}

#ifdef USE_POSIX_SIGNALS
int
port_sigblock (sigset_t mask)
{
    sigset_t omask;

    sigprocmask(SIG_BLOCK, &mask, &omask);
    return (omask);
}

int
port_sigmask (int sig)
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
port_sigsetmask (sigset_t mask)
{
    sigset_t omask;

    sigprocmask(SIG_SETMASK, &mask, &omask);
    return (omask);
}
#endif

int
get_cpu_times (unsigned long * secs, unsigned long * usecs)
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

    return 0;
#endif				/* end TIMES */
#endif				/* end else GET_PROCESS_STATS */
#endif				/* end else RUSAGE */
}

/* return the current working directory */
char *
     get_current_dir (char * buf, int limit)
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

char *port_strerror (int which)
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
INLINE char *memmove (register char * b, register char * a, register int s)
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

#ifdef WIN32
char *WinStrError(int err) {
    static char buf[30];
    if (errno < 10000) return strerror(err);
    sprintf(buf, "error #%d", err);
    return buf;
}
#endif

#ifdef MMAP_SBRK
void * sbrkx(long size){
  static void *end = 0;
  static unsigned long tsize = 0;
  void *tmp, *result;
  long long newsize;
  if(!end){
    tmp = mmap((void *)0x41000000, 4096, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS,0,0);
    tsize=4096;
    end=(void *)0x41000000;
    if(tmp != end)
      return NULL;
  }
  
  newsize = (long)end + size;
  result = end;
  end = newsize;
  newsize &= 0xFFFFF000;
  newsize += 0x1000;
  newsize -= 0x41000000;

  tmp = mremap((void *)0x41000000, tsize, newsize, 0);

  if(tmp != (void *) 0x41000000)
    return NULL;
  
  tsize = newsize;
  return result;
}

#else

void *sbrkx(long size){
#ifndef MINGW
  return sbrk(size);
#endif
}
#endif
