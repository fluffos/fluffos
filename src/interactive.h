#ifndef INTERACITVE_H
#define INTERACITVE_H

#include <event2/util.h>

#include "vm/vm.h"  // FIXME: for union string_or_func

#define MAX_TEXT 2048

#define I_NOECHO 0x1          /* input_to flag */
#define I_NOESC 0x2           /* input_to flag */
#define I_SINGLE_CHAR 0x4     /* get_char */
#define I_WAS_SINGLE_CHAR 0x8 /* was get_char */
#define SB_SIZE (NSLC * 3 + 3)

/* The I_* flags are input_to flags */
#define NOECHO I_NOECHO           /* don't echo lines */
#define NOESC I_NOESC             /* don't allow shell out */
#define SINGLE_CHAR I_SINGLE_CHAR /* get_char */
#define WAS_SINGLE_CHAR I_WAS_SINGLE_CHAR
#define HAS_PROCESS_INPUT 0x0010 /* interactive object has process_input()  */
#define HAS_WRITE_PROMPT 0x0020  /* interactive object has write_prompt()   */
#define CLOSING 0x0040           /* true when closing this file descriptor  */
#define CMD_IN_BUF 0x0080        /* there is a full command in input buffer */
#define NET_DEAD 0x0100
#define NOTIFY_FAIL_FUNC 0x0200 /* default_err_mesg is a function pointer  */
#define USING_TELNET 0x0400   /* they're using telnet, or something that understands telnet codes */
#define SKIP_COMMAND 0x0800   /* skip current command                    */
#define SUPPRESS_GA 0x1000    /* suppress go ahead                       */
#define USING_LINEMODE 0x2000 /* we've negotiated linemode               */
#define USING_MXP 0x4000      /* we've negotiated mxp */
#define USING_ZMP 0x8000      /* we've negotiated zmp */
#define USING_GMCP 0x10000    /* we've negotiated gmcp */
#define HANDSHAKE_COMPLETE 0x20000 /* websocket connected */
#define USING_COMPRESS 0x40000     /* we've negotiated compress */

// from ICU
struct UConverter;

struct interactive_t {
  struct object_t *ob; /* points to the associated object         */
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  struct sentence_t *input_to; /* to be called with next input line       */
  struct svalue_t *carryover;  /* points to args for input_to             */
  int num_carry;               /* number of args for input_to             */
#endif
  int connection_type;          /* the type of connection this is          */
  int fd;                       /* file descriptor for interactive object  */
  struct sockaddr_storage addr; /* socket address of interactive object    */
  ev_socklen_t addrlen;
  int local_port;      /* which of our ports they connected to    */
  int external_port;   /* external port index for connection      */
  const char *prompt;  /* prompt string for interactive object    */
  char text[MAX_TEXT]; /* input buffer for interactive object     */
  int text_end;        /* first free char in buffer               */
  int text_start;      /* where we are up to in user command buffer */
  int last_time;       /* time of last command executed           */
#ifndef NO_SNOOP
  struct object_t *snooped_by;
#endif
#ifndef NO_ADD_ACTION
  /* this or What ? is printed when error    */
  union string_or_func default_err_message;
#endif
  int trace_level;    /* debug flags -- 0 means no debugging     */
  char *trace_prefix; /* trace only object which has this as name  */
#ifdef OLD_ED
  struct ed_buffer_s *ed_buffer; /* local ed                        */
#endif
  unsigned int iflags; /* interactive flags */

  // iconv handle
  UConverter *trans;

  // libtelnet handle
  struct telnet_t *telnet;

  // libevent event handle.
  struct bufferevent *ev_buffer;
  struct event *ev_command;

  // libwebsocket handle
  struct lws *lws;
};

#endif /* INTERACTIVE_H */
