#ifndef SOCKET_CTRL_H
#define SOCKET_CTRL_H

/*
 * socket_ctrl.c
 */
int set_socket_nonblocking PROT((int, int));
int set_socket_owner PROT((int, int));
int set_socket_async PROT((int, int));

#ifdef WIN32
void SocketPerror PROT((char *, char *));
void CDECL cleanup_sockets PROT((void));
#endif

#endif
