/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef COMM_H
#define COMM_H

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "lpc_incl.h"
#include "network_incl.h"

#include "fliconv.h"

#define MAX_TEXT                   2048
#define MAX_SOCKET_PACKET_SIZE     1024
#define DESIRED_SOCKET_PACKET_SIZE 800
#define MESSAGE_BUF_SIZE           MESSAGE_BUFFER_SIZE  /* from options.h */
#define OUT_BUF_SIZE               2048
#define DFAULT_PROTO               0    /* use the appropriate protocol */
#define I_NOECHO                   0x1  /* input_to flag */
#define I_NOESC                    0x2  /* input_to flag */
#define I_SINGLE_CHAR              0x4  /* get_char */
#define I_WAS_SINGLE_CHAR          0x8  /* was get_char */
#define SB_SIZE                    (NSLC * 3 + 3)

#ifdef MINGW
#define SIGPIPE                13
#endif

#ifdef HAVE_ZLIB
#define COMPRESS_BUF_SIZE MESSAGE_BUF_SIZE
#endif

enum msgtypes {
    NAMEBYIP = 0, IPBYNAME, DATALEN
};

#define TS_DATA     0
#define TS_IAC      1
#define TS_WILL     2
#define TS_WONT     3
#define TS_DO       4
#define TS_DONT     5
#define TS_SB       6
#define TS_SB_IAC   7

/* The I_* flags are input_to flags */
#define NOECHO              I_NOECHO            /* don't echo lines */
#define NOESC               I_NOESC             /* don't allow shell out */
#define SINGLE_CHAR         I_SINGLE_CHAR       /* get_char */
#define WAS_SINGLE_CHAR     I_WAS_SINGLE_CHAR
#define HAS_PROCESS_INPUT   0x0010              /* interactive object has process_input()  */
#define HAS_WRITE_PROMPT    0x0020              /* interactive object has write_prompt()   */
#define CLOSING             0x0040              /* true when closing this file descriptor  */
#define CMD_IN_BUF          0x0080              /* there is a full command in input buffer */
#define NET_DEAD            0x0100
#define NOTIFY_FAIL_FUNC    0x0200              /* default_err_mesg is a function pointer  */
#define USING_TELNET        0x0400              /* they're using telnet, or something that */
                                                /* understands telnet codes                */
#define SKIP_COMMAND        0x0800              /* skip current command                    */
#define SUPPRESS_GA         0x1000              /* suppress go ahead                       */
#define USING_LINEMODE      0x2000              /* we've negotiated linemode               */
#define USING_MXP           0x4000              /* we've negotiated mxp */
#define USING_ZMP           0x8000              /* we've negotiated zmp */
#define USING_GMCP			0x10000				/* we've negotiated gmcp */

typedef struct interactive_s {
    object_t *ob;               /* points to the associated object         */
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    sentence_t *input_to;       /* to be called with next input line       */
    svalue_t *carryover;        /* points to args for input_to             */
    int num_carry;              /* number of args for input_to             */
#endif
    int connection_type;        /* the type of connection this is          */
    int fd;                     /* file descriptor for interactive object  */
#ifdef IPV6
    struct sockaddr_in6 addr;    /* socket address of interactive object    */
#else
    struct sockaddr_in addr;    /* socket address of interactive object    */
#endif
#ifdef F_QUERY_IP_PORT
    int local_port;             /* which of our ports they connected to    */
#endif
#ifdef F_NETWORK_STATS
    int external_port;          /* external port index for connection      */
#endif
    const char *prompt;         /* prompt string for interactive object    */
    char text[MAX_TEXT];        /* input buffer for interactive object     */
    int text_end;               /* first free char in buffer               */
    int text_start;             /* where we are up to in user command buffer */
    int last_time;              /* time of last command executed           */
#ifndef NO_SNOOP
    object_t *snooped_by;
#endif
#ifndef NO_ADD_ACTION
    /* this or What ? is printed when error    */
    union string_or_func default_err_message;
#endif
#ifdef TRACE
    int trace_level;            /* debug flags -- 0 means no debugging     */
    char *trace_prefix;         /* trace only object which has this as name  */
#endif
#ifdef OLD_ED
    struct ed_buffer_s *ed_buffer;  /* local ed                        */
#endif
#ifdef HAVE_ZLIB
    struct z_stream_s* compressed_stream; /* Is the data stream
                                             compressed or not */
    unsigned char compress_buf[COMPRESS_BUF_SIZE]; /* compress message buffer*/
#endif

    int message_producer;       /* message buffer producer index */
    int message_consumer;       /* message buffer consumer index */
    int message_length;         /* message buffer length */
    char message_buf[MESSAGE_BUF_SIZE]; /* message buffer */
    int iflags;                 /* interactive flags */
    char out_of_band;           /* Send a telnet sync operation            */
    int state;                  /* Current telnet state.  Bingly wop       */
    int sb_pos;                 /* Telnet suboption negotiation stuff      */
    struct translation *trans;
    char *sb_buf;
    int sb_size;
    char slc[NSLC][2];
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
#define IP_VALID(ip, ob) (ob && ip && ob->interactive == ip)
#define VALIDATE_IP(ip, ob) if (!IP_VALID(ip, ob)) goto failure

/*
 * comm.c
 */
extern fd_set readmask;
extern fd_set writemask;
extern int inet_packets;
extern int inet_volume;
#ifdef F_NETWORK_STATS
extern int inet_out_packets;
extern int inet_out_volume;
extern int inet_in_packets;
extern int inet_in_volume;
#ifdef PACKAGE_SOCKETS
extern int inet_socket_in_packets;
extern int inet_socket_in_volume;
extern int inet_socket_out_packets;
extern int inet_socket_out_volume;
#endif
#endif
extern int num_user;
#ifdef F_SET_HIDE
extern int num_hidden_users;
#endif
extern int add_message_calls;

extern interactive_t **all_users;
extern int max_users;
#ifdef HAS_CONSOLE
extern int has_console;
extern void restore_sigttin(void);
#endif

void CDECL add_vmessage (object_t *, const char *, ...);
void add_message (object_t *, const char *, int);
void add_binary_message (object_t *, const unsigned char *, int);

#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler (int);
#else
void sigalrm_handler (void);
#endif
void update_ref_counts_for_users (void);
INLINE void make_selectmasks (void);
void init_user_conn (void);
void init_addr_server (char *, int);
void ipc_remove (void);
void set_prompt (const char *);
INLINE void process_io (void);
int process_user_command (void);
int replace_interactive (object_t *, object_t *);
int set_call (object_t *, sentence_t *, int);
void remove_interactive (object_t *, int);
int flush_message (interactive_t *);
int query_addr_number (const char *, svalue_t *);
char *query_ip_name (object_t *);
const char *query_ip_number (object_t *);
char *query_host_name (void);
int query_idle (object_t *);
#ifndef NO_SNOOP
int new_set_snoop (object_t *, object_t *);
object_t *query_snoop (object_t *);
object_t *query_snooping (object_t *);
#endif

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable (void);
#endif

#endif                          /* COMM_H */
