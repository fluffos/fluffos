/*
 * portability.h: global portability #defines for MudOS, an LPmud driver
 *
 * If you have to have to change this file to get MudOS to compile and
 * run on your system, please provide us wth a copy of your modified 
 * portability.h file and the name of the type of system you are using.
 */

/* NOTE: Do not put anything into this file other than preprocessor
   directives (#define, #ifdef, #ifndef, etc).  If you must put something
   other than that, then be sure to put it between #ifndef _FUNC_SPEC_
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

#if defined(WINNT) || defined(WIN95)
#  ifndef WIN32
#    define WIN32
#      ifndef _FUNC_SPEC_
         typedef char * caddr_t;
#      endif
#  endif
#  define symlink(x, y) dos_style_link(x, y)
#  define CDECL __cdecl
#else
#  define CDECL
#endif

#ifdef WIN32
#  define WINSOCK
#  ifdef sun
#    undef sun
#  endif
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
 * Most implementation of System V Release 3 do not provide Berkeley signal
 * semantics by default.  Instead, POSIX signals are provided.  If your
 * implementation is System V Release 3 and you do not have Berkeley signals,
 * but you do have POSIX signals, then #define USE_POSIX_SIGNALS.
 */
#if (defined(_SEQUENT_) || defined(_M_UNIX))
#  define USE_POSIX_SIGNALS
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

/* Should be a configure check */
#define SIGNAL_FUNC_TAKES_INT defined(_AIX) || defined(NeXT) \
    || defined(_SEQUENT_) || defined(SVR4) \
    || defined(__386BSD__) || defined(apollo) || defined(cray) \
    || defined(SunOS_5) || defined(__bsdi__) || defined(linux)

/* Define MEMPAGESIZE to be some value if you wish to use BSDMALLOC
 * _and_ your system does not support the getpagesize() system call.
 * This page size should be terms of the number of bytes in a page of
 * system memory (not necessarily the same as the hardware page size).
 * You may be able to ascertain the correct value by searching your
 * /usr/include files or asking your system adminstrator.  
 *
 * FIXME: configure should check for getpagesize().
 */
#if !defined(linux) && !defined(__386BSD__) && !defined(__FreeBSD__) \
	&& !defined(OSF) && !defined(SunOS_4) && !defined(sgi)
#  ifdef _SC_PAGE_SIZE
#    define MEMPAGESIZE sysconf(_SC_PAGE_SIZE)
#  else
#    ifdef _SC_PAGESIZE
#      define MEMPAGESIZE sysconf(_SC_PAGESIZE)
#    else
       /* taken from smalloc.c's CHUNK_SIZE */
#      define MEMPAGESIZE 0x40000
#    endif
#  endif
#endif

/* define this if you system is BSD 4.2 (not 4.3) */
#undef BSD42

/* otherwise make_func will break */
#ifndef _FUNC_SPEC_
/* these should be obtained from .h files when Linux .h structure stabilizes */
#  if defined(linux) || defined(_M_UNIX)
/* This has FD_SETSIZE on some Linux's (Linuces?) */
#    include <sys/time.h>
#    include <sys/socket.h>
#    ifndef FD_SETSIZE
#      define FD_SETSIZE 256
#    endif
#  endif
#endif

#if defined(cray) && !defined(MAXPATHLEN)
#  define MAXPATHLEN PATH_MAX
#endif

#if defined(WIN32) && !defined(MAXPATHLEN)
#  define MAXPATHLEN 512
#endif

#if defined(_M_UNIX) && !defined(MAXPATHLEN)
#  define MAXPATHLEN 1024
#endif

/*
 * memmove()
 *  - for the few systems without [working] bcopy(); not sure why some
 *    gcc users don't have this in their c.lib...it's ANSI C (see K&R)
 */
#ifdef USE_BCOPY
/* sunos 4.x, msdos */
#define memmove(a,b,c) bcopy(b,a,c)
#endif

#if defined(sun) && !defined(SunOS_5)
#  define OLDCRYPT(x, y) _crypt(x, y)
#else
#  define OLDCRYPT(x, y) crypt(x, y)
#endif

#ifdef CUSTOM_CRYPT
#  define CRYPT(x, y) custom_crypt(x, y, 0)
#else
#  define CRYPT(x, y) OLDCRYPT(x, y)
#endif

#ifdef HAS_STRERROR
#  define port_strerror strerror
#endif

#ifdef WINSOCK
/* Windows stuff */

#  define WINSOCK_NO_FLAGS_SET  0

#  define OS_socket_write(f, m, l) send(f, m, l, WINSOCK_NO_FLAGS_SET)
#  define OS_socket_read(r, b, l) recv(r, b, l, WINSOCK_NO_FLAGS_SET)
#  define OS_socket_close(f) closesocket(f)
#  define OS_socket_ioctl(f, w, a) ioctlsocket(f, w, a)
#  define EWOULDBLOCK             WSAEWOULDBLOCK
#  define EINPROGRESS             WSAEINPROGRESS
#  define EALREADY                WSAEALREADY
#  define ENOTSOCK                WSAENOTSOCK
#  define EDESTADDRREQ            WSAEDESTADDRREQ
#  define EMSGSIZE                WSAEMSGSIZE
#  define EPROTOTYPE              WSAEPROTOTYPE
#  define ENOPROTOOPT             WSAENOPROTOOPT
#  define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#  define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#  define EOPNOTSUPP              WSAEOPNOTSUPP
#  define EPFNOSUPPORT            WSAEPFNOSUPPORT
#  define EAFNOSUPPORT            WSAEAFNOSUPPORT
#  define EADDRINUSE              WSAEADDRINUSE
#  define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#  define ENETDOWN                WSAENETDOWN
#  define ENETUNREACH             WSAENETUNREACH
#  define ENETRESET               WSAENETRESET
#  define ECONNABORTED            WSAECONNABORTED
#  define ECONNRESET              WSAECONNRESET
#  define ENOBUFS                 WSAENOBUFS
#  define EISCONN                 WSAEISCONN
#  define ENOTCONN                WSAENOTCONN
#  define ESHUTDOWN               WSAESHUTDOWN
#  define ETOOMANYREFS            WSAETOOMANYREFS
#  define ETIMEDOUT               WSAETIMEDOUT
#  define ECONNREFUSED            WSAECONNREFUSED
#  define ELOOP                   WSAELOOP
#  define EHOSTDOWN               WSAEHOSTDOWN
#  define EHOSTUNREACH            WSAEHOSTUNREACH
#  define EPROCLIM                WSAEPROCLIM
#  define EUSERS                  WSAEUSERS
#  define EDQUOT                  WSAEDQUOT
#  define ESTALE                  WSAESTALE
#  define EREMOTE                 WSAEREMOTE
#  define socket_errno WSAGetLastError()
#  define socket_perror(x, y) SocketPerror(x, y)

#  define FOPEN_READ "rb"
#  define FOPEN_WRITE "wb"
#  define OPEN_WRITE (O_WRONLY | O_BINARY)
#  define OPEN_READ (O_RDONLY | O_BINARY)
#else
/* Normal UNIX */
#  define OS_socket_write(f, m, l) send(f, m, l, 0)
#  define OS_socket_read(r, b, l) recv(r, b, l, 0)
#  define OS_socket_close(f) close(f)
#  define OS_socket_ioctl(f, w, a) ioctl(f, w, (caddr_t)a)
#  define socket_errno errno
#  define socket_perror(x, y) debug_perror(x, y)
#  define INVALID_SOCKET -1
#  define SOCKET_ERROR -1

#  define FOPEN_READ "r"
#  define FOPEN_WRITE "w"
#  define OPEN_WRITE O_WRONLY
#  define OPEN_READ O_RDONLY
#endif

#ifndef EWOULDBLOCK
#ifdef EAGAIN
#define EWOULDBLOCK EAGAIN
#endif
#endif

#endif				/* _PORT_H */
