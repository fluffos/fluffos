/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef COMM_H
#define COMM_H

#include "network_incl.h"

#define MAX_TEXT                   2048
#define MAX_SOCKET_PACKET_SIZE     1024
#define DESIRED_SOCKET_PACKET_SIZE 800
#define MESSAGE_BUF_SIZE           MESSAGE_BUFFER_SIZE	/* from options.h */
#define OUT_BUF_SIZE               2048
#define DFAULT_PROTO               0	/* use the appropriate protocol */
#define I_NOECHO                   0x1	/* input_to flag */
#define I_NOESC                    0x2	/* input_to flag */
#define I_SINGLE_CHAR              0x4  /* get_char */
#define I_WAS_SINGLE_CHAR          0x8  /* was get_char */
#define SB_SIZE			   100	/* More than enough */

enum msgtypes {
    NAMEBYIP = 0, IPBYNAME, DATALEN
};

/* The I_* flags are input_to flags */
#define NOECHO              I_NOECHO        /* don't echo lines */
#define NOESC               I_NOESC	    /* don't allow shell out */
#define SINGLE_CHAR         I_SINGLE_CHAR   /* get_char */
#define WAS_SINGLE_CHAR     I_WAS_SINGLE_CHAR
#define HAS_PROCESS_INPUT  16	/* interactive object has process_input()  */
#define HAS_WRITE_PROMPT   32	/* interactive object has write_prompt()   */
#define CLOSING            64   /* true when closing this file descriptor  */
#define CMD_IN_BUF        128	/* there is a full command in input buffer */
#define NET_DEAD          256
#define NOTIFY_FAIL_FUNC  512   /* default_err_mesg is a function pointer  */
#define USING_TELNET     1024   /* they're using telnet, or something that */
                                /* understands telnet codes                */
typedef struct interactive_s {
    object_t *ob;		/* points to the associated object         */
    sentence_t *input_to;	/* to be called with next input line       */
    int connection_type;        /* the type of connection this is          */
    int fd;			/* file descriptor for interactive object  */
    struct sockaddr_in addr;	/* socket address of interactive object    */
#ifdef F_QUERY_IP_PORT
    int local_port;		/* which of our ports they connected to    */
#endif
    char *prompt;		/* prompt string for interactive object    */
    char text[MAX_TEXT];	/* input buffer for interactive object     */
    int text_end;		/* first free char in buffer               */
    int text_start;		/* where we are up to in user command buffer */
    struct interactive_s *snoop_on;
    struct interactive_s *snoop_by;
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
    struct ed_buffer_s *ed_buffer;  /* local ed                        */
#endif
    int message_producer;	/* message buffer producer index */
    int message_consumer;	/* message buffer consumer index */
    int message_length;		/* message buffer length */
    char message_buf[MESSAGE_BUF_SIZE];	/* message buffer */
    int iflags;                 /* interactive flags */
    svalue_t *carryover;	/* points to args for input_to             */
    int num_carry;		/* number of args for input_to             */
    int out_of_band;		/* Send a telnet sync operation            */
    int state;			/* Current telnet state.  Bingly wop       */
    int sb_pos;			/* Telnet suboption negotiation stuff      */
    char sb_buf[SB_SIZE];
} interactive_t;

 /*
  * This macro is for testing whether ip is still valid, since many
  * functions call LPC code, which could otherwise use
  * enable_commands(), set_this_player(), or destruct() to cause
  * all hell to break loose by changing or dangling command_giver
  * or command_giver->interactive.  It also saves us a few dereferences
  * since we know we can trust ip, and also increases code readability.
  *
  * Basically, this should be used as follows:
  *
  * (1) when using command_giver:
  *     set a variable named ip to command_giver->interactive at a point
  *     when you know it is valid.  Then, after a call that might have
  *     called LPC code, check IP_VALID(command_giver), or use
  *     VALIDATE_IP.
  * (2) some other object:
  *     set a variable named ip to ob->interactive, and save ob somewhere;
  *     or if you are just dealing with an ip as input, save ip->ob somewhere.
  *     After calling LPC code, check IP_VALID(ob), or use VALIDATE_IP.
  * 
  * Yes, I know VALIDATE_IP uses a goto.  It's due to C's lack of proper
  * exception handling.  Only use it in subroutines that are set up
  * for it (i.e. define a failure label, and are set up to deal with
  * branching to it from arbitrary points).
  */
#define IP_VALID(ip, ob) (ob && ob->interactive == ip)
#define VALIDATE_IP(ip, ob) if (!IP_VALID(ip, ob)) goto failure

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
extern int add_message_calls;

extern interactive_t **all_users;
extern int max_users;

void add_vmessage PROT2V(object_t *, char *);
void add_message PROT((object_t *, char *));

#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler PROT((int));
#else
void sigalrm_handler PROT((void));
#endif
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
int replace_interactive PROT((object_t *, object_t *));
int set_call PROT((object_t *, sentence_t *, int));
void remove_interactive PROT((object_t *, int));
int flush_message PROT((interactive_t *));
int query_addr_number PROT((char *, char *));
char *query_ip_name PROT((object_t *));
char *query_ip_number PROT((object_t *));
char *query_host_name PROT((void));
int query_idle PROT((object_t *));
int new_set_snoop PROT((object_t *, object_t *));
object_t *query_snoop PROT((object_t *));
object_t *query_snooping PROT((object_t *));
void set_notify_fail_function PROT((funptr_t *));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable PROT((void));
#endif

#endif				/* COMM_H */
