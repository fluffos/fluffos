/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef COMM_H
#define COMM_H

/* union string_or_func */
#include "simulate.h"

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
#ifndef NO_ADD_ACTION
    /* this or What ? is printed when error    */
    union string_or_func default_err_message;
#endif
#ifdef TRACE
    int trace_level;		/* debug flags -- 0 means no debugging     */
    char *trace_prefix;		/* trace only object which has this as name  */
#endif
#ifdef OLD_ED
    struct ed_buffer *ed_buffer;/* local ed                                */
#endif
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

/*
 * comm.c
 */
extern int total_users;
extern fd_set readmask;
extern fd_set writemask;
extern int inet_packets;
extern int inet_volume;
extern int num_user;
extern int num_hidden;
extern struct interactive *all_users[];
extern int add_message_calls;

#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler PROT((int));
#else
void sigalrm_handler PROT((void));
#endif
void add_vmessage PROTVARGS(());
void add_message PROT((char *));
void update_ref_counts_for_users PROT((void));
INLINE void make_selectmasks PROT((void));
void init_user_conn PROT((void));
void init_addr_server PROT((char *, int));
void ipc_remove PROT((void));
void set_prompt PROT((char *));
void notify_no_command PROT((void));
void set_notify_fail_message PROT((char *));
INLINE void process_io PROT((void));
int process_user_command PROT((void));
int replace_interactive PROT((struct object *, struct object *));
int set_call PROT((struct object *, struct sentence *, int));
void remove_interactive PROT((struct object *));
int query_addr_number PROT((char *, char *));
char *query_ip_name PROT((struct object *));
char *query_ip_number PROT((struct object *));
char *query_host_name PROT((void));
int query_idle PROT((struct object *));
int new_set_snoop PROT((struct object *, struct object *));
struct object *query_snoop PROT((struct object *));
struct object *query_snooping PROT((struct object *));
void set_notify_fail_function PROT((struct funp *));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable PROT((void));
#endif

#endif				/* COMM_H */
