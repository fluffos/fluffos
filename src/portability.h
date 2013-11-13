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

/* Don't be fooled, windows support is broken, use cygwin instead,
   these are left here temporally for historical references.
   - sunyc@2013-05-09 */
#if defined(__WIN32__)
#define WINSOCK
#define CDECL __cdecl
#else
#define CDECL
#endif /* __WIN32__ */

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
#else /* Normal UNIX */
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

#endif              /* _PORT_H */
