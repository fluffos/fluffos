/* hosts/amiga/socket_tcp.c
**
** Implement the extra framework to use the Ami-TCP bsdsocket.library
** for MudOS.
**
** Written by Martin Brenner for Commodore socket.library
**
**   13-Jan-93 [lars]  Merged.
**   05-Jun-94 [robocoder]  Added this file for Ami-TCP
**                            bsdsocket.library support;
**                          Changed Exit() to exit(), to not bypass
**                            compiler's cleanup/termination code
**                          Note: not required for SAS/C (Lattice)
*/

/*-----------------------------------------------------------------------*/

#include <clib/exec_protos.h>
#include <exec/libraries.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "socket_amitcp.h"

#ifndef LATTICE
struct Library *SocketBase = NULL;

#endif

/* int socket_error; */

/* this is the maximum number of sockets that you want */

#define MAXSOCKS 50

/* required for Initialization of socket.library */

void amiga_sockinit()
{
#ifndef LATTICE
    if ((SocketBase = OpenLibrary("bsdsocket.library", 1L)) == NULL) {
	printf("Error opening bsdsocket.library\n");
	exit(10);
    }
    SetErrnoPtr(&errno, sizeof(errno));
#endif
}

/* exit in a clean way (close remaining sockets */

void amiga_sockexit()
{
#ifndef LATTICE
    CloseLibrary(SocketBase);
    SocketBase = NULL;
#endif
}

/*************************************************************************/
