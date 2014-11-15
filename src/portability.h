/*
 * portability.h: global portability defines.
 */

#ifndef PORTABILITY_H
#define PORTABILITY_H

/* some platform don't have sighandler_t */
#ifndef HAVE_SIGHANDLER_T
typedef void (*sighandler_t)(int);
#endif

/* CYGWIN build is working. - sunyc@2013-11-12 */
#ifdef __CYGWIN__
#undef WINNT
#undef WIN95
#undef WIN98
#undef WINSOCK
#undef WIN32
/* CYGWIN needs this to proper setup POSIX env. */
#undef __STRICT_ANSI__
#endif

#endif /* _PORT_H */
