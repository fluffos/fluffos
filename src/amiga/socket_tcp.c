/* hosts/amiga/socket_tcp.c
**
** Implement the extra framework to use the Commodore socket.library
** for LPMud.
**
** Written by Martin Brenner.
**
**   13-Jan-93 [lars]  Merged.
*/

/*-----------------------------------------------------------------------*/

#include <clib/exec_protos.h>

#include <sys/types.h>
#include <sys/socket.h>

struct Library *SockBase;

/* int socket_error; */

/* this is the maximum number of sockets that you want */

#define MAXSOCKS 50

/* required for Initialization of socket.library */

amiga_sockinit () {
  if ((SockBase = OpenLibrary ("socket.library", 1L)) == NULL) {
    printf ("Error opening socket.library\n");
    Exit(10);
  }
  setup_sockets (MAXSOCKS, &errno);
}

/* exit in a clean way (close remaining sockets */

amiga_sockexit() {
  cleanup_sockets();
  CloseLibrary (SockBase);
}

/*************************************************************************/

