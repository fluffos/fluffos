/* hosts/amiga/socket.h */

#ifdef AMIGA_TCP
#  include "socket_tcp.h"
#else
#  include "socket_sim.h"
#endif
