/*
 *  socket_efuns.h -- definitions and prototypes for socket_efuns.c
 *                    5-92 : Dwayne Fontenot : original coding.
 *                   10-92 : Dave Richards : less original coding.
 */

#ifndef _SOCKET_EFUNS_H_
#define _SOCKET_EFUNS_H_

#include "std.h"

#ifndef OS2

enum socket_mode {
    MUD, STREAM, DATAGRAM, STREAM_BINARY, DATAGRAM_BINARY
};
enum socket_state {
    CLOSED, UNBOUND, BOUND, LISTEN, DATA_XFER
};

#define	BUF_SIZE	2048	/* max reliable packet size	   */
#define CALLBK_BUF_SIZE	64	/* max length of callback string   */
#define ADDR_BUF_SIZE	64	/* max length of address string    */

struct lpc_socket {
    int fd;
    short flags;
    enum socket_mode mode;
    enum socket_state state;
    struct sockaddr_in l_addr;
    struct sockaddr_in r_addr;
    char name[ADDR_BUF_SIZE];
    struct object *owner_ob;
    struct object *release_ob;
    char read_callback[CALLBK_BUF_SIZE];
    char write_callback[CALLBK_BUF_SIZE];
    char close_callback[CALLBK_BUF_SIZE];
    char *r_buf;
    int r_off;
    long r_len;
    char *w_buf;
    int w_off;
    int w_len;
};

extern struct lpc_socket lpc_socks[MAX_EFUN_SOCKS];

#define	S_RELEASE	0x01
#define	S_BLOCKED	0x02
#define	S_HEADER	0x04
#define	S_WACCEPT	0x08
#define S_BINARY    0x10

void init_sockets PROT((void));
int check_valid_socket PROT((char *, int, struct object *, char *, int));
void socket_read_select_handler PROT((int));
void socket_write_select_handler PROT((int));
void assign_socket_owner PROT((struct svalue *, struct object *));
struct object *get_socket_owner PROT((int));
void dump_socket_status PROT((void));
void close_referencing_sockets PROT((struct object *));
int get_socket_address PROT((int, char *, int *));
int socket_bind PROT((int, int));
int socket_create PROT((enum socket_mode, char *, char *));
int socket_listen PROT((int, char *));
int socket_accept PROT((int, char *, char *));
int socket_connect PROT((int, char *, char *, char *));
int socket_write PROT((int, struct svalue *, char *));
int socket_close PROT((int));
int socket_release PROT((int, struct object *, char *));
int socket_acquire PROT((int, char *, char *, char *));
char *socket_error PROT((int));

#endif				/* OS2 */

#endif				/* _SOCKET_EFUNS_H_ */
