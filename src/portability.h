/*
 * portability.h: global portability #defines for MudOS, an LPmud driver
 *
 * If you have to have to change this file to get MudOS to compile and
 * run on your system, please provide us wth a copy of your modified 
 * portability.h file and the name of the type of system you are using.
 */

/* NOTE: Do not put anything into this file other than preprocessor
   directives (#define, #ifdef, #ifndef, etc).  If you must put something
   other than that, then be sure to put it between #ifdef _FUNC_SPEC_
   and #endif.  Otherwise make_func will break.
*/

#ifndef PORTABILITY_H
#define PORTABILITY_H

/* define this if you have an Ultrix system that the driver won't otherwise
   compile on (doesn't seem to be needed for DECstations).
*/
#undef OLD_ULTRIX

/* define this if you're using HP-UX 7.x (or below?) */
#undef OLD_HPUX

/* hack to figure out if we are being compiled under Solaris or not */
/* newer versions of gcc are smart enough to tell us */
#if defined(sun) && !defined(SunOS_5)
#  if defined(__svr4__) || defined(__sol__) || defined(SVR4)
#    define SunOS_5
#  else
#    define SunOS_4
#  endif
#endif

/* evidentally, gcc on OS/2 defines virtually nothing of use? */
#ifdef __EMX__
#  define OS2
#endif

#if defined(SunOS_5) && !defined(SVR4)
#  define SVR4
#endif

/* "hp68k" refers to Motorola 680xx HP's running BSD - not sure if they
   also can run HP-UX or not, but to be sure we check ....
    -bobf
*/
#if (defined(hp200) || defined(hp300) || defined(hp400) || defined(hp500)) \
        && !defined(hpux)
#  define hp68k
#  define _ANSI_H
#endif

/* I hear that gcc defines one of these, cc defines the other (w/ OSF/1 1.2)
    -bobf
*/
#if defined(__osf__) || defined(__OSF__)
#  define OSF
#endif

#if !defined(AMIGA) && ( \
    ((defined(LATTICE) || defined(__SASC)) && defined(_AMIGA)) || \
    (defined(AZTEC_C) && defined(MCH_AMIGA)) )
#  define AMIGA
#endif

/* HAS_MONCONTROL: define this if you wish to do profiling of the driver
   on a machine that has the moncontrol() system call.
*/
#if defined(NeXT)
#  define HAS_MONCONTROL
#endif

/* define this if your builtin version of inet_ntoa() works well.  It has a
 * problem on some sun 4's (SPARCstations) (if the driver crashes at
 * startup you might try undefining INET_NTOA_OK).
 * NOTE: you must define this when compiling on a NeXT or an RS/6000.
 */
#if (!defined(sparc))
#  define INET_NTOA_OK
#endif

/* Define what random number generator to use.
 * If no one is specified, a guaranteed bad one will be used.
 * use drand48 if you have it (it is the better random # generator)
 */

#if defined(NeXT) || defined(__386BSD__) || defined(hp68k) || \
        defined(__bsdi__) || defined(sequent) || defined(OS2) || defined(linux)
#  define RANDOM
#else				/* Sequent, HP, Sparc, RS/6000 */
#  define DRAND48
#endif

/*
 * Does the system have a getrusage() system call?
 * Sequent doesn't have it.  Solaris 2.1 (SunOS 5.1) has it in a compat
 * library but had trouble making it work correctly.
 */
#if (!defined(_SEQUENT_) && !defined(SVR4) && !defined(LATTICE) \
     && !defined(_AUX_SOURCE) && !defined(cray) && !defined(OLD_HPUX) \
     && !defined(_M_UNIX)) && !defined(OS2)
#  define RUSAGE
#endif

/* the !defined(_FUNC_SPEC) is needed to allow make_func to work okay. */
#if defined(hpux) && !defined(OLD_HPUX) && !defined(_FUNC_SPEC_)
#  include <sys/syscall.h>
#  define getrusage(a, b) syscall(SYS_GETRUSAGE, (a), (b))
#endif

/*
 * Dynix/ptx has a system-call similar to rusage().
 */
#ifdef _SEQUENT_
#  define GET_PROCESS_STATS
#endif

/*
 * Does the system have the times() system call?  Is only used if RUSAGE not
 * defined.
 */
#if defined(hpux) || defined(apollo) || defined(__386BSD__) || \
        defined(SVR4) || defined(_SEQUENT_) || defined(_AUX_SOURCE) || \
        defined(cray) || defined(SunOS_5) || defined(_M_UNIX)
#  define TIMES
#endif

/*
 * Define SYSV if you are running System V with a lower release level than
 * System V Release 4.
 */
#if (defined(_SEQUENT_))
#  define SYSV
#endif

/* define HAS_UALARM if ualarm() system call is available (or if ualarm.c
   will work)
*/
#if !(defined(SYSV) || defined(SVR4) || defined(cray) || defined(LATTICE)) \
    || defined(OS2)
#  define HAS_UALARM
#endif

/*
 * Define this if your operating system supports the gettimeofday() system
 * call.
 */
#if !defined(_SEQUENT_) && !defined(LATTICE)
#  define HAS_GETTIMEOFDAY
#endif

/*
 * Most implementation of System V Release 3 do not provide Berkeley signal
 * semantics by default.  Instead, POSIX signals are provided.  If your
 * implementation is System V Release 3 and you do not have Berkeley signals,
 * but you do have POSIX signals, then #define USE_POSIX_SIGNALS.
 */
#if (defined(_SEQUENT_) || defined(_M_UNIX))
#  define USE_POSIX_SIGNALS
#endif

/*
 * Define FCHMOD_MISSING only if your system doesn't have fchmod().
 */
/* HP, Sequent, NeXT, Sparc all have fchmod() */
#if defined(cray) || defined(LATTICE) || defined(_AIX) || defined(_M_UNIX) \
    || defined(OS2)
#  define FCHMOD_MISSING
#else
#  undef FCHMOD_MISSING
#endif

/*
 * Define HAS_SETDTABLESIZE if your system has getdtablesize()/setdtablesize().
 * If defined setdtablesize() is used to request the appropriate number of file
 * descriptors for the current configuration.
 *
 * NeXT and Sparc don't have it.
 *
 * Amiga's AmiTCP does but has too many usage constraints.
 */
#if (defined(_SEQUENT_))
#  define HAS_SETDTABLESIZE
#endif

/* undefine this if your system doesn't have unsigned chars */
/* NeXT, Sparc, HP, Sequent, and RS/6000 all have unsigned chars */
#define HAS_UNSIGNED_CHAR

/* SIGNAL_ERROR:
   look in /usr/include/signal.h for the return type of signal() when an
   error occurs
*/
#if defined(NeXT) || defined(accel) || defined(apollo) || \
    (defined(__386BSD__) && !(defined(__FreeBSD__) || defined(__NetBSD__))) \
    || defined(hp68k) || defined(sequent) || defined(_CX_UX)
#  define SIGNAL_ERROR BADSIG
#else
#  define SIGNAL_ERROR SIG_ERR
#endif

#define SIGNAL_FUNC_TAKES_INT defined(_AIX) || defined(NeXT) \
    || defined(_SEQUENT_) || defined(SVR4) \
    || defined(__386BSD__) || defined(apollo) || defined(cray) \
    || defined(SunOS_5) || defined(__bsdi__) || defined(linux)

/*
Define MEMPAGESIZE to be some value if you wish to use BSDMALLOC _and_ your
system does not support the getpagesize() system call.  This page size
should be terms of the number of bytes in a page of system memory (not
necessarily the same as the hardware page size).  You may be able to
ascertain the correct value by searching your /usr/include files or
asking your system adminstrator.
*/
#if !defined(linux) && !defined(__386BSD__) && !defined(__FreeBSD__) \
	&& !defined(OSF) && !defined(SunOS_4) && !defined(sgi)
#if defined(hpux) || defined(SunOS_5)
#  define MEMPAGESIZE sysconf(_SC_PAGE_SIZE)
#else
/* taken from smalloc.c's CHUNK_SIZE */
#  define MEMPAGESIZE 0x40000
#endif				/* hpux */
#endif				/* linux */

/* define this if you system is BSD 4.2 (not 4.3) */
#undef BSD42

/* otherwise make_func will break */
#ifndef _FUNC_SPEC_
/* these should be obtained from .h files when Linux .h structure stabilizes */
#if defined(linux) || defined(_M_UNIX)
/* This has FD_SETSIZE on some Linux's (Linuces?) */
#include <sys/time.h>
#ifndef SOMAXCONN
#define SOMAXCONN 5
#endif
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif
#endif
#endif

#if defined(cray) && !defined(MAXPATHLEN)
#define MAXPATHLEN PATH_MAX
#endif

#if defined(_M_UNIX) && !defined(MAXPATHLEN)
#define MAXPATHLEN 1024
#endif

/* UINT32 should be typedef'd to whatever type provides an unsigned 32-bit
   integer type
*/
#ifndef _FUNC_SPEC_
#if defined(__alpha)
typedef unsigned int UINT32;
#else
typedef unsigned long UINT32;
#endif
#endif

/*
 * memmove()
 *  - for the few systems without [working] bcopy(); not sure why some
 *    gcc users don't have this in their c.lib...it's ANSI C (see K&R)
 */
#if !defined(LATTICE) && !defined(SunOS_5) && !defined(OSF) && \
    !defined(_AIX) && !defined(linux) && !defined(ultrix) && \
    !defined(hpux) && !defined(sgi) && !defined(NeXT) && \
    !defined(_CX_UX)
#if defined(_AUX_SOURCE)
#define MEMMOVE_MISSING
#else
/* sunos 4.x, msdos */
#define memmove(a,b,c) bcopy(b,a,c)
#endif
#endif

/*
 * stdarg.h (ANSI C) vs varargs.h (defacto Unix standard)
 */
#if defined(LATTICE) 
#define HAS_STDARG_H
#endif

#define SIZEOF_SHORT     2
#define SIZEOF_INT       4
#define SIZEOF_FLOAT     4
#ifndef __alpha
#define SIZEOF_PTR       4
#else
#define SIZEOF_PTR       8
#endif
#endif				/* _PORT_H */
