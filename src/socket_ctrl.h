#ifndef SOCKET_CTRL_H
#define SOCKET_CTRL_H

/*
 * socket_ctrl.c
 */
int set_socket_nonblocking PROT((int, int));
int set_socket_owner PROT((int, int));
int set_socket_async PROT((int, int));

#endif
