/* hosts/amiga/socket.c */

#ifdef AMIGA_TCP
#include "socket_tcp.c"
#elif defined(AMITCP)
#include "socket_amitcp.c"
#else
#include "socket_sim.c"
#endif
