/* hosts/amiga/socket.h */

#if defined(AMIGA_TCP) || defined(AMITCP)
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#ifdef AMIGA_TCP
#include "socket_tcp.h"
#elif defined(AMITCP)
#include "socket_amitcp.h"
#else
#include "socket_sim.h"
#endif
