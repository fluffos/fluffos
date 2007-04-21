#ifndef SOCKET_CTRL_H
#define SOCKET_CTRL_H

/*
 * socket_ctrl.c
 */
int set_socket_nonblocking (int, int);
int set_socket_owner (int, int);
int set_socket_async (int, int);

#ifdef WIN32
void SocketPerror (char *, char *);
void CDECL cleanup_sockets (void);
#endif

#endif
