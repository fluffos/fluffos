#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SOCKET_T int
#define socket_close(s) s_close(s)
#define socket_ioctl(s,t,u) s_ioctl(s,t,u)
#define socket_write(s,b,l) send(s,b,l,0)
#define socket_read(s,b,l) recv(s,b,l,0)
#define socket_select(s,t,u,v,w) select(s,t,u,v,w)
#define socket_number(s) (s)

/* #define perror(str) printf("%s: %d %s\n", str, errno, strerror(errno)) */

#endif /* SOCKET_TCP_H */
