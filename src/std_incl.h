#if !defined(STD_INCL_H) && !defined(_FUNC_SPEC_)
#define STD_INCL_H

#if defined(NeXT) || defined(__386BSD__) || defined(LATTICE) || defined(SunOS_5)
#  include <stdlib.h>
#endif
#if defined(__386BSD__) || defined(LATTICE) || defined(SunOS_5) || defined(sgi)
#  include <unistd.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#if !defined(LATTICE) && !defined(OS2) && !defined(_AUX_SOURCE)
#  include <fcntl.h>
#endif

#if !defined(hp68k)
#  include <time.h>
#endif
#ifdef LATTICE
#  include <dos.h>
#  undef VOID
#else
#  include <sys/time.h>
#  ifdef TIMES
#    include <sys/times.h>
#  endif
#endif
#ifdef _SEQUENT_
#  include <usclkc.h>
#endif

#ifndef _AUX_SOURCE
/* supposedly, memmove is in here, but not defined in any lib */
#  include <string.h>
#endif

#ifndef NO_SOCKETS
#if !defined(OS2)
#  include <netinet/in.h>
#  include <arpa/inet.h>
#endif
#if defined(LATTICE) || defined(__SASC)
#  include "amiga/amiga.h"
#  include "amiga/nsignal.h"
#  include "amiga/socket.h"
#endif
#endif

#if defined(LATTICE) || defined(OS2)
#define link(x,y) { \
    char link_cmd[100]; \
    sprintf(link_cmd, "copy %s %s", x, y); \
    system(link_cmd); \
}
#endif

#ifdef NeXT
#  include <libc.h>
#endif

#ifdef OS2
#  include <sys/timeb.h>
#endif

#if !defined(NeXT) && !defined(LATTICE)
#  include <sys/param.h>
#endif

#ifdef OLD_ULTRIX
#  include <stddef.h>
#  define size_t unsigned
#endif

#ifdef NeXT
#  include <mach.h>
#endif

#ifdef sun
#  include <alloca.h>
#endif

#ifdef HAS_STDARG_H
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#endif
