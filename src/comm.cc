/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "base/std.h"

#include "comm.h"

#include <algorithm>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <memory>
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <unistd.h>       // for gethostname

#include "event.h"
#include "fliconv.h"
#include "interactive.h"
#include "thirdparty/libtelnet/libtelnet.h"
#include "net/telnet.h"
#include "user.h"
#include "vm/vm.h"

#include "packages/core/dns.h"         // FIXME?
#include "packages/core/ed.h"          // FIXME?
#include "packages/core/add_action.h"  // FIXME?

// in backend.cc
extern void update_load_av();

/*
 * local function prototypes.
 */
static char *get_user_command(interactive_t *);
static char *first_cmd_in_buf(interactive_t *);
static int cmd_in_buf(interactive_t *);
static int call_function_interactive(interactive_t *, char *);
static void print_prompt(interactive_t *);

#ifdef NO_SNOOP
#define handle_snoop(str, len, who)
#else
#define handle_snoop(str, len, who) \
  if ((who)->snooped_by) receive_snoop(str, len, who->snooped_by)

static void receive_snoop(const char *, int, object_t *ob);

#endif

/*
 * public local variables.
 */
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

/*
 * Initialize new user connection socket.
 */
void init_user_conn() {
  for (int i = 0; i < 5; i++) {
#ifdef F_NETWORK_STATS
    external_port[i].in_packets = 0;
    external_port[i].in_volume = 0;
    external_port[i].out_packets = 0;
    external_port[i].out_volume = 0;
#endif
    if (!external_port[i].port) continue;
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
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
#ifdef IPV6
    hints.ai_flags |= AI_V4MAPPED;
#endif

    int ret;

    auto mudip = CONFIG_STR(__MUD_IP__);
    if (mudip != nullptr && strlen(mudip) > 0) {
      ret = getaddrinfo(mudip, service, &hints, &res);
    } else {
      ret = getaddrinfo(NULL, service, &hints, &res);
    }

    if (ret) {
      debug_message("init_user_conn: getaddrinfo error: %s \n", gai_strerror(ret));
      exit(3);
    }

    // Listen on connection event
    new_external_port_event_listener(&external_port[i], res->ai_addr, res->ai_addrlen);

    debug_message("Accepting connections on %s.\n",
                  sockaddr_to_string((sockaddr *)res->ai_addr, res->ai_addrlen));

    freeaddrinfo(res);
  }
}

/*
 * Shut down new user accept file descriptor.
 */
void shutdown_external_ports() {
  int i;

  for (i = 0; i < 5; i++) {
    if (!external_port[i].port) {
      continue;
    }
    if (external_port[i].ev_conn) evconnlistener_free(external_port[i].ev_conn);
    if (evutil_closesocket(external_port[i].fd) == -1) {
      debug_message("shutdown_external_ports: failed: %s",
                    evutil_socket_error_to_string(evutil_socket_geterror(external_port[i].fd)));
    }
  }

  debug_message("closed external ports\n");
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
void new_user_handler(int fd, struct sockaddr *addr, size_t addrlen, port_def_t *port) {
  debug(connections, "New connection from %s.\n", sockaddr_to_string(addr, addrlen));

  int one = 1;
  if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) == -1) {
    debug(connections, "async_on_accept: fd %d, set_socket_tcp_nodelay error: %s.\n", fd,
          evutil_socket_error_to_string(evutil_socket_geterror(fd)));
  }

  /*
   * initialize new user interactive data structure.
   */
  auto user = user_add();

  user->connection_type = port->kind;
  user->ob = master_ob;
  user->last_time = get_current_time();

#ifdef USE_ICONV
  user->trans = get_translator("UTF-8");
#else
  user->trans = (struct translation *)master_ob;
// never actually used, but avoids multiple ifdefs later on!
#endif

  user->fd = fd;
  user->local_port = port->port;
  user->external_port = (port - external_port);  // FIXME: pointer arith

  memcpy(&user->addr, addr, addrlen);
  user->addrlen = addrlen;

  set_command_giver(master_ob);
  master_ob->flags |= O_ONCE_INTERACTIVE;

  master_ob->interactive = user;

  // FIXME: this belongs in async_on_accept()
  new_user_event_listener(user);

  // Initialize telnet support
  user->telnet = net_telnet_init(user);

  set_prompt("> ");

  /*
   * The user object has one extra reference. It is asserted that the
   * master_ob is loaded.  Save a pointer to the master ob incase it
   * changes during APPLY_CONNECT.  We want to free the reference on
   * the right copy of the object.
   */
  object_t *master, *ob;
  svalue_t *ret;

  master = master_ob;
  add_ref(master_ob, "new_user");
  push_number(user->local_port);
  ret = safe_apply_master_ob(APPLY_CONNECT, 1);
  /* master_ob->interactive can be zero if the master object self
   destructed in the above (don't ask) */
  set_command_giver(0);
  if (ret == 0 || ret == (svalue_t *)-1 || ret->type != T_OBJECT || !master_ob->interactive) {
    debug_message("Can not accept connection from %s due to error in connect().\n",
                  sockaddr_to_string((sockaddr *)&user->addr, user->addrlen));
    if (master_ob->interactive) {
      remove_interactive(master_ob, 0);
    }
    return;
  }
  /*
   * There was an object returned from connect(). Use this as the user
   * object.
   */
  ob = ret->u.ob;
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

  if (user->connection_type == PORT_TELNET) {
    send_initial_telent_negotiantions(user);
  }

  // Call logon() on the object.
  if (!(ob->flags & O_DESTRUCTED)) {
    /* current_object no longer set */
    ret = safe_apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
    if (ret == NULL) {
      debug_message(
          "new_user_handler: logon() on object %s has failed, the user is left "
          "dangling.\n",
          ob->obname);
    }
    /* function not existing is no longer fatal */
  } else {
    debug_message(
        "new_user_handler: object is gone before logon(), the user is left "
        "dangling. \n");
  }

  debug(connections, ("new_user_handler: end\n"));
  set_command_giver(0);
} /* new_user_handler() */

#ifndef NO_SNOOP
static void receive_snoop(const char *buf, int len, object_t *snooper) {
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
static int shadow_catch_message(object_t *ob, const char *str) {
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
void add_message(object_t *who, const char *data, int len) {
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
  auto ip = who->interactive;
  int translen;
  char *trans = translate(ip->trans->outgoing, data, len, &translen);
#ifdef SHADOW_CATCH_MESSAGE
  if (ip->connection_type == PORT_TELNET) {
    telnet_send(ip->telnet, trans, translen);
  } else {
    bufferevent_write(ip->ev_buffer, trans, translen);
  }

  /*
   * shadow handling.
   */
  if (shadow_catch_message(who, data)) {
#ifdef SNOOP_SHADOWED
    handle_snoop(data, len, ip);
#endif
    return;
  }
#endif /* NO_SHADOWS */
  handle_snoop(data, len, ip);

  add_message_calls++;
} /* add_message() */

void add_vmessage(object_t *who, const char *format, ...) {
  va_list args;
  va_start(args, format);
  do {
    int result = vsnprintf(nullptr, 0, format, args);
    if (result < 0) break;
    std::unique_ptr<char> msg(new char[result]);
    result = vsnprintf(msg.get(), result, format, args);
    if (result < 0) break;
    add_message(who, msg.get(), strlen(msg.get()));
    break;
  } while (false);
  va_end(args);
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message(interactive_t *ip) {
  /*
   * if ip is not valid, do nothing.
   */
  if (!ip) {
    debug(connections, ("flush_message: invalid target!\n"));
    return 0;
  }

  // Flush things normally.
  if (bufferevent_flush(ip->ev_buffer, EV_WRITE, BEV_FLUSH) == -1) {
    return 0;
  }

  // For socket bufferevent, bufferevent_flush is actually a no-op, thus we have to
  // implement our own.
  auto fd = bufferevent_getfd(ip->ev_buffer);
  if (fd == -1) {
    return 0;
  }

  auto output = bufferevent_get_output(ip->ev_buffer);
  auto total = evbuffer_get_length(output);
  if (total > 0) {
    evbuffer_unfreeze(output, 1);
    auto wrote = evbuffer_write(output, fd);
    evbuffer_freeze(output, 1);
    return wrote != -1;
  }
  return 0;
}

void flush_message_all() {
  users_foreach([](interactive_t *user) { flush_message(user); });
}

/*
 * Read pending data for a user into user->interactive->text.
 * This also does telnet negotiation.
 */
void get_user_data(interactive_t *ip) {
  int num_bytes, text_space;
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
          memmove(ip->ws_text, ip->ws_text + ip->ws_text_start,
                  ip->ws_text_end - ip->ws_text_start);
          ws_space += ip->ws_text_start;
          ip->ws_text_end -= ip->ws_text_start;
          ip->ws_text_start = 0;
        }
      }
      if ((ip->iflags & HANDSHAKE_COMPLETE) && (!ip->ws_size) && ws_space > 8) {
        ws_space = 8;  // only read the header or we'll end up queueing several
                       // websocket packets with no triggers to read them
      }
      if (ip->ws_size && ws_space > ip->ws_size) {
        ws_space = ip->ws_size;  // keep the next packet in the socket
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

  num_bytes = bufferevent_read(ip->ev_buffer, buf, text_space);

  if (num_bytes == -1) {
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
          } else if (size == 127) {  // insane real size
            ip->iflags |= NET_DEAD;
            remove_interactive(ip->ob, 0);
            return;
          }
          ip->ws_size = size;
          if (msize & 0x80) {
            memcpy(&ip->ws_mask, &ip->ws_text[ip->ws_text_start], 4);
            ip->ws_text_start += 4;
          } else {
            ip->ws_mask = 0;
          }
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
        // and on with the telnet case
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

        break;  // they're not allowed to send the other stuff until we replied,
                // so all data should be handshake stuff
      }
      break;
    case PORT_TELNET: {
      int start = ip->text_end;

      // this will read data into ip->text
      telnet_recv(ip->telnet, (const char *)&buf[0], num_bytes);

      if (ip->text_end > start) {
        /* handle snooping - snooper does not see type-ahead due to
         telnet being in linemode */
        if (!(ip->iflags & NOECHO)) {
          handle_snoop(ip->text + start, ip->text_end - start, ip);
        }

        // If we read something, search for command.
        if (cmd_in_buf(ip)) {
          ip->iflags |= CMD_IN_BUF;
          event_active(ip->ev_command, EV_TIMEOUT, 0);
        }
      }
      break;
    }
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
    } break;

#ifndef NO_BUFFER_TYPE
    case PORT_BINARY: {
      buffer_t *buffer;

      buffer = allocate_buffer(num_bytes);
      memcpy(buffer->item, buf, num_bytes);

      push_refed_buffer(buffer);
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
    } break;
#endif
  }
}

static int clean_buf(interactive_t *ip) {
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

    for (p = ip->text + ip->text_start; p < ip->text + ip->text_end; p++) {
      if (*p == '\r' || *p == '\n') {
        ip->text_start += p - (ip->text + ip->text_start) + 1;
        ip->iflags &= ~SKIP_COMMAND;
        return clean_buf(ip);
      }
    }
  }

  return (ip->text_end > ip->text_start);
}

static int cmd_in_buf(interactive_t *ip) {
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
  for (p = ip->text + ip->text_start; p < ip->text + ip->text_end; p++) {
    if (*p == '\r' || *p == '\n') {
      return 1;
    }
  }

  /* duh, no command */
  return 0;
}

static char *first_cmd_in_buf(interactive_t *ip) {
  char *p;
  static char tmp[2];

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
    tmp[0] = *p;
    ip->text[ip->text_start++] = 0;
    if (!clean_buf(ip)) {
      ip->iflags &= ~CMD_IN_BUF;
    }
    return tmp;
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
 * Return the first command of the next user in sequence that has a complete
 * command in their buffer.  A command is defined to be a single character
 * when SINGLE_CHAR is set, or a newline terminated string otherwise.
 */
static char *get_user_command(interactive_t *ip) {
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

  if ((ip->iflags & NOECHO) && !(ip->iflags & SINGLE_CHAR)) {
    /* must not enable echo before the user input is received */
    set_localecho(command_giver->interactive, true);
    ip->iflags &= ~NOECHO;
  }

  ip->last_time = get_current_time();
  return user_command;
} /* get_user_command() */

static void process_input(interactive_t *ip, char *user_command) {
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
int process_user_command(interactive_t *ip) {
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

  current_interactive = command_giver; /* this is yuck phooey, sigh */
  if (ip) {
    clear_notify(ip->ob);
  }

  // FIXME: move this to somewhere else
  update_load_av();

  debug(connections, "process_user_command: command_giver = /%s\n", command_giver->obname);

  if (!ip) {
    goto exit;
  }

  user_command = translate_easy(ip->trans->incoming, user_command);

  if ((ip->iflags & USING_MXP) && user_command[0] == ' ' && user_command[1] == '[' &&
      user_command[3] == 'z') {
    svalue_t *ret;
    copy_and_push_string(user_command);

    ret = safe_apply(APPLY_MXP_TAG, ip->ob, 1, ORIGIN_DRIVER);
    if (ret && ret->type == T_NUMBER && ret->u.number) {
      goto exit;
    }
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
void remove_interactive(object_t *ob, int dested) {
  /* don't have to worry about this dangling, since this is the routine
   * that causes this to dangle elsewhere, and we are protected from
   * getting called recursively by CLOSING.  safe_apply() should be
   * used here, since once we start this process we can't back out,
   * so jumping out with an error would be bad.
   */
  interactive_t *ip = ob->interactive;

  if (!ip) {
    return;
  }

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

  // Cleanup events
  if (ip->ev_buffer != NULL) {
    bufferevent_free(ip->ev_buffer);
    ip->ev_buffer = NULL;
  }
  if (ip->ev_command != NULL) {
    event_free(ip->ev_command);
    ip->ev_command = NULL;
  }

  // Free telnet handle
  if (ip->telnet != NULL) {
    telnet_free(ip->telnet);
    ip->telnet = NULL;
  }

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
  user_del(ip);
  FREE(ip);
  ob->interactive = 0;
  free_object(&ob, "remove_interactive");
  return;
} /* remove_interactive() */

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
static int call_function_interactive(interactive_t *i, char *str) {
  object_t *ob;
  funptr_t *funp;
  char *function;
  svalue_t *args;
  sentence_t *sent;
  int num_arg;
  int was_single = 0;
  int was_noecho = 0;

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
      set_linemode(i, true);
      if (i->iflags & NOECHO) {
        i->iflags &= ~NOECHO;
        set_localecho(i, true);
      }
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
    was_single = 1;
    if (i->iflags & NOECHO) {
      was_noecho = 1;
      i->iflags &= ~NOECHO;
    }
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
    (void)safe_apply(function, ob, num_arg + 1, ORIGIN_INTERNAL);
  } else {
    safe_call_function_pointer(funp, num_arg + 1);
  }

  pop_stack(); /* remove `function' from stack */

  if (was_single && !(i->iflags & SINGLE_CHAR)) {
    i->text_start = i->text_end = 0;
    i->text[0] = '\0';
    i->iflags &= ~CMD_IN_BUF;
    set_linemode(i, true);
  }
  if (was_noecho && !(i->iflags & NOECHO)) {
    set_localecho(i, true);
  }

  return (1);
} /* call_function_interactive() */

int set_call(object_t *ob, sentence_t *sent, int flags) {
  if (ob == 0 || sent == 0) {
    return (0);
  }
  if (ob->interactive == 0 || ob->interactive->input_to) {
    return (0);
  }
  ob->interactive->input_to = sent;
  ob->interactive->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
  if (flags & I_NOECHO) {
    set_localecho(ob->interactive, false);
  }
  if (flags & I_SINGLE_CHAR) {
    set_charmode(ob->interactive);
  }
  return (1);
} /* set_call() */
#endif

void set_prompt(const char *str) {
  if (command_giver && command_giver->interactive) {
    command_giver->interactive->prompt = str;
  }
} /* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt(interactive_t *ip) {
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
      if (!IP_VALID(ip, ob)) {
        return;
      }
      ip->iflags &= ~HAS_WRITE_PROMPT;
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  }
#endif
  if (!IP_VALID(ip, ob)) {
    return;
  }
  // Stavros: A lot of clients use this TELNET_GA to differentiate
  // prompts from other text
  if ((ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
    telnet_iac(ip->telnet, TELNET_GA);
  }
} /* print_prompt() */

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
int new_set_snoop(object_t *by, object_t *victim) {
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
      users_foreach([by](interactive_t *user) {
        if (user->snooped_by == by) {
          user->snooped_by = 0;
        }
      });
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
  new_set_snoop(by, NULL);

  // setup new snoop
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
  }
  by->flags |= O_SNOOP;
  ip->snooped_by = by;

  return 1;
} /* set_new_snoop() */
#endif

char *query_host_name() {
  static char name[400];

  gethostname(name, sizeof(name));
  name[sizeof(name) - 1] = '\0'; /* Just to make sure */
  return (name);
} /* query_host_name() */

#ifndef NO_SNOOP
object_t *query_snoop(object_t *ob) {
  if (!ob->interactive) {
    return 0;
  }
  return ob->interactive->snooped_by;
} /* query_snoop() */

object_t *query_snooping(object_t *ob) {
  if (!(ob->flags & O_SNOOP)) {
    return 0;
  }
  for (auto &user : users()) {
    if (user->snooped_by == ob) {
      return user->ob;
    }
  }
  // TODO: change this to dfatal
  // fatal("couldn't find snoop target.\n");
  return 0;
} /* query_snooping() */
#endif

int query_idle(object_t *ob) {
  if (!ob->interactive) {
    error("query_idle() of non-interactive object.\n");
  }
  return (get_current_time() - ob->interactive->last_time);
} /* query_idle() */

#ifdef F_EXEC
int replace_interactive(object_t *ob, object_t *obfrom) {
  if (ob->interactive) {
    error("Bad argument 1 to exec()\n");
  }
  if (!obfrom->interactive) {
    error("Bad argument 2 to exec()\n");
  }
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
} /* replace_interactive() */
#endif

#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type() {
  auto ip = command_giver->interactive;
  telnet_request_ttype(ip->telnet);
  flush_message(ip);
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type() {
  auto ip = command_giver->interactive;
  telnet_start_request_ttype(ip->telnet);
  flush_message(ip);
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size() {
  auto ip = command_giver->interactive;

  if ((st_num_arg == 1) && (sp->u.number == 0)) {
    telnet_dont_naws(ip->telnet);
  } else {
    telnet_do_naws(ip->telnet);
  }

  if (st_num_arg == 1) {
    sp--;
  }
  flush_message(ip);
}
#endif

#ifdef F_WEBSOCKET_HANDSHAKE_DONE
void f_websocket_handshake_done() {
  if (!current_interactive) {
    return;
  }
  auto ip = current_interactive->interactive;
  ip->iflags |= HANDSHAKE_COMPLETE;
  send_initial_telent_negotiantions(ip);
}
#endif

const char *sockaddr_to_string(const sockaddr *addr, socklen_t len) {
  static char result[NI_MAXHOST + NI_MAXSERV];

  char host[NI_MAXHOST], service[NI_MAXSERV];
  int ret = getnameinfo(addr, len, host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV);

  if (ret) {
    debug(sockets, "sockaddr_to_string fail: %s.\n", evutil_gai_strerror(ret));
    strcpy(result, "<invalid address>");
    return result;
  }

  snprintf(result, sizeof(result), strchr(host, ':') != NULL ? "[%s]:%s" : "%s:%s", host, service);

  return result;
}
