/*
 * amiga/socket.c
 *
 *   - based on code by Martin Brenner & Lars Düning
 *   - for non-SAS/C users (ie without autoinitialization/autotermination libs)
 */

#include <clib/exec_protos.h>
#include <exec/libraries.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "socket.h"

#ifndef __SASC

/* this is the maximum number of sockets that you want */
#define MAXSOCKS 50

#ifdef AS225
/*
 * AS225 socket.library
 */
struct Library *SockBase = NULL;

void amiga_sockinit() {
    if ((SockBase = OpenLibrary("socket.library", 1L)) == NULL) {
	printf("Error opening socket.library\n");
	exit(10);
    }
    setup_sockets(MAXSOCKS, &errno);
}

void amiga_sockexit() {
    cleanup_sockets();
    CloseLibrary(SockBase);
    SockBase = NULL;
}

#endif /* AS225 */

#ifdef AMITCP
/*
 * AmiTCP bsdsocket.library
 */
struct Library *SocketBase = NULL;

void amiga_sockinit() {
    if ((SocketBase = OpenLibrary("bsdsocket.library", 1L)) == NULL) {
	printf("Error opening bsdsocket.library\n");
	exit(10);
    }
    SetErrnoPtr(&errno, sizeof(errno));
}

void amiga_sockexit() {
    CloseLibrary(SocketBase);
    SocketBase = NULL;
}
#endif /* AMITCP */

#endif /* __SASC */
