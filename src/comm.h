/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef _COMM_H_
#define _COMM_H_

#if !defined(OS2)
#include <sys/socket.h>
#if !defined(apollo) && !defined(linux) && !defined(_M_UNIX) && !defined(LATTICE)
#include <sys/socketvar.h>
#endif
#ifndef LATTICE
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#endif

#define MAX_TEXT                   2048
#define MAX_SOCKET_PACKET_SIZE     1024
#define DESIRED_SOCKET_PACKET_SIZE 800
#define MESSAGE_BUF_SIZE           MESSAGE_BUFFER_SIZE	/* from options.h */
#define OUT_BUF_SIZE               2048
#define DFAULT_PROTO               0	/* use the appropriate protocol */
#define I_NOECHO                   0x1	/* input_to flag */
#define I_NOESC                    0x2	/* input_to flag */
#define I_SINGLE_CHAR              0x4  /* get_char */
#define SB_SIZE			   100	/* More than enough */

enum msgtypes {
    NAMEBYIP = 0, IPBYNAME, DATALEN
};

/* The I_* flags are input_to flags */
#define NOECHO              I_NOECHO        /* don't echo lines */
#define NOESC               I_NOESC	    /* don't allow shell out */
#define SINGLE_CHAR         I_SINGLE_CHAR   /* get_char */
#define HAS_PROCESS_INPUT   8	/* interactive object has process_input()  */
#define HAS_WRITE_PROMPT   16	/* interactive object has write_prompt()   */
#define CLOSING            32   /* true when closing this file descriptor  */
#define CMD_IN_BUF         64	/* there is a full command in input buffer */
#define NET_DEAD          128
#define NOTIFY_FAIL_FUNC  256   /* default_err_mesg is a function pointer  */
#define USING_TELNET      512   /* they're using telnet, or something that */
                                /* understands telnet codes                */
struct interactive {
    struct object *ob;		/* points to the associated object         */
    struct sentence *input_to;	/* to be called with next input line       */
#ifdef OS2
    long named_pipe;
#else
    int fd;			/* file descriptor for interactive object  */
    struct sockaddr_in addr;	/* socket address of interactive object    */
#endif
    char *prompt;		/* prompt string for interactive object    */
    char text[MAX_TEXT];	/* input buffer for interactive object     */
    int text_end;		/* first free char in buffer               */
    int text_start;		/* where we are up to in user command buffer */
    struct interactive *snoop_on;
    struct interactive *snoop_by;
    int last_time;		/* time of last command executed           */
    /* this or What ? is printed when error    */
    union string_or_func default_err_message;
#ifdef TRACE
    int trace_level;		/* debug flags -- 0 means no debugging     */
    char *trace_prefix;		/* trace only object which has this as name  */
#endif
    struct ed_buffer *ed_buffer;/* local ed                                */
    int message_producer;	/* message buffer producer index */
    int message_consumer;	/* message buffer consumer index */
    int message_length;		/* message buffer length */
    char message_buf[MESSAGE_BUF_SIZE];	/* message buffer */
    int iflags;                 /* interactive flags */
    struct svalue *carryover;	/* points to args for input_to             */
    int num_carry;		/* number of args for input_to             */
    int out_of_band;		/* Send a telnet sync operation            */
    int state;			/* Current telnet state.  Bingly wop       */
    int sb_pos;			/* Telnet suboption negotiation stuff      */
    char sb_buf[SB_SIZE];
#ifdef DEBUG_COMM_FREEZE
    char debug_block[1024];
    char debug_block_size;
#endif
};

#endif				/* _COMM_H_ */
