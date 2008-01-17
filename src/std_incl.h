#if !defined(STD_INCL_H) && !defined(_FUNC_SPEC_)
#define STD_INCL_H

#ifdef INCL_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef INCL_UNISTD_H
#  include <unistd.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#ifdef INCL_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef INCL_TIME_H
#  include <time.h>
#endif
#ifdef INCL_SYS_TIME_H
#  include <sys/time.h>
#endif
#ifdef INCL_DOS_H
#  include <dos.h>
#endif
#ifdef INCL_SYS_TIMES_H
#  include <sys/times.h>
#endif
#ifdef INCL_USCLKC_H
#  include <usclkc.h>
#endif

#ifdef INCL_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#ifdef INCL_SYS_CRYPT_H
#  include <sys/crypt.h>
#endif
#ifdef INCL_CRYPT_H
#  include <crypt.h>
#endif
#ifdef INCL_VALUES_H
#  include <values.h>
#endif
#ifdef INCL_LIMITS_H
#  include <limits.h>
#endif
#ifdef INCL_MALLOC_H
#  include <malloc.h>
#endif
#ifdef INCL_LOCALE_H
#  include <locale.h>
#endif

#ifdef INCL_DLFCN_H
#  include <dlfcn.h>
#endif

#ifndef _AUX_SOURCE
/* supposedly, memmove is in here, but not defined in any lib */
#  include <string.h>
#endif

#ifndef NO_SOCKETS
#  ifdef INCL_NETINET_IN_H
#    include <netinet/in.h>
#  endif
#  ifdef INCL_ARPA_INET_H
#    include <arpa/inet.h>
#  endif
#endif

#if defined(WIN32)
int dos_style_link (char *, char *);
#define link(x, y) dos_style_link(x, y)
#endif

#ifdef NeXT
#  include <libc.h>
#endif

#ifdef WIN32
#  include <sys/timeb.h>
#endif

#if !defined(NeXT) && !defined(WIN32)
#  include <sys/param.h>
#endif

#ifdef OLD_ULTRIX
#  include <stddef.h>
#  define size_t unsigned
#endif

#ifdef INCL_MACH_MACH_H
#  include <mach/mach.h>
#endif

#ifdef INCL_MACH_H
#  include <mach.h>
#endif

#ifdef sun
#  include <alloca.h>
#endif

#include <stdarg.h>

#ifdef INCL_BSTRING_H
#include <bstring.h>
#endif

/* Note: This is now only used if _both_ USHRT_MAX and MAXSHORT fail to exist*/
#ifndef USHRT_MAX
#define USHRT_MAX ((1 << (sizeof(short)*8)) - 1)
#endif

#endif
