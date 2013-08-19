/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "std.h"
#include "comm.h"
#include "main.h"
#include "socket_efuns.h"
#include "backend.h"
#include "socket_ctrl.h"
#include "debug.h"
#include "ed.h"
#include "file.h"
#include "master.h"
#include "add_action.h"
#include "eval.h"
#include "console.h"

#include "event.h"
#include "dns.h"

#ifndef ENOSR
#define ENOSR 63
#endif

#ifndef ANSI_SUBSTITUTE
#define ANSI_SUBSTITUTE 0x20
#endif

#ifndef ADDRFAIL_NOTIFY
#define ADDRFAIL_NOTIFY 0
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define TELOPT_MSSP 70
#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
#define TELOPT_MXP  91  // mud extension protocol
#define TELOPT_ZMP  93  // zenith mud protocol
#define TELOPT_GMCP 201 // something mud communication protocol, how many do we need?

#define MSSP_VAR 1
#define MSSP_VAL 2

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef ENV_FILLER
#define ENV_FILLER 0x1e
#endif

#define TELOPT_NEW_ENVIRON 39
#define NEW_ENV_IS               0
#define NEW_ENV_SEND             1
#define NEW_ENV_INFO             2
#define NEW_ENV_VAR              0
#define NEW_ENV_VALUE            1
#define NEW_ENV_ESC              2
#define NEW_ENV_USERVAR          3


static unsigned char telnet_break_response[] = {  28, IAC, WILL, TELOPT_TM };
static unsigned char telnet_ip_response[]    = { 127, IAC, WILL, TELOPT_TM };
static unsigned char telnet_abort_response[] = { IAC, DM };
static unsigned char telnet_do_tm_response[] = { IAC, WILL, TELOPT_TM };
static unsigned char telnet_do_naws[]        = { IAC, DO, TELOPT_NAWS };
static unsigned char telnet_dont_naws[]      = { IAC, DONT, TELOPT_NAWS };
static unsigned char telnet_do_ttype[]       = { IAC, DO, TELOPT_TTYPE };
static unsigned char telnet_term_query[]     = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };
static unsigned char telnet_no_echo[]        = { IAC, WONT, TELOPT_ECHO };
static unsigned char telnet_no_single[]      = { IAC, WONT, TELOPT_SGA };
static unsigned char telnet_yes_echo[]       = { IAC, WILL, TELOPT_ECHO };
static unsigned char telnet_yes_single[]     = { IAC, WILL, TELOPT_SGA };
static unsigned char telnet_ga[]             = { IAC, GA };
static unsigned char telnet_ayt_response[]   = { '\n', '[', '-', 'Y', 'e', 's', '-', ']', ' ', '\n' };
static unsigned char telnet_line_mode[]      = { IAC, DO, TELOPT_LINEMODE };
static unsigned char telnet_lm_mode[]        = { IAC, SB, TELOPT_LINEMODE, LM_MODE, MODE_EDIT | MODE_TRAPSIG, IAC, SE };
static unsigned char telnet_char_mode[]      = { IAC, DONT, TELOPT_LINEMODE };

static unsigned char slc_default_flags[] = { SLC_NOSUPPORT, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_NOSUPPORT,
    SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_NOSUPPORT, SLC_NOSUPPORT,
    SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT
                                           };
static unsigned char slc_default_chars[] = { 0x00, BREAK, IP, AO, AYT, 0x00, 0x00, 0x00,
    SUSP, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00
                                           };
#ifdef HAVE_ZLIB
static unsigned char telnet_compress_send_request_v2[] = { IAC, WILL,
    TELOPT_COMPRESS2
                                                         };

static unsigned char telnet_compress_send_request_v1[] = { IAC, WILL,
    TELOPT_COMPRESS
                                                         };

static unsigned char telnet_compress_v1_response[] = { IAC, SB,
    TELOPT_COMPRESS, WILL,
    SE
                                                     };
static unsigned char telnet_compress_v2_response[] = { IAC, SB,
    TELOPT_COMPRESS2, IAC,
    SE
                                                     };

#endif
static unsigned char telnet_do_mxp[]     = { IAC, DO, TELOPT_MXP };
static unsigned char telnet_will_mxp[]     = { IAC, SB, TELOPT_MXP, IAC, SE };
static unsigned char telnet_will_mssp[] = { IAC, WILL, TELOPT_MSSP };
static unsigned char telnet_start_mssp[] = { IAC, SB, TELOPT_MSSP };
static unsigned char telnet_mssp_value[] = {MSSP_VAR, '%', 's', MSSP_VAL, '%', 's', 0};
static unsigned char telnet_end_sub[] = {IAC, SE};
static unsigned char telnet_will_zmp[] = { IAC, WILL, TELOPT_ZMP};
static unsigned char telnet_start_zmp[] = { IAC, SB, TELOPT_ZMP};
static unsigned char telnet_do_newenv[]     = { IAC, DO, TELOPT_NEW_ENVIRON };
static unsigned char telnet_send_uservar[]     = { IAC, SB, TELOPT_NEW_ENVIRON, NEW_ENV_SEND, IAC, SE };
static unsigned char telnet_will_gmcp[] = {IAC, WILL, TELOPT_GMCP};
static unsigned char telnet_start_gmcp[] = {IAC, SB, TELOPT_GMCP};
/*
 * local function prototypes.
 */

static char *get_user_command(void);
static char *first_cmd_in_buf(interactive_t *);
static int cmd_in_buf(interactive_t *);
static int call_function_interactive(interactive_t *, char *);
static void print_prompt(interactive_t *);

void new_user_handler(port_def_t *);

static void end_compression(interactive_t *);
static void start_compression(interactive_t *);
static int send_compressed(interactive_t *ip, unsigned char *data, int length);
static int flush_compressed_output(interactive_t *ip);

#ifdef NO_SNOOP
#  define handle_snoop(str, len, who)
#else
#  define handle_snoop(str, len, who) if ((who)->snooped_by) receive_snoop(str, len, who->snooped_by)

static void receive_snoop(const char *, int, object_t *ob);

#endif

/*
 * public local variables.
 */
fd_set readmask, writemask;
int num_user;
#ifdef F_SET_HIDE
int num_hidden_users = 0;       /* for the O_HIDDEN flag.  This counter must
 * be kept up to date at all times!  If you
 * modify the O_HIDDEN flag in an object,
 * make sure that you update this counter if
 * the object is interactive. */
#endif
int add_message_calls = 0;
#ifdef F_NETWORK_STATS
int inet_out_packets = 0;
int inet_out_volume = 0;
int inet_in_packets = 0;
int inet_in_volume = 0;
#ifdef PACKAGE_SOCKETS
int inet_socket_in_packets = 0;
int inet_socket_in_volume = 0;
int inet_socket_out_packets = 0;
int inet_socket_out_volume = 0;
#endif
#endif
int inet_packets = 0;
int inet_volume = 0;
interactive_t **all_users = 0;
int max_users = 0;
#ifdef HAS_CONSOLE
int has_console = -1;
#endif

static
void set_linemode(interactive_t *ip)
{
  if (ip->iflags & USING_LINEMODE) {
    add_binary_message_noflush(ip->ob, telnet_line_mode, sizeof(telnet_line_mode));
    add_binary_message_noflush(ip->ob, telnet_lm_mode, sizeof(telnet_lm_mode));
    flush_message(ip);
  } else {
    add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
  }
}

static
void set_charmode(interactive_t *ip)
{
  if (ip->iflags & USING_LINEMODE) {
    add_binary_message(ip->ob, telnet_char_mode, sizeof(telnet_char_mode));
  } else {
    add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
  }
}

#ifndef NO_SNOOP
static void
receive_snoop(const char *buf, int len, object_t *snooper)
{
  /* command giver no longer set to snooper */
#ifdef RECEIVE_SNOOP
  char *str;

  str = new_string(len, "receive_snoop");
  memcpy(str, buf, len);
  str[len] = 0;
  push_malloced_string(str);
  apply(APPLY_RECEIVE_SNOOP, snooper, 1, ORIGIN_DRIVER);
#else
  /* snoop output is now % in all cases */
  add_message(snooper, "%", 1);
  add_message(snooper, buf, len);
#endif
}
#endif

/*
 * Initialize new user connection socket.
 */
void init_user_conn()
{
  int optval;
  int i;
  int have_fd6;
  int fd6_which = -1;

  /* Check for fd #6 open as a valid socket */
  optval = 1;
  have_fd6 = (setsockopt(6, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
                         sizeof(optval)) == 0);

  for (i = 0; i < 5; i++) {
#ifdef F_NETWORK_STATS
    external_port[i].in_packets = 0;
    external_port[i].in_volume = 0;
    external_port[i].out_packets = 0;
    external_port[i].out_volume = 0;
#endif
    if (!external_port[i].port) {
#if defined(FD6_KIND) && defined(FD6_PORT)
      if (!have_fd6) {
        continue;
      }
      fd6_which = i;
      have_fd6 = 0;
      if (FD6_KIND == PORT_UNDEFINED || FD6_PORT < 1) {
        debug_message("Socket passed to fd 6 ignored (support is disabled).\n");
        continue;
      }

      debug_message("Accepting connections on fd 6 (port %d).\n", FD6_PORT);
      external_port[i].kind = FD6_KIND;
      external_port[i].port = FD6_PORT;
      external_port[i].fd = 6;
#else
      continue;
#endif
    } else {
      /*
       * fill in socket address information.
       */
      struct addrinfo *res;

      char service[NI_MAXSERV];
      snprintf(service, sizeof(service), "%u", external_port[i].port);

      struct addrinfo hints;
      memset(&hints, 0, sizeof(struct addrinfo));
#ifdef IPV6
      hints.ai_family = AF_INET6;
#else
      hints.ai_family = AF_INET;
#endif
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_V4MAPPED;

      int ret;
      if (MUD_IP[0]) {
        ret = getaddrinfo(MUD_IP, service, &hints, &res);
      } else {
        ret = getaddrinfo(NULL, service, &hints, &res);
      }

      if (ret) {
        debug_message("init_user_conn: getaddrinfo error: %s \n", gai_strerror(ret));
        exit(3);
      }

      /*
       * create socket of proper type.
       */
      if ((external_port[i].fd = socket(res->ai_family, res->ai_socktype,
                                        res->ai_protocol)) == -1) {
        socket_perror("init_user_conn: socket", 0);
        exit(1);
      }

      /*
       * enable local address reuse.
       */
      optval = 1;
      if (setsockopt(external_port[i].fd, SOL_SOCKET, SO_REUSEADDR,
                     (char *) &optval, sizeof(optval)) == -1) {
        socket_perror("init_user_conn: setsockopt", 0);
        exit(2);
      }

#ifdef FD_CLOEXEC
      fcntl(external_port[i].fd, F_SETFD, FD_CLOEXEC);
#endif

      /*
       * bind name to socket.
       */
      if (bind(external_port[i].fd, res->ai_addr, res->ai_addrlen) == -1) {
        socket_perror("init_user_conn: bind", 0);
        exit(3);
      }

      // cleanup
      freeaddrinfo(res);
    }

    /*
     * set socket non-blocking,
     */
    if (set_socket_nonblocking(external_port[i].fd, 1) == -1) {
      socket_perror("init_user_conn: set_socket_nonblocking 1", 0);
      if (i != fd6_which) {
        exit(8);
      }
    }
    /*
     * listen on socket for connections.
     */
    sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    getsockname(external_port[i].fd, (sockaddr *)&addr, &len);
    debug_message("Accepting connections on %s.\n", sockaddr_to_string((sockaddr *)&addr, len));

    if (listen(external_port[i].fd, 128) == -1) {
      socket_perror("init_user_conn: listen", 0);
      if (i != fd6_which) {
        exit(10);
      }
    }
    // Listen on connetion event
    new_external_port_event_listener(&external_port[i]);
  }
  if (have_fd6) {
    debug_message("No more ports available; fd #6 ignored.\n");
  }
}

/*
 * Shut down new user accept file descriptor.
 */
void shutdown_external_ports()
{
  int i;

  for (i = 0; i < 5; i++) {
    if (!external_port[i].port) { continue; }
    if (external_port[i].ev_read) event_free(external_port[i].ev_read);
    if (OS_socket_close(external_port[i].fd) == -1) {
      socket_perror("ipc_remove: close", 0);
    }
  }

  debug_message("closed external ports\n");
}

/*
 * If there is a shadow for this object, then the message should be
 * sent to it. But only if catch_tell() is defined. Beware that one of the
 * shadows may be the originator of the message, which means that we must
 * not send the message to that shadow, or any shadows in the linked list
 * before that shadow.
 *
 * Also note that we don't need to do this in the case of
 * INTERACTIVE_CATCH_TELL, since catch_tell() was already called
 * _instead of_ add_message(), and shadows got their chance then.
 */
#if !defined(INTERACTIVE_CATCH_TELL) && !defined(NO_SHADOWS)
#define SHADOW_CATCH_MESSAGE
#endif

#ifdef SHADOW_CATCH_MESSAGE
static int shadow_catch_message(object_t *ob, const char *str)
{
  if (!ob->shadowed) {
    return 0;
  }
  while (ob->shadowed != 0 && ob->shadowed != current_object) {
    ob = ob->shadowed;
  }
  while (ob->shadowing) {
    copy_and_push_string(str);
    if (apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER))
      /* this will work, since we know the */
      /* function is defined */
    {
      return 1;
    }
    ob = ob->shadowing;
  }
  return 0;
}
#endif

/*
 * Send a message to an interactive object. If that object is shadowed,
 * special handling is done.
 */
void add_message(object_t *who, const char *data, int len)
{
  interactive_t *ip;
  const char *cp;
  const char *end;
  char *trans;
  int translen;
  /*
   * if who->interactive is not valid, write message on stderr.
   * (maybe)
   */
  if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
      (who->interactive->iflags & (NET_DEAD | CLOSING))) {
#ifdef NONINTERACTIVE_STDERR_WRITE
    putc(']', stderr);
    fwrite(data, len, 1, stderr);
#endif
    return;
  }
  ip = who->interactive;
  trans = translate(ip->trans->outgoing, data, len, &translen);
#ifdef SHADOW_CATCH_MESSAGE
  /*
   * shadow handling.
   */
  if (shadow_catch_message(who, data)) {
#ifdef SNOOP_SHADOWED
    handle_snoop(data, len, ip);
#endif
    return;
  }
#endif                          /* NO_SHADOWS */

  /*
   * write message into ip->message_buf.
   */
  end = trans + translen;
  for (cp = trans; cp < end; cp++) {
    if (ip->message_length == MESSAGE_BUF_SIZE) {
      if (!flush_message(ip)) {
        debug(connections, ("Broken connection during add_message."));
        return;
      }
      if (ip->message_length == MESSAGE_BUF_SIZE) {
        break;
      }
    }
    if ((*cp == '\n' || *cp == -1)
#ifndef NO_BUFFER_TYPE
        && ip->connection_type != PORT_BINARY
#endif
       ) {
      if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
        if (!flush_message(ip)) {
          debug(connections, ("Broken connection during add_message."));
          return;
        }
        if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
          break;
        }
      }
      ip->message_buf[ip->message_producer] = (*cp == '\n') ? '\r' : -1;
      ip->message_producer = (ip->message_producer + 1)
                             % MESSAGE_BUF_SIZE;
      ip->message_length++;
    }
    ip->message_buf[ip->message_producer] = *cp;
    ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
    ip->message_length++;
  }

  handle_snoop(data, len, ip);

  event_add(ip->ev_write, NULL);
#ifdef FLUSH_OUTPUT_IMMEDIATELY
  flush_message(ip);
#endif

  add_message_calls++;
}                               /* add_message() */

/* WARNING: this can only handle results < LARGEST_PRINTABLE_STRING in size */
void add_vmessage(object_t *who, const char *format, ...)
{
  char new_string_data[LARGEST_PRINTABLE_STRING + 1];

  va_list args;
  V_START(args, format);
  V_VAR(object_t *, who, args);
  V_VAR(char *, format, args);

  new_string_data[0] = '\0';
  // FIXME: vsnprintf returns length of string, so we could
  // handle arbitrary length, should fix this.
  vsnprintf(new_string_data, sizeof(new_string_data), format, args);
  va_end(args);

  add_message(who, new_string_data, strlen(new_string_data));
}

void add_binary_message_noflush(object_t *who, const unsigned char *data, int len)
{
  interactive_t *ip;
  const unsigned char *cp, *end;

  /*
   * if who->interactive is not valid, bail
   */
  if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
      (who->interactive->iflags & (NET_DEAD | CLOSING))) {
    return;
  }
  ip = who->interactive;

  /*
   * write message into ip->message_buf.
   */
  end = data + len;
  for (cp = data; cp < end; cp++) {
    if (ip->message_length == MESSAGE_BUF_SIZE) {
      if (!flush_message(ip)) {
        debug(connections, ("Broken connection during add_message."));
        return;
      }
      if (ip->message_length == MESSAGE_BUF_SIZE) {
        break;
      }
    }
    ip->message_buf[ip->message_producer] = *cp;
    ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
    ip->message_length++;
  }
  add_message_calls++;
}

void add_binary_message(object_t *who, const unsigned char *data, int len)
{
  add_binary_message_noflush(who, data, len);
  if (who && who->interactive) {
    flush_message(who->interactive);
  }
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message(interactive_t *ip)
{
  int length, num_bytes;

  /*
   * if ip is not valid, do nothing.
   */
  if (!ip || (ip->iflags & (NET_DEAD | CLOSING))) {
    debug(connections, ("flush_message: invalid target!\n"));
    return 0;
  }
  /*
   * write ip->message_buf[] to socket.
   */
  while (ip->message_length != 0) {
    if (ip->message_consumer < ip->message_producer) {
      length = ip->message_producer - ip->message_consumer;
    } else {
      length = MESSAGE_BUF_SIZE - ip->message_consumer;
    }
#ifdef HAVE_ZLIB
    if (ip->compressed_stream) {
      num_bytes = send_compressed(ip, (unsigned char *)ip->message_buf +
                                  ip->message_consumer,  length);
    } else {
#endif
      if (ip->connection_type == PORT_WEBSOCKET && (ip->iflags & HANDSHAKE_COMPLETE)) {
        ip->out_of_band = 0;
        //ok we're in trouble, we have to send the whole thing at once, otherwise we don't know the size!
        //try to get away with only sending small chunks
        int sendsize = length;
        if (length > 125) {
          sendsize = 125;
        }
        unsigned short flags = htons(sendsize | 0x8200); //82 is final packet (of this message) type binary
        int sendres = send(ip->fd, &flags, 2, 0);
        if (sendres <= 0) {
          return 1;    //wait
        }
        if (sendres == 1) {
          ip->iflags |= NET_DEAD;
          return 0;
        }
        num_bytes = send(ip->fd, ip->message_buf + ip->message_consumer, sendsize, 0);
        if (num_bytes != sendsize) {
          ip->iflags |= NET_DEAD;
          return 0;
        }
      } else {
        num_bytes = send(ip->fd, ip->message_buf + ip->message_consumer,
                         length, ip->out_of_band | MSG_NOSIGNAL);
      }
#ifdef HAVE_ZLIB
    }
#endif
    if (!num_bytes) {
      ip->iflags |= NET_DEAD;
      return 0;
    }
    if (num_bytes == -1) {
      if (socket_errno == EWOULDBLOCK || socket_errno == EAGAIN) {
        debug(connections, ("flush_message: write: Operation would block.\n"));
        event_add(ip->ev_write, NULL);
        return 1;
      } else if (socket_errno == EINTR) {
        debug(connections, ("flush_message: write: Interrupted system call.\n"));
        event_add(ip->ev_write, NULL);
        return 1;
      } else {
        debug(connections, "flush_message: write failed: %s, user connection dead.\n",
              evutil_socket_error_to_string(evutil_socket_geterror(ip->fd)));
        ip->iflags |= NET_DEAD;
        return 0;
      }
    }
    ip->message_consumer = (ip->message_consumer + num_bytes) %
                           MESSAGE_BUF_SIZE;
    ip->message_length -= num_bytes;
    ip->out_of_band = 0;
    inet_packets++;
    inet_volume += num_bytes;
#ifdef F_NETWORK_STATS
    inet_out_packets++;
    inet_out_volume += num_bytes;
    external_port[ip->external_port].out_packets++;
    external_port[ip->external_port].out_volume += num_bytes;
#endif
  }
  return 1;
}                               /* flush_message() */

static int send_mssp_val(mapping_t *map, mapping_node_t *el, void *obp)
{
  object_t *ob = (object_t *)obp;
  if (el->values[0].type == T_STRING && el->values[1].type == T_STRING) {
    char buf[1024];
    int len = sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string, el->values[1].u.string);
    add_binary_message(ob, (unsigned char *)buf, len);
  } else if (el->values[0].type == T_STRING && el->values[1].type == T_ARRAY && el->values[1].u.arr->size > 0 && el->values[1].u.arr->item[0].type == T_STRING) {
    char buf[10240];
    int len = sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string, el->values[1].u.arr->item[0].u.string);
    add_binary_message_noflush(ob, (unsigned char *)buf, len);
    array_t *ar = el->values[1].u.arr;
    int i;
    unsigned char val = MSSP_VAL;
    for (i = 1; i < ar->size; i++) {
      if (ar->item[i].type == T_STRING) {
        add_binary_message_noflush(ob, &val, 1);
        add_binary_message_noflush(ob, (const unsigned char *)ar->item[i].u.string, strlen(ar->item[i].u.string));
      }
    }
    flush_message(ob->interactive);
  }
  return 0;
}

static void copy_chars(interactive_t *ip, unsigned const char *from, int num_bytes)
{
  int i, start, x;
  unsigned char dont_response[3] = { IAC, DONT, 0 };
  unsigned char wont_response[3] = { IAC, WONT, 0 };

  start = ip->text_end;
  for (i = 0;  i < num_bytes;  i++) {
    switch (ip->state) {
      case TS_DATA:
        switch ((unsigned char)from[i]) {
          case IAC:
            ip->state = TS_IAC;
            break;

#if defined(NO_ANSI) && defined(STRIP_BEFORE_PROCESS_INPUT)
          case 0x1b:
            ip->text[ip->text_end++] = ANSI_SUBSTITUTE;
            break;
#endif

          case 0x08:
          case 0x7f:
            if (ip->iflags & SINGLE_CHAR) {
              ip->text[ip->text_end++] = from[i];
            } else {
              if (ip->text_end > 0) {
                ip->text_end--;
              }
            }
            break;

          default:
            ip->text[ip->text_end++] = from[i];
            break;
        }
        break;

      case TS_IAC:
        switch ((unsigned char)from[i]) {
          case IAC:
            ip->state = TS_DATA;
            ip->text[ip->text_end++] = from[i];
            break;

          case WILL:
            ip->state = TS_WILL;
            break;

          case WONT:
            ip->state = TS_WONT;
            break;

          case DO:
            ip->state = TS_DO;
            break;

          case DONT:
            ip->state = TS_DONT;
            break;

          case SB:
            ip->state = TS_SB;
            ip->sb_pos = 0;
            break;

          case BREAK:
            add_binary_message(ip->ob, telnet_break_response, sizeof(telnet_break_response));
            break;

          case IP:    /* interrupt process */
            add_binary_message(ip->ob, telnet_ip_response, sizeof(telnet_ip_response));
            break;

          case AYT:   /* are you there?  you bet */
            add_binary_message(ip->ob, telnet_ayt_response, sizeof(telnet_ayt_response));
            break;

          case AO:    /* abort output */
            flush_message(ip);
            ip->out_of_band = MSG_OOB;
            add_binary_message(ip->ob, telnet_abort_response, sizeof(telnet_abort_response));
            break;

          default:
            ip->state = TS_DATA;
            break;
        }
        break;

      case TS_WILL:
        ip->iflags |= USING_TELNET;
        switch ((unsigned char)from[i]) {
          case TELOPT_TTYPE:
            add_binary_message(ip->ob, telnet_term_query, sizeof(telnet_term_query));
            break;

          case TELOPT_LINEMODE:
            /* Do linemode and set the mode: EDIT + TRAPSIG */
            ip->iflags |= USING_LINEMODE;
            set_linemode(ip);
            break;

          case TELOPT_ECHO:
          case TELOPT_NAWS:
            /* do nothing, but don't send a dont response */
            break;

          case TELOPT_MXP :
            /* Mxp is enabled, tell the mudlib about it. */
            apply(APPLY_MXP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
            ip->iflags |= USING_MXP;
            break;

          case TELOPT_NEW_ENVIRON :
            add_binary_message(ip->ob, telnet_send_uservar, sizeof(telnet_send_uservar));
            break;

          default:
            dont_response[2] = from[i];
            add_binary_message(ip->ob, dont_response, sizeof(dont_response));
            break;
        }
        ip->state = TS_DATA;
        break;

      case TS_WONT:
        ip->iflags |= USING_TELNET;
        switch ((unsigned char)from[i]) {
          case TELOPT_LINEMODE:
            /* If we're in single char mode, we just requested for
             * linemode to be disabled, so don't remove our flag.
             */
            if (!(ip->iflags & SINGLE_CHAR)) {
              ip->iflags &= ~USING_LINEMODE;
            }
            break;
        }
        ip->state = TS_DATA;
        break;

      case TS_DO:
        switch ((unsigned char)from[i]) {
          case TELOPT_TM:
            add_binary_message(ip->ob, telnet_do_tm_response, sizeof(telnet_do_tm_response));
            break;

          case TELOPT_SGA:
            if (ip->iflags & USING_LINEMODE) {
              ip->iflags |= SUPPRESS_GA;
              add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
            } else {
              if (ip->iflags & SINGLE_CHAR) {
                add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
              } else {
                add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
              }
            }
            break;
          case TELOPT_GMCP:
            apply(APPLY_GMCP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
            ip->iflags |= USING_GMCP;
            break;
          case TELOPT_ECHO:
            /* do nothing, but don't send a wont response */
            break;
          case TELOPT_MSSP: {
            add_binary_message(ip->ob, telnet_start_mssp, sizeof(telnet_start_mssp));
            svalue_t *res = apply_master_ob(APPLY_GET_MUD_STATS, 0);
            mapping_t *map;
            if (res <= (svalue_t *)0 || res->type != T_MAPPING) {
              map = allocate_mapping(0);
              free_svalue(&apply_ret_value, "telnet neg");
              apply_ret_value.type = T_MAPPING;
              apply_ret_value.u.map = map;
            } else {
              map = res->u.map;
            }
            //ok, so we have a mapping, first make sure we send the required values
            char *tmp = findstring("NAME");
            if (tmp) {
              svalue_t *name = find_string_in_mapping(map, tmp);
              if (!name || name->type != T_STRING) {
                tmp = 0;
              }
            }
            if (!tmp) {
              char buf[1024];
              int len = sprintf(buf, (char *)telnet_mssp_value, "NAME", MUD_NAME);
              add_binary_message(ip->ob, (unsigned char *)buf, len);
            }
            tmp = findstring("PLAYERS");
            if (tmp) {
              svalue_t *players = find_string_in_mapping(map, tmp);
              if (!players || players->type != T_STRING) {
                tmp = 0;
              }
            }
            if (!tmp) {
              char buf[1024];
              char num[5];
              sprintf(num, "%d", num_user);
              int len = sprintf(buf, (char *)telnet_mssp_value, "PLAYERS", num);
              add_binary_message(ip->ob, (unsigned char *)buf, len);
            }
            tmp = findstring("UPTIME");
            if (tmp) {
              svalue_t *upt = find_string_in_mapping(map, tmp);
              if (!upt || upt->type != T_STRING) {
                tmp = 0;
              }
            }
            if (!tmp) {
              char buf[1024];
              char num[20];

              sprintf(num, "%ld", boot_time);
              int len = sprintf(buf, (char *)telnet_mssp_value, "UPTIME", num);
              add_binary_message(ip->ob, (unsigned char *)buf, len);
            }
            //now send the rest
            mapTraverse(map, send_mssp_val, ip->ob);
            add_binary_message(ip->ob, telnet_end_sub, sizeof(telnet_end_sub));
          }
          break;
#ifdef HAVE_ZLIB
          case TELOPT_COMPRESS :
            if (!ip->compressed_stream) {
              add_binary_message(ip->ob, telnet_compress_v1_response,
                                 sizeof(telnet_compress_v1_response));
              start_compression(ip);
            }
            break;
          case TELOPT_COMPRESS2 :
            if (!ip->compressed_stream) {
              add_binary_message(ip->ob, telnet_compress_v2_response,
                                 sizeof(telnet_compress_v2_response));
              start_compression(ip);
            }
            break;
#endif
          case TELOPT_ZMP :
            ip->iflags |= USING_ZMP;
            break;
          default:
            wont_response[2] = from[i];
            add_binary_message(ip->ob, wont_response, sizeof(wont_response));
            break;
        }
        ip->state = TS_DATA;
        break;

      case TS_DONT:
        switch ((unsigned char)from[i]) {
          case TELOPT_SGA:
            if (ip->iflags & USING_LINEMODE) {
              ip->iflags &= ~SUPPRESS_GA;
              add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
            }
            break;
#ifdef HAVE_ZLIB
          case TELOPT_COMPRESS2:
            // If we are told not to use v2, then try v1.
            add_binary_message(ip->ob, telnet_compress_send_request_v1,
                               sizeof(telnet_compress_send_request_v1));
            break;
#endif
        }
        ip->state = TS_DATA;
        break;

      case TS_SB:
        if ((unsigned char)from[i] == IAC) {
          ip->state = TS_SB_IAC;
          break;
        }
        if (ip->sb_pos < ip->sb_size - 1) {
          ip->sb_buf[ip->sb_pos++] = from[i];
        } else if (ip->sb_size < MAX_STRING_LENGTH) {
          ip->sb_size *= 2;
          if (ip->sb_size > MAX_STRING_LENGTH) {
            ip->sb_size = MAX_STRING_LENGTH;
          }
          ip->sb_buf = (unsigned char *)DREALLOC(ip->sb_buf, ip->sb_size,
                                                 TAG_TEMPORARY, "comm: TS_SB");
          if (ip->sb_pos < ip->sb_size - 1) {
            ip->sb_buf[ip->sb_pos++] = from[i];
          }
        }
        break;

      case TS_SB_IAC:
        switch ((unsigned char)from[i]) {
          case IAC:
            if (ip->sb_pos < SB_SIZE - 1) {
              ip->sb_buf[ip->sb_pos++] = from[i];
              ip->state = TS_SB;
            }
            break;

          case SE:
            ip->state = TS_DATA;
            ip->sb_buf[ip->sb_pos] = 0;
            switch ((unsigned char)ip->sb_buf[0]) {

              case TELOPT_NEW_ENVIRON : {
                int j, k;
                char env_buf[BUF_SIZE];
                j = 0;
                k = 1;
                while (k < (ip->sb_pos - 1)) {
                  k++;
                  // RFC 1572: variable type can be either 0 (VAR) or
                  // 3 (USERVAR)
                  if (ip->sb_buf[k] == 0 || ip->sb_buf[k] == 3) {
                    env_buf[j] = ENV_FILLER;
                  } else if (ip->sb_buf[k] == 1) {
                    // RFC 1572: variable type/value separator: 1
                    env_buf[j] = 1;
                    // These are either variable name or values.
                  } else if (ip->sb_buf[k] > 31) {
                    env_buf[j] = ip->sb_buf[k];
                  } else {
                    // FIXME: we don't handle case of 2 (ESC) either.
                    // In case something is wrong, we simply ignore all
                    // environment variables, to be safe.
                    debug_message("TELNET Environment: client %s sent malformed"
                                  " request, skipped!\n",
                                  sockaddr_to_string((sockaddr *)&ip->addr, ip->addrlen));
                    env_buf[0] = '\0';
                    break;
                  }
                  j++;
                }
                env_buf[j] = 0;
                copy_and_push_string(env_buf);
                apply(APPLY_RECEIVE_ENVIRON, ip->ob, 1, ORIGIN_DRIVER);
                break;

              }
              case TELOPT_LINEMODE:
                switch ((unsigned char)ip->sb_buf[1]) {
                  case LM_MODE:
                    /* Don't do anything with an ACK */
                    if (!(ip->sb_buf[2] & MODE_ACK)) {
                      unsigned char sb_ack[] = { IAC, SB, TELOPT_LINEMODE, LM_MODE, MODE_EDIT | MODE_TRAPSIG | MODE_ACK, IAC, SE };

                      /* Accept only EDIT and TRAPSIG && force them too */
                      add_binary_message(ip->ob, sb_ack, sizeof(sb_ack));
                    }
                    break;

                  case LM_SLC: {
                    int slc_length = 4;
                    unsigned char slc_response[SB_SIZE + 6] = { IAC, SB, TELOPT_LINEMODE, LM_SLC };

                    for (x = 2;  x < ip->sb_pos;  x += 3) {
                      /* no response for an ack */
                      if (ip->sb_buf[x + 1] & SLC_ACK) {
                        continue;
                      }

                      /* If we get { 0, SLC_DEFAULT, 0 } or { 0, SLC_VARIABLE, 0 } return a list of values */
                      /* If it's SLC_DEFAULT, reset to defaults first */
                      if (!ip->sb_buf[x] && !ip->sb_buf[x + 2]) {
                        if (ip->sb_buf[x + 1] == SLC_DEFAULT || ip->sb_buf[x + 1] == SLC_VARIABLE) {
                          int n;

                          for (n = 0;  n < NSLC;  n++) {
                            slc_response[slc_length++] = n + 1;
                            if (ip->sb_buf[x + 1] == SLC_DEFAULT) {
                              ip->slc[n][0] = slc_default_flags[n];
                              ip->slc[n][1] = slc_default_chars[n];
                              slc_response[slc_length++] = SLC_DEFAULT;
                            } else {
                              slc_response[slc_length++] = ip->slc[n][0];
                            }
                            slc_response[slc_length++] = ip->slc[n][1];
                          }
                          break;
                        }
                      }

                      slc_response[slc_length++] = ip->sb_buf[x]--;

                      /* If the first octet is out of range, we don't support it */
                      /* If the default flag is not supported, we don't support it */
                      if (ip->sb_buf[x] >= NSLC || slc_default_flags[ip->sb_buf[x]] == SLC_NOSUPPORT) {
                        slc_response[slc_length++] = SLC_NOSUPPORT;
                        slc_response[slc_length++] = ip->sb_buf[x + 2];
                        if ((unsigned char)ip->sb_buf[x + 2] == IAC) {
                          slc_response[slc_length++] = IAC;
                        }
                        continue;
                      }

                      switch ((ip->sb_buf[x + 1] & SLC_LEVELBITS)) {
                        case SLC_NOSUPPORT:
                          if (slc_default_flags[ip->sb_buf[x]] == SLC_CANTCHANGE) {
                            slc_response[slc_length++] = SLC_CANTCHANGE;
                            slc_response[slc_length++] = ip->slc[ip->sb_buf[x]][1];
                            break;
                          }
                          slc_response[slc_length++] = SLC_ACK | SLC_NOSUPPORT;
                          slc_response[slc_length++] = ip->sb_buf[x + 2];
                          ip->slc[ip->sb_buf[x]][0] = SLC_NOSUPPORT;
                          ip->slc[ip->sb_buf[x]][1] = 0;
                          break;

                        case SLC_VARIABLE:
                          if (slc_default_flags[ip->sb_buf[x]] == SLC_CANTCHANGE) {
                            slc_response[slc_length++] = SLC_CANTCHANGE;
                            slc_response[slc_length++] = ip->slc[ip->sb_buf[x]][1];
                            break;
                          }
                          slc_response[slc_length++] = SLC_ACK | SLC_VARIABLE;
                          slc_response[slc_length++] = ip->sb_buf[x + 2];
                          ip->slc[ip->sb_buf[x]][0] = ip->sb_buf[x + 1];
                          ip->slc[ip->sb_buf[x]][1] = ip->sb_buf[x + 2];
                          break;

                        case SLC_CANTCHANGE:
                          slc_response[slc_length++] = SLC_ACK | SLC_CANTCHANGE;
                          slc_response[slc_length++] = ip->sb_buf[x + 2];
                          ip->slc[ip->sb_buf[x]][0] = ip->sb_buf[x + 1];
                          ip->slc[ip->sb_buf[x]][1] = ip->sb_buf[x + 2];
                          break;

                        case SLC_DEFAULT:
                          slc_response[slc_length++] = slc_default_flags[ip->sb_buf[x]];
                          slc_response[slc_length++] = slc_default_flags[ip->sb_buf[x]];
                          ip->slc[ip->sb_buf[x]][0] = slc_default_flags[ip->sb_buf[x]];
                          ip->slc[ip->sb_buf[x]][1] = slc_default_chars[ip->sb_buf[x]];
                          break;

                        default:
                          slc_response[slc_length++] = SLC_NOSUPPORT;
                          slc_response[slc_length++] = ip->sb_buf[x + 2];
                          if ((unsigned char)slc_response[slc_length - 1] == IAC) {
                            slc_response[slc_length++] = IAC;
                          }
                          break;
                      }
                    }

                    if (slc_length > 4) {
                      /* send our response */
                      slc_response[slc_length++] = IAC;
                      slc_response[slc_length++] = SE;
                      add_binary_message(ip->ob, slc_response, slc_length);
                    }
                  }
                  break;

                  case DO: {
                    unsigned char sb_wont[] = { IAC, SB, TELOPT_LINEMODE, WONT, 0, IAC, SE };

                    /* send back IAC SB TELOPT_LINEMODE WONT x IAC SE */
                    sb_wont[4] = ip->sb_buf[2];
                    add_binary_message(ip->ob, sb_wont, sizeof(sb_wont));
                  }
                  break;

                  case WILL: {
                    unsigned char sb_dont[] = { IAC, SB, TELOPT_LINEMODE, DONT, 0, IAC, SE };

                    /* send back IAC SB TELOPT_LINEMODE DONT x IAC SE */
                    sb_dont[4] = ip->sb_buf[2];
                    add_binary_message(ip->ob, sb_dont, sizeof(sb_dont));
                  }
                  break;
                }
                break;

              case TELOPT_NAWS:
                if (ip->sb_pos >= 5) {
                  push_number(((unsigned char)ip->sb_buf[1] << 8) | (unsigned char)ip->sb_buf[2]);
                  push_number(((unsigned char)ip->sb_buf[3] << 8) | (unsigned char)ip->sb_buf[4]);
                  apply(APPLY_WINDOW_SIZE, ip->ob, 2, ORIGIN_DRIVER);
                }
                break;

              case TELOPT_TTYPE:
                if (!ip->sb_buf[1]) {
                  copy_and_push_string((const char *)(ip->sb_buf + 2));
                  apply(APPLY_TERMINAL_TYPE, ip->ob, 1, ORIGIN_DRIVER);
                }
                break;
              case TELOPT_ZMP: {
                array_t *arr = allocate_array(max_array_size);
                ip->sb_buf = (unsigned char *)REALLOC(ip->sb_buf, MAX(ip->sb_pos + 2, SB_SIZE));
                ip->sb_size = MAX(ip->sb_pos + 2, SB_SIZE);
                ip->sb_buf[ip->sb_pos] = 0;
                copy_and_push_string((char *)ip->sb_buf + 1);
                int off = 0;
                int aro = 0;
                while (1) {
                  off += strlen((char *)ip->sb_buf + 1 + off) + 2;
                  if (off >= ip->sb_pos - 1) {
                    break;
                  }
                  arr->item[aro].u.string = string_copy((char *)&ip->sb_buf[off], "ZMP");
                  arr->item[aro].type = T_STRING;
                  arr->item[aro++].subtype = STRING_MALLOC;
                }
                arr = resize_array(arr, aro);
                push_refed_array(arr);
                apply(APPLY_ZMP, ip->ob, 2, ORIGIN_DRIVER);

              }
              break;
              case TELOPT_GMCP:
                ip->sb_buf[ip->sb_pos] = 0;
                copy_and_push_string((char *)&ip->sb_buf[1]);
                apply(APPLY_GMCP, ip->ob, 1, ORIGIN_DRIVER);
                break;
              default:
                for (x = 0;  x < ip->sb_pos;  x++) {
                  ip->sb_buf[x] = (ip->sb_buf[x] ? ip->sb_buf[x] : 'I');
                }
                copy_and_push_string((char *)ip->sb_buf);
                apply(APPLY_TELNET_SUBOPTION, ip->ob, 1, ORIGIN_DRIVER);
                break;
            }
            break;

          default:
            /*
             * Apparently, old versions of MudOS would revert to TS_DATA here.
             * Later versions handle the IAC, and then go back to TS_DATA mode.
             * I don't think either is proper, but the related RFC documents
             * aren't clear on what to do (854, 855).  It is my feeling, that
             * the safest thing to do here is to ignore the option.
             * -- Marius, 6-Jun-2000
             */
            ip->state = TS_SB;
            break;
        }
        break;
    }
  }

  if (ip->text_end > start) {
    /* handle snooping - snooper does not see type-ahead due to
       telnet being in linemode */
    if (!(ip->iflags & NOECHO)) {
      handle_snoop(ip->text + start, ip->text_end - start, ip);
    }
  }
}

/*
 * Read pending data for a user into user->interactive->text.
 * This also does telnet negotiation.
 */
void get_user_data(interactive_t *ip)
{
  int  num_bytes, text_space;
  unsigned char buf[MAX_TEXT];
  int ws_space;

  text_space = sizeof(buf);

  debug(connections, "get_user_data: USER %d\n", ip->fd);

  /* compute how much data we can read right now */
  switch (ip->connection_type) {
    case PORT_WEBSOCKET:
      ws_space = MAX_TEXT - ip->ws_text_end;
      /* check if we need more space */
      if (ws_space < MAX_TEXT / 16) {
        if (ip->ws_text_start > 0) {
          memmove(ip->ws_text, ip->ws_text + ip->ws_text_start, ip->ws_text_end - ip->ws_text_start);
          ws_space += ip->ws_text_start;
          ip->ws_text_end -= ip->ws_text_start;
          ip->ws_text_start = 0;
        }
      }
      if ((ip->iflags & HANDSHAKE_COMPLETE) && (!ip->ws_size) && ws_space > 8) {
        ws_space = 8;    //only read the header or we'll end up queueing several websocket packets with no triggers to read them
      }
      if (ip->ws_size && ws_space > ip->ws_size) {
        ws_space = ip->ws_size;    //keep the next packet in the socket
      }
      break;
    case PORT_TELNET:
      text_space = MAX_TEXT - ip->text_end;

      /* check if we need more space */
      if (text_space < MAX_TEXT / 16) {
        if (ip->text_start > 0) {
          memmove(ip->text, ip->text + ip->text_start, ip->text_end - ip->text_start);
          text_space += ip->text_start;
          ip->text_end -= ip->text_start;
          ip->text_start = 0;
        }
      }
      break;

    case PORT_MUD:
      if (ip->text_end < 4) {
        text_space = 4 - ip->text_end;
      } else {
        text_space = *(volatile int *)ip->text - ip->text_end + 4;
      }
      break;

    default:
      text_space = sizeof(buf);
      break;
  }

  /* read the data from the socket */
  debug(connections, "get_user_data: read on fd %d\n", ip->fd);
  num_bytes = OS_socket_read(ip->fd, buf, text_space);

  if (!num_bytes) {
    // if (ip->iflags & CLOSING)
    //    debug_message("get_user_data: tried to read from closing fd.\n");
    ip->iflags |= NET_DEAD;
    remove_interactive(ip->ob, 0);
    return;
  }

  if (num_bytes == -1) {
#ifdef EWOULDBLOCK
    if (socket_errno == EWOULDBLOCK) {
      debug(connections, "get_user_data: read on fd %d: Operation would block.\n", ip->fd);
      return;
    }
#endif
    debug(connections, "get_user_data: fd %d, read error: %s.\n", ip->fd,
          evutil_socket_error_to_string(evutil_socket_geterror(ip->fd)));
    ip->iflags |= NET_DEAD;
    remove_interactive(ip->ob, 0);
    return;
  }

#ifdef F_NETWORK_STATS
  inet_in_packets++;
  inet_in_volume += num_bytes;
  external_port[ip->external_port].in_packets++;
  external_port[ip->external_port].in_volume += num_bytes;
#endif

  /* process the data that we've just read */

  switch (ip->connection_type) {
    case PORT_WEBSOCKET:
      if (ip->iflags & HANDSHAKE_COMPLETE) {
        memcpy(ip->ws_text + ip->ws_text_end, buf, num_bytes);
        ip->ws_text_end += num_bytes;
        if (!ip->ws_size) {
          unsigned char *data = (unsigned char *)&ip->ws_text[ip->ws_text_start];
          if (ip->ws_text_end - ip->ws_text_start < 8) {
            break;
          }
          unsigned char msize = data[1];
          int size = msize & 0x7f;
          ip->ws_text_start += 2;
          if (size == 126) {
            size = (data[2] << 8) | data[3];
            ip->ws_text_start += 2;
          } else if (size == 127) { //insane real size
            ip->iflags |= NET_DEAD;
            remove_interactive(ip->ob, 0);
            return;
          }
          ip->ws_size = size;
          if (msize & 0x80) {
            memcpy(&ip->ws_mask, &ip->ws_text[ip->ws_text_start], 4);
            ip->ws_text_start += 4;
          } else { ip->ws_mask = 0; }
          ip->ws_maskoffs = 0;
        }
        int i;
        if (ip->ws_size) {
          int *wdata = (int *)&ip->ws_text[ip->ws_text_start];
          int *dest = (int *)&buf[0];
          if (ip->ws_maskoffs) {
            int newmask;
            for (i = 0; i < 4; i++) {
              ((char *)&newmask)[i] = ((char *)&ip->ws_mask)[(i + ip->ws_maskoffs) % 4];
            }
            ip->ws_mask = newmask;
            ip->ws_maskoffs = 0;
          }
          i = 0;
          while (ip->ws_size > 3 && ip->ws_text_end - ip->ws_text_start > 3) {
            dest[i] = wdata[i] ^ ip->ws_mask;
            i++;
            ip->ws_text_start += 4;
            ip->ws_size -= 4;
          }
          num_bytes = i * 4;
          int left = ip->ws_size;
          if (left > ip->ws_text_end - ip->ws_text_start) {
            left = ip->ws_text_end - ip->ws_text_start;
          }
          if (left) {
            ip->ws_maskoffs = left;
            dest[i] = wdata[i] ^ ip->ws_mask;
            num_bytes += left;
            ip->ws_text_start += left;
            ip->ws_size -= left;
          }
        }
        //          for(i=0;i<num_bytes;i++)
        //              printf("%x ", buf[i]);
        //          puts("");
        //and on with the telnet case
      } else {
        char *str = new_string(num_bytes, "PORT_WEBSOCKET");
        memcpy(str, buf, num_bytes);
        ip->ws_size = 0;
        ip->ws_text_end = 0;
        str[num_bytes] = 0;
        push_malloced_string(str);
        if (current_interactive) {
          fatal("eek! someone already here\n");
          return;
        }
        object_t *ob = ip->ob;
        set_command_giver(ob);
        current_interactive = ob;
        safe_apply(APPLY_PROCESS_INPUT, ob, 1, ORIGIN_DRIVER);
        set_command_giver(0);
        current_interactive = 0;

        break; //they're not allowed to send the other stuff until we replied, so all data should be handshake stuff
      }
      break;
    case PORT_TELNET:
      copy_chars(ip, buf, num_bytes);
      if (cmd_in_buf(ip)) {
        ip->iflags |= CMD_IN_BUF;
        event_active(ip->ev_command, EV_TIMEOUT, 0);
      }
      break;

    case PORT_MUD:
      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      if (num_bytes == text_space) {
        if (ip->text_end == 4) {
          *(volatile int *)ip->text = ntohl(*(int *)ip->text);
          if (*(volatile int *)ip->text > MAX_TEXT - 5) {
            remove_interactive(ip->ob, 0);
          }
        } else {
          svalue_t value;

          ip->text[ip->text_end] = 0;
          if (restore_svalue(ip->text + 4, &value) == 0) {
            STACK_INC;
            *sp = value;
          } else {
            push_undefined();
          }
          ip->text_end = 0;
          safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
        }
      }
      break;

    case PORT_ASCII: {
      char *nl, *p;

      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      p = ip->text + ip->text_start;
      while ((nl = (char *)memchr(p, '\n', ip->text_end - ip->text_start))) {
        ip->text_start = (nl + 1) - ip->text;

        *nl = 0;
        if (*(nl - 1) == '\r') {
          *--nl = 0;
        }

        if (!(ip->ob->flags & O_DESTRUCTED)) {
          char *str;

          str = new_string(nl - p, "PORT_ASCII");
          memcpy(str, p, nl - p + 1);
          push_malloced_string(str);
          safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
        }

        if (ip->text_start == ip->text_end) {
          ip->text_start = ip->text_end = 0;
          break;
        }

        p = nl + 1;
      }
    }
    break;

#ifndef NO_BUFFER_TYPE
    case PORT_BINARY: {
      buffer_t *buffer;

      buffer = allocate_buffer(num_bytes);
      memcpy(buffer->item, buf, num_bytes);

      push_refed_buffer(buffer);
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
    }
    break;
#endif
  }
}

static int clean_buf(interactive_t *ip)
{
  /* skip null input */
  while (ip->text_start < ip->text_end && !*(ip->text + ip->text_start)) {
    ip->text_start++;
  }

  /* if we've advanced beyond the end of the buffer, reset it */
  if (ip->text_start >= ip->text_end) {
    ip->text_start = ip->text_end = 0;
  }

  /* if we're skipping the current command, check to see if it has been
     completed yet.  if it has, flush it and clear the skip bit */
  if (ip->iflags & SKIP_COMMAND) {
    char *p;

    for (p = ip->text + ip->text_start;  p < ip->text + ip->text_end;  p++) {
      if (*p == '\r' || *p == '\n') {
        ip->text_start += p - (ip->text + ip->text_start) + 1;
        ip->iflags &= ~SKIP_COMMAND;
        return clean_buf(ip);
      }
    }
  }

  return (ip->text_end > ip->text_start);
}

static int cmd_in_buf(interactive_t *ip)
{
  char *p;

  /* do standard input buffer cleanup */
  if (!clean_buf(ip)) {
    return 0;
  }

  /* if we're in single character mode, we've got input */
  if (ip->iflags & SINGLE_CHAR) {
    return 1;
  }

  /* search for a newline.  if found, we have a command */
  for (p = ip->text + ip->text_start;  p < ip->text + ip->text_end;  p++) {
    if (*p == '\r' || *p == '\n') {
      return 1;
    }
  }

  /* duh, no command */
  return 0;
}

static char *first_cmd_in_buf(interactive_t *ip)
{
  char *p;
#ifdef GET_CHAR_IS_BUFFERED
  static char tmp[2];
#endif

  /* do standard input buffer cleanup */
  if (!clean_buf(ip)) {
    return 0;
  }

  p = ip->text + ip->text_start;

  /* if we're in single character mode, we've got input */
  if (ip->iflags & SINGLE_CHAR) {
    if (*p == 8 || *p == 127) {
      *p = 0;
    }
#ifndef GET_CHAR_IS_BUFFERED
    ip->text_start++;
    if (!clean_buf(ip)) {
      ip->iflags &= ~CMD_IN_BUF;
    }
    return p;
#else
    tmp[0] = *p;
    ip->text[ip->text_start++] = 0;
    if (!clean_buf(ip)) {
      ip->iflags &= ~CMD_IN_BUF;
    }
    return tmp;
#endif
  }

  /* search for the newline */
  while (ip->text[ip->text_start] != '\n' && ip->text[ip->text_start] != '\r') {
    ip->text_start++;
  }

  /* check for "\r\n" or "\n\r" */
  if (ip->text_start + 1 < ip->text_end &&
      ((ip->text[ip->text_start] == '\r' && ip->text[ip->text_start + 1] == '\n') ||
       (ip->text[ip->text_start] == '\n' && ip->text[ip->text_start + 1] == '\r'))) {
    ip->text[ip->text_start++] = 0;
  }

  ip->text[ip->text_start++] = 0;
  if (!cmd_in_buf(ip)) {
    ip->iflags &= ~CMD_IN_BUF;
  }

  return p;
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
void new_user_handler(port_def_t *port)
{
  int new_socket_fd;
  struct sockaddr_storage addr;
  socklen_t length;
  int i, x;
  object_t *master, *ob;
  svalue_t *ret;

  debug(connections, "new_user_handler: accept on fd %d\n", port->fd);

  length = sizeof(addr);
  new_socket_fd = accept(port->fd, (struct sockaddr *) &addr, &length);
  if (new_socket_fd < 0) {
    if (socket_errno == EWOULDBLOCK) {
      debug(connections, ("new_user_handler: accept: Operation would block\n"));
    } else {
      debug(connections, "new_user_handler: fd %d, accept error: %s.\n", port->fd,
            evutil_socket_error_to_string(evutil_socket_geterror(port->fd)));
    }
    return;
  }

  /*
   * according to Amylaar, 'accepted' sockets in Linux 0.99p6 don't
   * properly inherit the nonblocking property from the listening socket.
   * Marius, 19-Jun-2000: this happens on other platforms as well, so just
   * do it for everyone
   */
  if (set_socket_nonblocking(new_socket_fd, 1) == -1) {
    debug(connections, "new_user_handler: fd %d, set_socket_nonblocking 1 error: %s.\n", new_socket_fd,
          evutil_socket_error_to_string(evutil_socket_geterror(new_socket_fd)));
    OS_socket_close(new_socket_fd);
    return;
  }

  if (set_socket_tcp_nodelay(new_socket_fd, 1) == -1) {
    debug(connections, "new_user_handler: fd %d, set_socket_tcp_nodelay error: %s.\n", new_socket_fd,
          evutil_socket_error_to_string(evutil_socket_geterror(new_socket_fd)));
  }

  /* find the first available slot */
  for (i = 0; i < max_users; i++)
    if (!all_users[i]) { break; }

  if (i == max_users) {
    if (all_users) {
      all_users = RESIZE(all_users, max_users + 10, interactive_t *,
                         TAG_USERS, "new_user_handler");
    } else {
      all_users = CALLOCATE(10, interactive_t *,
                            TAG_USERS, "new_user_handler");
    }
    while (max_users < i + 10) {
      all_users[max_users++] = 0;
    }
  }

  set_command_giver(master_ob);
  master_ob->interactive =
    (interactive_t *)
    DXALLOC(sizeof(interactive_t), TAG_INTERACTIVE,
            "new_user_handler");
#ifndef NO_ADD_ACTION
  master_ob->interactive->default_err_message.s = 0;
#endif
  master_ob->interactive->connection_type = port->kind;
  master_ob->interactive->sb_buf = (unsigned char *)DMALLOC(SB_SIZE,
                                   TAG_PERMANENT, "new_user_handler");
  master_ob->interactive->sb_size = SB_SIZE;
  master_ob->flags |= O_ONCE_INTERACTIVE;
  /*
   * initialize new user interactive data structure.
   */
  master_ob->interactive->ob = master_ob;
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  master_ob->interactive->input_to = 0;
#endif
  master_ob->interactive->iflags = 0;
  master_ob->interactive->text[0] = '\0';
  master_ob->interactive->text_end = 0;
  master_ob->interactive->text_start = 0;
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  master_ob->interactive->carryover = NULL;
  master_ob->interactive->num_carry = 0;
#endif
#ifndef NO_SNOOP
  master_ob->interactive->snooped_by = 0;
#endif
  master_ob->interactive->last_time = current_time;
#ifdef TRACE
  master_ob->interactive->trace_level = 0;
  master_ob->interactive->trace_prefix = 0;
#endif
#ifdef OLD_ED
  master_ob->interactive->ed_buffer = 0;
#endif
#ifdef HAVE_ZLIB
  master_ob->interactive->compressed_stream = NULL;
#endif

  master_ob->interactive->message_producer = 0;
  master_ob->interactive->message_consumer = 0;
  master_ob->interactive->message_length = 0;
  master_ob->interactive->state = TS_DATA;
  master_ob->interactive->out_of_band = 0;
  master_ob->interactive->ws_text_start = 0;
  master_ob->interactive->ws_text_end = 0;
  master_ob->interactive->ws_size = 0;
#ifdef USE_ICONV
  master_ob->interactive->trans = get_translator("UTF-8");
#else
  master_ob->interactive->trans = (struct translation *) master_ob;
  //never actually used, but avoids multiple ifdefs later on!
#endif
  for (x = 0;  x < NSLC;  x++) {
    master_ob->interactive->slc[x][0] = slc_default_flags[x];
    master_ob->interactive->slc[x][1] = slc_default_chars[x];
  }
  all_users[i] = master_ob->interactive;
  all_users[i]->fd = new_socket_fd;
#ifdef F_QUERY_IP_PORT
  all_users[i]->local_port = port->port;
#endif
#ifdef F_NETWORK_STATS
  all_users[i]->external_port = (port - external_port); // FIXME: pointer arith
#endif

  new_user_event_listener(i);

  // all_users[i] setup finishes
  set_prompt("> ");

  memcpy((char *) &all_users[i]->addr, (char *)&addr, length);
  all_users[i]->addrlen = length;

  char host[NI_MAXHOST];
  getnameinfo((sockaddr *)&addr, length, host, sizeof(host), NULL, 0 , NI_NUMERICHOST);
  debug(connections, "New connection from %s.\n", host);
  num_user++;

  /*
   * The user object has one extra reference. It is asserted that the
   * master_ob is loaded.  Save a pointer to the master ob incase it
   * changes during APPLY_CONNECT.  We want to free the reference on
   * the right copy of the object.
   */
  master = master_ob;
  add_ref(master_ob, "new_user");
  push_number(port->port);
  ret = safe_apply_master_ob(APPLY_CONNECT, 1);
  /* master_ob->interactive can be zero if the master object self
     destructed in the above (don't ask) */
  set_command_giver(0);
  if (ret == 0 || ret == (svalue_t *) - 1 || ret->type != T_OBJECT
      || !master_ob->interactive) {
    if (master_ob->interactive) {
      remove_interactive(master_ob, 0);
    }
    debug_message("Can not accept connection from %s due to error in connect().\n", host);
    return;
  }
  /*
   * There was an object returned from connect(). Use this as the user
   * object.
   */
  ob = ret->u.ob;
#ifdef F_SET_HIDE
  if (ob->flags & O_HIDDEN) {
    num_hidden_users++;
  }
#endif
  ob->interactive = master_ob->interactive;
  ob->interactive->ob = ob;
  ob->flags |= O_ONCE_INTERACTIVE;
  /*
   * assume the existance of write_prompt and process_input in user.c
   * until proven wrong (after trying to call them).
   */
  ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);

  free_object(&master, "new_user");

  master_ob->flags &= ~O_ONCE_INTERACTIVE;
  master_ob->interactive = 0;
  add_ref(ob, "new_user");
  set_command_giver(ob);
  query_name_by_addr(ob);

  if (port->kind == PORT_TELNET) {
    /* Ask permission to ask them for their terminal type */
    add_binary_message_noflush(ob, telnet_do_ttype, sizeof(telnet_do_ttype));
    /* Ask them for their window size */
    add_binary_message_noflush(ob, telnet_do_naws, sizeof(telnet_do_naws));
#ifdef HAVE_ZLIB
    add_binary_message_noflush(ob, telnet_compress_send_request_v2,
                               sizeof(telnet_compress_send_request_v2));
#endif
    // Ask them if they support mxp.
    add_binary_message_noflush(ob, telnet_do_mxp, sizeof(telnet_do_mxp));
    // And mssp
    add_binary_message_noflush(ob, telnet_will_mssp, sizeof(telnet_will_mssp));
    // May as well ask for zmp while we're there!
    add_binary_message_noflush(ob, telnet_will_zmp, sizeof(telnet_will_zmp));
    // Also newenv
    add_binary_message_noflush(ob, telnet_do_newenv, sizeof(telnet_do_newenv));
    // gmcp *yawn*
    add_binary_message_noflush(ob, telnet_will_gmcp, sizeof(telnet_will_gmcp));

    // LAST: flush out packet.
    flush_message(ob->interactive);
  }

  // Call logon() on the object.
  if (!(ob->flags & O_DESTRUCTED)) {
    /* current_object no longer set */
    ret = safe_apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
    if (ret == NULL) {
      debug_message("new_user_handler: logon() on object %s has failed, the user is left dangling.\n", ob->obname);
    }
    /* function not existing is no longer fatal */
  } else {
    debug_message("new_user_handler: object is gone before logon(), the user is left dangling. \n");
  }

  debug(connections, ("new_user_handler: end\n"));
  set_command_giver(0);
}                               /* new_user_handler() */

/*
 * Return the first command of the next user in sequence that has a complete
 * command in their buffer.  A command is defined to be a single character
 * when SINGLE_CHAR is set, or a newline terminated string otherwise.
 */
static char *get_user_command(interactive_t *ip)
{
  char *user_command = NULL;

  if (!ip || !ip->ob || (ip->ob->flags & O_DESTRUCTED)) {
    return NULL;
  }

  /* if there's a command in the buffer, pull it out! */
  if (ip->iflags & CMD_IN_BUF) {
    user_command = first_cmd_in_buf(ip);
  }

  /* no command found - return NULL */
  if (!user_command) {
    return NULL;
  }

  /* got a command - return it and set command_giver */
  debug(connections, "get_user_command: user_command = (%s)\n", user_command);
  save_command_giver(ip->ob);

#ifndef GET_CHAR_IS_BUFFERED
  if (ip->iflags & NOECHO) {
#else
  if ((ip->iflags & NOECHO) && !(ip->iflags & SINGLE_CHAR)) {
#endif
    /* must not enable echo before the user input is received */
    add_binary_message(command_giver, telnet_no_echo, sizeof(telnet_no_echo));
    ip->iflags &= ~NOECHO;
  }

  ip->last_time = current_time;
  return user_command;
}                               /* get_user_command() */

static int escape_command(interactive_t *ip, char *user_command)
{
  if (user_command[0] != '!') {
    return 0;
  }
#ifdef OLD_ED
  if (ip->ed_buffer) {
    return 1;
  }
#endif
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to && (!(ip->iflags & NOESC) && !(ip->iflags & I_SINGLE_CHAR))) {
    return 1;
  }
#endif
  return 0;
}

static void process_input(interactive_t *ip, char *user_command)
{
  svalue_t *ret;

  if (!(ip->iflags & HAS_PROCESS_INPUT)) {
    parse_command(user_command, command_giver);
    return;
  }

  /*
   * send a copy of user input back to user object to provide
   * support for things like command history and mud shell
   * programming languages.
   */
  copy_and_push_string(user_command);
  ret = apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
  if (!IP_VALID(ip, command_giver)) {
    return;
  }
  if (!ret) {
    ip->iflags &= ~HAS_PROCESS_INPUT;
    parse_command(user_command, command_giver);
    return;
  }

#ifndef NO_ADD_ACTION
  if (ret->type == T_STRING) {
    static char buf[MAX_TEXT];

    strncpy(buf, ret->u.string, MAX_TEXT - 1);
    parse_command(buf, command_giver);
  } else {
    if (ret->type != T_NUMBER || !ret->u.number) {
      parse_command(user_command, command_giver);
    }
  }
#endif
}

/*
 * This is the user command handler. This function is called when
 * a user command needs to be processed.
 * This function calls get_user_command() to get a user command.
 * One user command is processed per execution of this function.
 */
int process_user_command(interactive_t *ip)
{
  char *user_command;

  /*
   * WARNING: get_user_command() sets command_giver via
   * save_command_giver(), but only when the return is non-zero!
   */
  if (!(user_command = get_user_command(ip))) {
    return 0;
  }

  if (ip != command_giver->interactive) {
    fatal("BUG: process_user_command.");
  }

  current_interactive = command_giver;    /* this is yuck phooey, sigh */
  if (ip) { clear_notify(ip->ob); }

  // FIXME: move this to somewhere else
  update_load_av();

  debug(connections, "process_user_command: command_giver = /%s\n", command_giver->obname);

  if (!ip) {
    goto exit;
  }

  user_command = translate_easy(ip->trans->incoming, user_command);

  if ((ip->iflags & USING_MXP) && user_command[0] == ' ' && user_command[1] == '[' && user_command[3] == 'z') {
    svalue_t *ret;
    copy_and_push_string(user_command);

    ret = safe_apply(APPLY_MXP_TAG, ip->ob, 1, ORIGIN_DRIVER);
    if (ret && ret->type == T_NUMBER && ret->u.number) {
      goto exit;
    }
  }

  if (escape_command(ip, user_command)) {
    if (ip->iflags & SINGLE_CHAR) {
      /* only 1 char ... switch to line buffer mode */
      ip->iflags |= WAS_SINGLE_CHAR;
      ip->iflags &= ~SINGLE_CHAR;
#ifdef GET_CHAR_IS_BUFFERED
      ip->text_start = ip->text_end = *ip->text = 0;
#endif
      set_linemode(ip);
    } else {
      if (ip->iflags & WAS_SINGLE_CHAR) {
        /* we now have a string ... switch back to char mode */
        ip->iflags &= ~WAS_SINGLE_CHAR;
        ip->iflags |= SINGLE_CHAR;
        set_charmode(ip);
        if (!IP_VALID(ip, command_giver)) {
          goto exit;
        }
      }

      process_input(ip, user_command + 1);
    }

    goto exit;
  }

#ifdef OLD_ED
  if (ip->ed_buffer) {
    ed_cmd(user_command);
    goto exit;
  }
#endif

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (call_function_interactive(ip, user_command)) {
    goto exit;
  }
#endif

  process_input(ip, user_command);

exit:
  /*
   * Print a prompt if user is still here.
   */
  if (IP_VALID(ip, command_giver)) {
    print_prompt(ip);
    // FIXME: this doesn't belong here, should be moved to event.cc
    if (ip->iflags & CMD_IN_BUF) {
      event_active(ip->ev_command, EV_TIMEOUT, 0);
    }
  }

  current_interactive = 0;
  restore_command_giver();
  return 1;
}

/*
 * Remove an interactive user immediately.
 */
void remove_interactive(object_t *ob, int dested)
{
  int idx;
  /* don't have to worry about this dangling, since this is the routine
   * that causes this to dangle elsewhere, and we are protected from
   * getting called recursively by CLOSING.  safe_apply() should be
   * used here, since once we start this process we can't back out,
   * so jumping out with an error would be bad.
   */
  interactive_t *ip = ob->interactive;

  if (!ip) { return; }

  if (ip->iflags & CLOSING) {
    if (!dested) {
      debug_message("Double call to remove_interactive()\n");
    }
    return;
  }
  debug(connections, "Closing connection from %s.\n",
        sockaddr_to_string((struct sockaddr *)&ip->addr, ip->addrlen));
  flush_message(ip);
  ip->iflags |= CLOSING;

#ifdef OLD_ED
  if (ip->ed_buffer) {
    save_ed_buffer(ob);
  }
#else
  if (ob->flags & O_IN_EDIT) {
    object_save_ed_buffer(ob);
    ob->flags &= ~O_IN_EDIT;
  }
#endif

  if (!dested) {
    /*
     * auto-notification of net death
     */
    save_command_giver(ob);
    safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);
    restore_command_giver();
  }

#ifndef NO_SNOOP
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
    ip->snooped_by = 0;
  }
#endif

#ifdef HAVE_ZLIB
  if (ip->compressed_stream) {
    end_compression(ip);
  }
#endif

  // Cleanup events
  if (ip->ev_read != NULL) {
    event_free(ip->ev_read);
    ip->ev_read = NULL;
  }
  if (ip->ev_write != NULL) {
    event_free(ip->ev_write);
    ip->ev_write = NULL;
  }
  if (ip->ev_command != NULL) {
    event_free(ip->ev_command);
    ip->ev_command = NULL;
  }
  if (ip->ev_data != NULL) {
    delete ip->ev_data;
    ip->ev_data = NULL;
  }

  debug(connections, "remove_interactive: closing fd %d\n", ip->fd);
  if (OS_socket_close(ip->fd) == -1) {
    debug(connections, "remove_interactive: close error: %s",
          evutil_socket_error_to_string(evutil_socket_geterror(ip->fd)));
  }
#ifdef F_SET_HIDE
  if (ob->flags & O_HIDDEN) {
    num_hidden_users--;
  }
#endif
  num_user--;
  clear_notify(ip->ob);
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to) {
    free_object(&ip->input_to->ob, "remove_interactive");
    free_sentence(ip->input_to);
    if (ip->num_carry > 0) {
      free_some_svalues(ip->carryover, ip->num_carry);
    }
    ip->carryover = NULL;
    ip->num_carry = 0;
    ip->input_to = 0;
  }
#endif
  for (idx = 0; idx < max_users; idx++)
    if (all_users[idx] == ip) { break; }
  DEBUG_CHECK(idx == max_users, "remove_interactive: could not find and remove user!\n");

  FREE(ip->sb_buf);
  FREE(ip);
  ob->interactive = 0;
  all_users[idx] = 0;
  free_object(&ob, "remove_interactive");
  return;
}                               /* remove_interactive() */

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
static int call_function_interactive(interactive_t *i, char *str)
{
  object_t *ob;
  funptr_t *funp;
  char *function;
  svalue_t *args;
  sentence_t *sent;
  int num_arg;
#ifdef GET_CHAR_IS_BUFFERED
  int was_single = 0;
  int was_noecho = 0;
#endif

  i->iflags &= ~NOESC;
  if (!(sent = i->input_to)) {
    return (0);
  }

  /*
   * Special feature: input_to() has been called to setup a call to a
   * function.
   */
  if (sent->ob->flags & O_DESTRUCTED) {
    /* Sorry, the object has selfdestructed ! */
    free_object(&sent->ob, "call_function_interactive");
    free_sentence(sent);
    i->input_to = 0;
    if (i->num_carry) {
      free_some_svalues(i->carryover, i->num_carry);
    }
    i->carryover = NULL;
    i->num_carry = 0;
    i->input_to = 0;
    if (i->iflags & SINGLE_CHAR) {
      /*
       * clear single character mode
       */
      i->iflags &= ~SINGLE_CHAR;
#ifndef GET_CHAR_IS_BUFFERED
      set_linemode(i);
#else
      was_single = 1;
      if (i->iflags & NOECHO) {
        was_noecho = 1;
        i->iflags &= ~NOECHO;
      }
#endif
    }

    return (0);
  }
  /*
   * We must all references to input_to fields before the call to apply(),
   * because someone might want to set up a new input_to().
   */

  /* we put the function on the stack in case of an error */
  STACK_INC;
  if (sent->flags & V_FUNCTION) {
    function = 0;
    sp->type = T_FUNCTION;
    sp->u.fp = funp = sent->function.f;
    funp->hdr.ref++;
  } else {
    sp->type = T_STRING;
    sp->subtype = STRING_SHARED;
    sp->u.string = function = sent->function.s;
    ref_string(function);
  }
  ob = sent->ob;

  free_object(&sent->ob, "call_function_interactive");
  free_sentence(sent);

  /*
   * If we have args, we have to copy them, so the svalues on the
   * interactive struct can be FREEd
   */
  num_arg = i->num_carry;
  if (num_arg) {
    args = i->carryover;
    i->num_carry = 0;
    i->carryover = NULL;
  } else {
    args = NULL;
  }

  i->input_to = 0;
  if (i->iflags & SINGLE_CHAR) {
    /*
     * clear single character mode
     */
    i->iflags &= ~SINGLE_CHAR;
#ifndef GET_CHAR_IS_BUFFERED
    set_linemode(i);
#else
    was_single = 1;
    if (i->iflags & NOECHO) {
      was_noecho = 1;
      i->iflags &= ~NOECHO;
    }
#endif
  }

  copy_and_push_string(str);
  /*
   * If we have args, we have to push them onto the stack in the order they
   * were in when we got them.  They will be popped off by the called
   * function.
   */
  if (args) {
    transfer_push_some_svalues(args, num_arg);
    FREE(args);
  }
  /* current_object no longer set */
  if (function) {
    if (function[0] == APPLY___INIT_SPECIAL_CHAR) {
      error("Illegal function name.\n");
    }
    (void) apply(function, ob, num_arg + 1, ORIGIN_INTERNAL);
  } else {
    call_function_pointer(funp, num_arg + 1);
  }

  pop_stack();                /* remove `function' from stack */

#ifdef GET_CHAR_IS_BUFFERED
  if (IP_VALID(i, ob)) {
    if (was_single && !(i->iflags & SINGLE_CHAR)) {
      i->text_start = i->text_end = 0;
      i->text[0] = '\0';
      i->iflags &= ~CMD_IN_BUF;
      set_linemode(i);
    }
    if (was_noecho && !(i->iflags & NOECHO)) {
      add_binary_message(i->ob, telnet_no_echo, sizeof(telnet_no_echo));
    }
  }
#endif

  return (1);
}                               /* call_function_interactive() */

int set_call(object_t *ob, sentence_t *sent, int flags)
{
  if (ob == 0 || sent == 0) {
    return (0);
  }
  if (ob->interactive == 0 || ob->interactive->input_to) {
    return (0);
  }
  ob->interactive->input_to = sent;
  ob->interactive->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
  if (flags & I_NOECHO) {
    add_binary_message(ob, telnet_yes_echo, sizeof(telnet_yes_echo));
  }
  if (flags & I_SINGLE_CHAR) {
    set_charmode(ob->interactive);
  }
  return (1);
}                               /* set_call() */
#endif

void set_prompt(const char *str)
{
  if (command_giver && command_giver->interactive) {
    command_giver->interactive->prompt = str;
  }
}                               /* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt(interactive_t *ip)
{
  object_t *ob = ip->ob;

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to == 0) {
#endif
    /* give user object a chance to write its own prompt */
    if (!(ip->iflags & HAS_WRITE_PROMPT)) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#ifdef OLD_ED
    else if (ip->ed_buffer) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#endif
    else if (!apply(APPLY_WRITE_PROMPT, ip->ob, 0, ORIGIN_DRIVER)) {
      if (!IP_VALID(ip, ob)) { return; }
      ip->iflags &= ~HAS_WRITE_PROMPT;
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  }
#endif
  if (!IP_VALID(ip, ob)) { return; }
  /*
   * Put the IAC GA thing in here... Moved from before writing the prompt;
   * vt src says it's a terminator. Should it be inside the no-input_to
   * case? We'll see, I guess.
   */
  if ((ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
    add_binary_message(command_giver, telnet_ga, sizeof(telnet_ga));
  }
  if (!IP_VALID(ip, ob)) { return; }
}                               /* print_prompt() */

/*
 * Let object 'me' snoop object 'you'. If 'you' is 0, then turn off
 * snooping.
 *
 * This routine is almost identical to the old set_snoop. The main
 * difference is that the routine writes nothing to user directly,
 * all such communication is taken care of by the mudlib. It communicates
 * with master.c in order to find out if the operation is permissble or
 * not. The old routine let everyone snoop anyone. This routine also returns
 * 0 or 1 depending on success.
 */
#ifndef NO_SNOOP
int new_set_snoop(object_t *by, object_t *victim)
{
  interactive_t *ip;
  object_t *tmp;

  if (by->flags & O_DESTRUCTED) {
    return 0;
  }
  if (victim && (victim->flags & O_DESTRUCTED)) {
    return 0;
  }

  if (victim) {
    if (!victim->interactive) {
      error("Second argument of snoop() is not interactive!\n");
    }
    ip = victim->interactive;
  } else {
    /*
     * Stop snoop.
     */
    if (by->flags & O_SNOOP) {
      int i;

      for (i = 0; i < max_users; i++) {
        if (all_users[i] && all_users[i]->snooped_by == by) {
          all_users[i]->snooped_by = 0;
        }
      }
      by->flags &= ~O_SNOOP;
    }
    return 1;
  }

  /*
   * Protect against snooping loops.
   */
  tmp = by;
  while (tmp) {
    if (tmp == victim) {
      return 0;
    }

    /* the person snooping us, if any */
    tmp = (tmp->interactive ? tmp->interactive->snooped_by : 0);
  }

  /*
   * Terminate previous snoop, if any.
   */
  if (by->flags & O_SNOOP) {
    int i;

    for (i = 0; i < max_users; i++) {
      if (all_users[i] && all_users[i]->snooped_by == by) {
        all_users[i]->snooped_by = 0;
      }
    }
  }
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
  }
  by->flags |= O_SNOOP;
  ip->snooped_by = by;

  return 1;
}                               /* set_new_snoop() */
#endif

char *query_host_name()
{
  static char name[400];

  gethostname(name, sizeof(name));
  name[sizeof(name) - 1] = '\0';      /* Just to make sure */
  return (name);
}                               /* query_host_name() */

#ifndef NO_SNOOP
object_t *query_snoop(object_t *ob)
{
  if (!ob->interactive) {
    return 0;
  }
  return ob->interactive->snooped_by;
}                               /* query_snoop() */

object_t *query_snooping(object_t *ob)
{
  int i;

  if (!(ob->flags & O_SNOOP)) { return 0; }
  for (i = 0; i < max_users; i++) {
    if (all_users[i] && all_users[i]->snooped_by == ob) {
      return all_users[i]->ob;
    }
  }
  fatal("couldn't find snoop target.\n");
  return 0;
}                               /* query_snooping() */
#endif

int query_idle(object_t *ob)
{
  if (!ob->interactive) {
    error("query_idle() of non-interactive object.\n");
  }
  return (current_time - ob->interactive->last_time);
}                               /* query_idle() */

#ifdef F_EXEC
int replace_interactive(object_t *ob, object_t *obfrom)
{
  if (ob->interactive) {
    error("Bad argument 1 to exec()\n");
  }
  if (!obfrom->interactive) {
    error("Bad argument 2 to exec()\n");
  }
#ifdef F_SET_HIDE
  if ((ob->flags & O_HIDDEN) != (obfrom->flags & O_HIDDEN)) {
    if (ob->flags & O_HIDDEN) {
      num_hidden_users++;
    } else {
      num_hidden_users--;
    }
  }
#endif
  ob->interactive = obfrom->interactive;
  /*
   * assume the existance of write_prompt and process_input in user.c until
   * proven wrong (after trying to call them).
   */
  ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);
  obfrom->interactive = 0;
  ob->interactive->ob = ob;
  ob->flags |= O_ONCE_INTERACTIVE;
  obfrom->flags &= ~O_ONCE_INTERACTIVE;
  add_ref(ob, "exec");
  if (obfrom == command_giver) {
    set_command_giver(ob);
  }

  free_object(&obfrom, "exec");
  return (1);
}                               /* replace_interactive() */
#endif

void outbuf_zero(outbuffer_t *outbuf)
{
  outbuf->real_size = 0;
  outbuf->buffer = 0;
}

int outbuf_extend(outbuffer_t *outbuf, int l)
{
  int limit;

  DEBUG_CHECK(l < 0, "Negative length passed to outbuf_extend.\n");

  l = (l > MAX_STRING_LENGTH ? MAX_STRING_LENGTH : l);

  if (outbuf->buffer) {
    limit = MSTR_SIZE(outbuf->buffer);
    if (outbuf->real_size + l > limit) {
      if (outbuf->real_size == MAX_STRING_LENGTH) { return 0; } /* TRUNCATED */

      /* assume it's going to grow some more */
      limit = (outbuf->real_size + l) * 2;
      if (limit > MAX_STRING_LENGTH) {
        limit = MAX_STRING_LENGTH;
        outbuf->buffer = extend_string(outbuf->buffer, limit);
        return limit - outbuf->real_size;
      }
      outbuf->buffer = extend_string(outbuf->buffer, limit);
    }
  } else {
    outbuf->buffer = new_string(l, "outbuf_extend");
    outbuf->real_size = 0;
  }
  return l;
}

void outbuf_add(outbuffer_t *outbuf, const char *str)
{
  int l, limit;

  if (!outbuf) { return; }
  l = strlen(str);
  if ((limit = outbuf_extend(outbuf, l)) > 0) {
    strncpy(outbuf->buffer + outbuf->real_size, str, limit);
    outbuf->real_size += (l > limit ? limit : l);
    *(outbuf->buffer + outbuf->real_size) = 0;
  }
}

void outbuf_addchar(outbuffer_t *outbuf, char c)
{
  if (outbuf && (outbuf_extend(outbuf, 1) > 0)) {
    *(outbuf->buffer + outbuf->real_size++) = c;
    *(outbuf->buffer + outbuf->real_size) = 0;
  }
}

void outbuf_addv(outbuffer_t *outbuf, const char *format, ...)
{
  char buf[LARGEST_PRINTABLE_STRING + 1];
  va_list args;

  V_START(args, format);
  V_VAR(outbuffer_t *, outbuf, args);
  V_VAR(char *, format, args);

  vsnprintf(buf, LARGEST_PRINTABLE_STRING, format, args);
  va_end(args);

  if (!outbuf) { return; }

  outbuf_add(outbuf, buf);
}

void outbuf_fix(outbuffer_t *outbuf)
{
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
  }
}

void outbuf_push(outbuffer_t *outbuf)
{
  STACK_INC;
  sp->type = T_STRING;
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);

    sp->subtype = STRING_MALLOC;
    sp->u.string = outbuf->buffer;
  } else {
    sp->subtype = STRING_CONSTANT;
    sp->u.string = "";
  }
}

#ifdef HAVE_ZLIB
void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
  return DCALLOC(items, size, TAG_TEMPORARY, "zlib_alloc");
}

void zlib_free(void *opaque, void *address)
{
  FREE(address);
}

static void end_compression(interactive_t *ip)
{
  unsigned char dummy[1];

  if (!ip->compressed_stream) {
    return ;
  }

  ip->compressed_stream->avail_in = 0;
  ip->compressed_stream->next_in = dummy;

  if (deflate(ip->compressed_stream, Z_FINISH) != Z_STREAM_END) {
  }
  flush_compressed_output(ip);
  deflateEnd(ip->compressed_stream);
  FREE(ip->compressed_stream);
  ip->compressed_stream = NULL;
}

static void start_compression(interactive_t *ip)
{
  z_stream *zcompress;

  if (ip->compressed_stream) {
    return ;
  }
  zcompress = (z_stream *) DXALLOC(sizeof(z_stream), TAG_INTERACTIVE,
                                   "start_compression");
  zcompress->next_in = NULL;
  zcompress->avail_in = 0;
  zcompress->next_out = ip->compress_buf;
  zcompress->avail_out = COMPRESS_BUF_SIZE;
  zcompress->zalloc = zlib_alloc;
  zcompress->zfree = zlib_free;
  zcompress->opaque = NULL;

  if (deflateInit(zcompress, 9) != Z_OK) {
    FREE(zcompress);
    fprintf(stderr, "Compression failed.\n");
    return ;
  }

  // Ok, compressing.
  ip->compressed_stream = zcompress;
}

static int flush_compressed_output(interactive_t *ip)
{
  int iStart, nBlock, nWrite, len;
  z_stream *zcompress;
  int ret = 1;

  if (!ip->compressed_stream) {
    return ret;
  }

  zcompress = ip->compressed_stream;

  /* Try to write out some data.. */
  len = zcompress->next_out - ip->compress_buf;
  if (len > 0) {
    /* we have some data to write */

    nWrite = 0;
    for (iStart = 0; iStart < len; iStart += nWrite) {
      if (len - iStart < 4096) {
        nBlock = len - iStart;
      } else {
        nBlock =  4096;
      }
      nWrite = send(ip->fd, &ip->compress_buf[iStart], nBlock,
                    ip->out_of_band);
      if (nWrite < 0) {
        if (errno == EAGAIN
#ifndef WIN32
            || errno == ENOSR
#endif
           ) {
          event_add(ip->ev_write, NULL);
          ret = 2;
          break;
        }
        debug(connections, "flush_compressed_output: Error sending compressed data: %s.\n",
              evutil_socket_error_to_string(evutil_socket_geterror(ip->fd)));

        return FALSE; /* write error */
      }

      if (nWrite <= 0) {
        break;
      }
    }

    if (iStart) {
      /* We wrote "iStart" bytes */
      if (iStart < len) {
        memmove(ip->compress_buf, ip->compress_buf + iStart, len -
                iStart);

      }

      zcompress->next_out = ip->compress_buf + len - iStart;
    }
  }

  return ret;
}


static int send_compressed(interactive_t *ip, unsigned char *data, int length)
{
  z_stream *zcompress;
  int wr = 1;
  int first = 1;

  zcompress = ip->compressed_stream;
  zcompress->next_in = data;
  zcompress->avail_in = length;
  while (zcompress->avail_in && (wr == 1 || first)) {
    if (wr == 2) {
      first = 0;
    }
    zcompress->avail_out = COMPRESS_BUF_SIZE - (zcompress->next_out -
                           ip->compress_buf);

    if (zcompress->avail_out) {
      deflate(zcompress, Z_SYNC_FLUSH);
    }

    if (!(wr = flush_compressed_output(ip))) {
      return 0;
    }
  }
  return length;
}
#endif

#ifdef F_ACT_MXP
void f_act_mxp()
{
  add_binary_message(current_object, telnet_will_mxp, sizeof(telnet_will_mxp));
}
#endif

#ifdef F_SEND_ZMP
void f_send_zmp()
{
  add_binary_message_noflush(current_object, telnet_start_zmp, sizeof(telnet_start_zmp));
  add_binary_message_noflush(current_object, (const unsigned char *)(sp - 1)->u.string, strlen((sp - 1)->u.string));
  int i;
  unsigned char zero = 0;
  for (i = 0; i < sp->u.arr->size; i++) {
    if (sp->u.arr->item[i].type == T_STRING) {
      add_binary_message_noflush(current_object, &zero, 1);
      add_binary_message_noflush(current_object, (const unsigned char *)sp->u.arr->item[i].u.string, strlen(sp->u.arr->item[i].u.string));
    }
  }
  add_binary_message_noflush(current_object, &zero, 1);
  add_binary_message_noflush(current_object, telnet_end_sub, sizeof(telnet_end_sub));
  flush_message(current_object->interactive);
  pop_2_elems();
}
#endif

#ifdef F_SEND_GMCP
void f_send_gmcp()
{
  add_binary_message_noflush(current_object, telnet_start_gmcp, sizeof(telnet_start_gmcp));
  add_binary_message_noflush(current_object, (const unsigned char *)(sp->u.string), strlen(sp->u.string));
  add_binary_message_noflush(current_object, telnet_end_sub, sizeof(telnet_end_sub));
  flush_message(current_object->interactive);

  pop_stack();
}
#endif

#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type()
{
  add_binary_message(command_giver, telnet_term_query, sizeof(telnet_term_query));
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type()
{
  add_binary_message(command_giver, telnet_do_ttype, sizeof(telnet_do_ttype));
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size()
{
  if ((st_num_arg == 1) && (sp->u.number == 0))
    add_binary_message(command_giver, telnet_dont_naws,
                       sizeof(telnet_dont_naws));
  else {
    add_binary_message(command_giver, telnet_do_naws, sizeof(telnet_do_naws));
  }

  if (st_num_arg == 1) {
    sp--;
  }
}
#endif

#ifdef F_WEBSOCKET_HANDSHAKE_DONE
void f_websocket_handshake_done()
{
  if (!current_interactive) {
    return;
  }

  flush_message(current_interactive->interactive);
  current_interactive->interactive->iflags |= HANDSHAKE_COMPLETE;
  object_t *ob = current_interactive; //command_giver;
  /* Ask permission to ask them for their terminal type */
  add_binary_message_noflush(ob, telnet_do_ttype, sizeof(telnet_do_ttype));
  /* Ask them for their window size */
  add_binary_message_noflush(ob, telnet_do_naws, sizeof(telnet_do_naws));
  // Ask them if they support mxp.
  add_binary_message_noflush(ob, telnet_do_mxp, sizeof(telnet_do_mxp));
  // And mssp
  add_binary_message_noflush(ob, telnet_will_mssp, sizeof(telnet_will_mssp));
  // May as well ask for zmp while we're there!
  add_binary_message_noflush(ob, telnet_will_zmp, sizeof(telnet_will_zmp));
  // Also newenv
  add_binary_message_noflush(ob, telnet_do_newenv, sizeof(telnet_do_newenv));
  // gmcp *yawn*
  add_binary_message_noflush(ob, telnet_will_gmcp, sizeof(telnet_will_gmcp));
  // Flush message
  flush_message(ob->interactive);
}
#endif
