#ifndef NETWORK_INCL_H
#define NETWORK_INCL_H

#ifdef INCL_SYS_TYPES_H
#  include <sys/types.h>
#endif
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
#  include <winsock.h>
#endif

#endif
