/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef _COMM_H_
#define _COMM_H_

#include <sys/socket.h>
#include <sys/socketvar.h>
#ifdef _AIX
#include <sys/select.h>
#endif /* _AIX */
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_TEXT                   2048
#define MAX_SOCKET_PACKET_SIZE     1024
#define DESIRED_SOCKET_PACKET_SIZE 800
#define MESSAGE_BUF_SIZE           MESSAGE_BUFFER_SIZE /* from options.h */
#define OUT_BUF_SIZE               2048
#define DFAULT_PROTO               0   /* use the appropriate protocol */
#define I_NOECHO                   0x1 /* input_to flag */
#define I_NOESC                    0x2 /* input_to flag */

enum msgtypes {NAMEBYIP, IPBYNAME};

struct interactive {
  int fd;                     /* file descriptor for interactive object      */
  struct object *ob;          /* points to the associated object             */
  struct sentence *input_to;  /* to be called with next input line           */
  struct sockaddr_in addr;    /* socket address of interactive object        */
  char *prompt;               /* prompt string for interactive object        */
  int closing;                /* true when closing this file descriptor      */
  char text[MAX_TEXT];        /* input buffer for interactive object         */
  int text_end;               /* first free char in buffer                   */
  int text_start;             /* where we are up to in user command buffer   */
  int cmd_in_buf;             /* there is a full command in input buffer     */
  struct interactive *snoop_on;
  struct interactive *snoop_by;
  int noecho;                 /* don't echo lines                            */
  int noesc;                  /* don't allow shell out                       */
  int last_time;              /* time of last command executed               */
  char *default_err_message;  /* this or What ? is printed when error        */
#ifdef ACCESS_RESTRICTED
  void *access_class;         /* represents cluster from which user connects */
#endif /* ACCESS_RESTRICTED */
  int trace_level;            /* debug flags -- 0 means no debugging         */
  char *trace_prefix;         /* trace only object which has this as name    */
  struct ed_buffer *ed_buffer;/* local ed                                    */
  int message_producer;	      /* message buffer producer index */
  int message_consumer;	      /* message buffer consumer index */
  int message_length;         /* message buffer length */
  char message_buf[MESSAGE_BUF_SIZE];	/* message buffer */
  int has_process_input;      /* interactive object has process_input()      */
  int has_write_prompt;       /* interactive object has write_prompt()       */
  int single_char;            /* single character mode                       */
  struct svalue *carryover;   /* points to args for input_to                 */
  int num_carry;              /* number of args for input_to                 */
  int net_dead;
};

#endif /* _COMM_H_ */
