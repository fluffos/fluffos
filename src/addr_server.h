/*
 *  addr_server.h -- definitions and prototypes for addr_server.c
 *               8-92 : Dwayne Fontenot : original coding.
 */

#ifndef ADDR_SERVER_H
#define ADDR_SERVER_H

#include "network_incl.h"

#define DFAULT_PROTO 0		/* use the appropriate protocol    */
#define MAX_CONNS 4		/* max number of connections       */
#define SNAME_LEN 64		/* length of symbolic name string  */
#define MAX_EVENTS_IN_QUEUE        100
#define MAX_EVENTS_TO_PROCESS      100
#define IN_BUF_SIZE                100

enum conn_states {
    CONN_CLOSED, CONN_OPEN
};

enum ev_type {
    NEW_CONN, CONN
};

enum msgtypes {
    NAMEBYIP = 0, IPBYNAME, DATALEN
};

typedef struct conn *conn_ptr;

typedef struct conn {
    int fd;			/* file descriptor                 */
    int state;			/* connection state                */
    struct sockaddr_in addr;	/* address struct for connected    */
    char sname[SNAME_LEN];	/* symbolic name of connected host */
    int leftover;		/* unprocessed bytes in data queue */
    char buf[IN_BUF_SIZE];
}    connection;

typedef struct queue_elem *queue_element_ptr;

typedef struct queue_elem {
    short event_type;		/* event type                                  */
    int fd;			/* file descriptor on which data is pending    */
    queue_element_ptr next;
}          queue_element;

#endif				/* _ADDR_SERVER_H_ */
