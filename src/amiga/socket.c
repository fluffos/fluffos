/* hosts/amiga/socket.c */

#ifdef AMIGA_TCP
#  include "socket_tcp.c"
#else
#  include "socket_sim.c"
#endif
