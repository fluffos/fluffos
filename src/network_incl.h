#ifndef NETWORK_INCL_H
#define NETWORK_INCL_H

#include "configure.h"

#ifdef INCL_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef INCL_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#ifdef INCL_NETDB_H
#  include <netdb.h>
#endif
#ifdef INCL_SYS_SEMA_H
#  include <sys/sema.h>
#endif

/* defined in <sys/sema.h> on HP-PA/RISC; causes problems with telnet */
#undef SE

#ifdef INCL_ARPA_TELNET_H
#  include <arpa/telnet.h>
#else
#  include "telnet.h"
#endif
#ifdef INCL_SYS_SOCKETVAR_H
#    include <sys/socketvar.h>
#endif
#ifdef INCL_SOCKET_H
#  include <socket.h>
#endif
#ifdef INCL_RESOLVE_H
#  include <resolv.h>
#endif

#ifdef WINSOCK
/* Windows stuff */
#  include <winsock.h>

#  define WINSOCK_NO_FLAGS_SET  0

#  define OS_socket_write(f, m, l) send(f, m, l, WINSOCK_NO_FLAGS_SET)
#  define OS_socket_read(r, b, l) recv(r, b, l, WINSOCK_NO_FLAGS_SET)
#  define OS_socket_close(f) closesocket(f)
#  define OS_socket_ioctl(f, w, a) ioctlsocket(f, w, a)
#else
/* Normal UNIX */
#  define OS_socket_write(f, m, l) write(f, m, l)
#  define OS_socket_read(r, b, l) read(r, b, l)
#  define OS_socket_close(f) close(f)
#  define OS_socket_ioctl(f, w, a) ioctl(f, w, (caddr_t)a)
#endif

#endif

