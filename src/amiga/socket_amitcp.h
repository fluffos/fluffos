/*
 * amiga/socket_amitcp.h
 *
 * Added by Anthon Pang (aka Robocoder)
 *
 * Note: if a module (.c file) uses these functions on both socket and
 *       file descriptors, function calls on file descriptors must have
 *       the function name parenthesized to keep the preprocessor from
 *       expanding the macro, eg
 *
 *       (close)(file_fd);      ->      close(file_fd);
 *       close(socket_fd);      ->      CloseSocket(socket_fd)
 */
#ifndef SOCKET_AMITCP_H
#define SOCKET_AMITCP_H

#undef close
#undef ioctl
#undef write
#undef read

#define close(s) CloseSocket(s)
#define ioctl(s,t,u) IoctlSocket(s,t,u)
#define write(s,b,l) send(s,b,l,0)
#define read(s,b,l) recv(s,b,l,0)

void amiga_sockinit(void);
void amiga_sockexit(void);

#endif				/* SOCKET_AMITCP_H */
