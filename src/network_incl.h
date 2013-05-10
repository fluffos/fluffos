#ifndef NETWORK_INCL_H
#define NETWORK_INCL_H

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_SYS_SEMA_H
#  include <sys/sema.h>
#endif

/* defined in <sys/sema.h> on HP-PA/RISC; causes problems with telnet */
#undef SE

#ifdef HAVE_ARPA_TELNET_H
#  include <arpa/telnet.h>
#endif
#ifdef HAVE_SYS_SOCKETVAR_H
#    include <sys/socketvar.h>
#endif
#ifdef HAVE_SOCKET_H
#  include <socket.h>
#endif
#ifdef HAVE_RESOLVE_H
#  include <resolv.h>
#endif

#ifdef WINSOCK
#  include <winsock2.h>
#endif

#endif
