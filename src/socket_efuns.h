/*
 *  socket_efuns.h -- definitions and prototypes for socket_efuns.c
 *                    5-92 : Dwayne Fontenot : original coding.
 *                   10-92 : Dave Richards : less original coding.
 */

#ifndef _SOCKET_EFUNS_H_
#define _SOCKET_EFUNS_H_

enum socket_mode { MUD, STREAM, DATAGRAM };
enum socket_state { CLOSED, UNBOUND, BOUND, LISTEN, DATA_XFER };

#define DFAULT_PROTO	0	/* use the appropriate protocol    */
#define	BUF_SIZE	2048	/* max reliable packet size	   */
#define CALLBK_BUF_SIZE	64	/* max length of callback string   */
#define ADDR_BUF_SIZE	64	/* max length of address string    */

struct lpc_socket {
    int			fd;
    short		flags;
    enum socket_mode	mode;
    enum socket_state	state;
    struct sockaddr_in	l_addr;
    struct sockaddr_in	r_addr;
    char		name[ADDR_BUF_SIZE];
    struct object *	owner_ob;
    struct object *	release_ob;
    char		read_callback[CALLBK_BUF_SIZE];
    char		write_callback[CALLBK_BUF_SIZE];
    char		close_callback[CALLBK_BUF_SIZE];
    char *		r_buf;
    int			r_off;
    long		r_len;
    char *		w_buf;
    int			w_off;
    int			w_len;
};

#define	S_RELEASE	0x01
#define	S_BLOCKED	0x02
#define	S_HEADER	0x04
#define	S_WACCEPT	0x08

int socket_create PROT((enum socket_mode, char *, char *));

#endif /* _SOCKET_EFUNS_H_ */
