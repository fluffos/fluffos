/*
 *  addr_server.h -- definitions and prototypes for addr_server.c
 *               8-92 : Dwayne Fontenot : original coding.
 */

#ifndef _ADDR_SERVER_H_
#define _ADDR_SERVER_H_

#define DFAULT_PROTO 0       /* use the appropriate protocol    */
#define MAX_CONNS 4          /* max number of connections       */
#define SNAME_LEN 64         /* length of symbolic name string  */
#define MAX_EVENTS_IN_QUEUE        100
#define MAX_EVENTS_TO_PROCESS      100

enum conn_states {CLOSED,OPEN};

enum ev_type {NEW_CONN, CONN};

enum msgtypes {NAMEBYIP, IPBYNAME};

typedef struct conn *conn_ptr;

typedef struct conn {
  int fd;                    /* file descriptor                 */
  int state;                 /* connection state                */
  struct sockaddr_in addr;   /* address struct for connected    */
  char sname[SNAME_LEN];     /* symbolic name of connected host */
} connection;

typedef struct queue_elem *queue_element_ptr;

typedef struct queue_elem {
  short event_type;          /* event type                                  */
  int fd;                    /* file descriptor on which data is pending    */
  queue_element_ptr next;
} queue_element;

void init_conn_sock();

void sigpipe_handler();

void sigio_handler();

void new_conn_handler();

void conn_data_handler();

void enqueue_datapending();

void dequeue_top_event();

void pop_queue_element();

void push_queue_element();

void handle_top_event();

int index_by_fd();

void terminate();

#endif /* _ADDR_SERVER_H_ */
