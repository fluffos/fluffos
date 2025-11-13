/*
 * socket.h -- socket mode and option constants for LPC
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

/* Socket modes */
#define MUD 0
#define STREAM 1
#define DATAGRAM 2
#define STREAM_BINARY 3
#define DATAGRAM_BINARY 4
#define STREAM_TLS 5
#define STREAM_TLS_BINARY 6

/* Socket options */
#define SO_TLS_VERIFY_PEER 1
#define SO_TLS_SNI_HOSTNAME 2
#define SO_TLS_CERT 3
#define SO_TLS_KEY 4

#endif /* _SOCKET_H_ */
