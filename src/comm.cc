/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "base/std.h"

#include "comm.h"

#include <event2/buffer.h>       // for evbuffer_freeze, etc
#include <event2/bufferevent.h>  // for bufferevent_enable, etc
#include <event2/event.h>        // for EV_TIMEOUT, etc
#include <event2/listener.h>     // for evconnlistener_free, etc
#include <event2/util.h>         // for evutil_closesocket, etc
#include <stdarg.h>              // for va_end, va_list, va_copy, etc
#include <stdio.h>               // for snprintf, vsnprintf, fwrite, etc
#include <string.h>              // for NULL, memcpy, strlen, etc
#include <unistd.h>              // for gethostname
#include <memory>                // for unique_ptr
// Network stuff
#ifndef _WIN32
#include <netdb.h>        // for addrinfo, freeaddrinfo, etc
#include <netinet/in.h>   // for ntohl, IPPROTO_TCP
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <sys/socket.h>   // for SOCK_STREAM
#else
#include <ws2tcpip.h>
#endif
// ICU
#include <unicode/ucnv.h>

#include "backend.h"
#include "interactive.h"
#include "thirdparty/libtelnet/libtelnet.h"
#include "net/telnet.h"
#include "net/websocket.h"
#include "user.h"
#include "vm/vm.h"

#include "packages/core/add_action.h"  // FIXME?
#include "packages/core/dns.h"         // FIXME?
#include "packages/core/ed.h"          // FIXME?

// in backend.cc
extern void update_load_av();
/*
 * local function prototypes.
 */
static char *get_user_command(interactive_t * /*ip*/);
static char *first_cmd_in_buf(interactive_t * /*ip*/);
static int call_function_interactive(interactive_t * /*i*/, char * /*str*/);
static void print_prompt(interactive_t * /*ip*/);

#ifdef NO_SNOOP
#define handle_snoop(str, len, who)
#else
#define handle_snoop(str, len, who) \
  if ((who)->snooped_by) receive_snoop(str, len, who->snooped_by)

static void receive_snoop(const char * /*buf*/, int /*len*/, object_t *ob);

#endif

namespace {
// User socket event
struct user_event_data {
  int idx;
};

void maybe_schedule_user_command(interactive_t *user) {
  // If user has a complete command, schedule a command execution.
  if (user->iflags & CMD_IN_BUF) {
    struct timeval zero_sec = {0, 0};
    evtimer_del(user->ev_command);
    evtimer_add(user->ev_command, &zero_sec);
  }
}

void on_user_command(evutil_socket_t fd, short what, void *arg) {
  debug(event, "User has an full command ready: %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");
  auto user = reinterpret_cast<interactive_t *>(arg);

  if (user == nullptr) {
    DEBUG_FATAL("on_user_command: user == NULL, Driver BUG.");
    return;
  }

  // FIXME: this function currently calls into mudlib and will throw errors
  // This catch block should be moved one level down.
  error_context_t econ;
  save_context(&econ);
  set_eval(max_eval_cost);
  try {
    process_user_command(user);
  } catch (const char *) {
    restore_context(&econ);
  }
  pop_context(&econ);

  /* Has to be cleared if we jumped out of process_user_command() */
  current_interactive = nullptr;

  // if user still have pending command, continue to schedule it.
  //
  // NOTE: It is important to only execute one command here, then schedule next
  // command at the tail, This ensure users have a fair chance that no one can
  // keep running commands.
  //
  // currently command scehduling is done inside process_user_command().
  //
  // maybe_schedule_user_command(user);
}

void on_user_read(bufferevent *bev, void *arg) {
  auto user = reinterpret_cast<interactive_t *>(arg);

  if (user == nullptr) {
    DEBUG_FATAL("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  // Read user input
  get_user_data(user);

  // TODO: currently get_user_data() will schedule command execution.
  // should probably move it here.
}

void on_user_write(bufferevent *bev, void *arg) {
  auto user = reinterpret_cast<interactive_t *>(arg);
  if (user == nullptr) {
    DEBUG_FATAL("on_user_write: user == NULL, Driver BUG.");
    return;
  }
  // nothing to do.
}

void on_user_events(bufferevent *bev, short events, void *arg) {
  auto user = reinterpret_cast<interactive_t *>(arg);

  if (user == nullptr) {
    DEBUG_FATAL("on_user_events: user == NULL, Driver BUG.");
    return;
  }

  if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    user->iflags |= NET_DEAD;
    remove_interactive(user->ob, 0);
  } else {
    debug(event, "on_user_events: ignored unknown events: %d\n", events);
  }
}

void new_user_event_listener(event_base *base, interactive_t *user) {
  auto bev =
      bufferevent_socket_new(base, user->fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
  bufferevent_setcb(bev, on_user_read, on_user_write, on_user_events, user);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  bufferevent_set_timeouts(bev, nullptr, nullptr);

  user->ev_buffer = bev;
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
void new_user_handler(evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr,
                      int addrlen, void *arg) {
  debug(connections, "New connection from %s.\n", sockaddr_to_string(addr, addrlen));

  // TODO: we don't really need to pass in port, we can figure out by
  // evconnlistener_get_fd and compare it
  auto *port = reinterpret_cast<port_def_t *>(arg);

  {
    int one = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
#ifndef _WIN32
                   &one,
#else
                   (const char *)&one,
#endif
                   sizeof(one)) == -1) {
      debug(connections, "new_user_handler: user fd %td, set_socket_tcp_nodelay error: %s.\n", fd,
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
    }
  }

  if (port->kind == PORT_WEBSOCKET) {
    // For websocket connections, wait until they are handshake finished.
    init_user_websocket(port->lws_context, fd);
    return;
  } else {
    // For other connections go straight to no handshake necessary, schedule to logon.
    auto base = evconnlistener_get_base(listener);

    auto user = new_user(port, fd, addr, addrlen);
    new_user_event_listener(base, user);

    if (user->connection_type == PORT_TELNET) {
      user->telnet = net_telnet_init(user);
      send_initial_telnet_negotiations(user);
    }

    event_base_once(
        base, -1, EV_TIMEOUT,
        [](evutil_socket_t fd, short what, void *arg) {
          auto user = reinterpret_cast<interactive_t *>(arg);
          on_user_logon(user);
        },
        (void *)user, nullptr);
  }
  debug(connections, ("new_user_handler: end\n"));
} /* new_user_handler() */

}  // namespace

// Initialize an new user
interactive_t *new_user(port_def_t *port, evutil_socket_t fd, sockaddr *addr,
                        ev_socklen_t addrlen) {
  /*
   * initialize new user interactive data structure.
   */
  auto user = user_add();

  user->connection_type = port->kind;
  user->ob = master_ob;
  user->last_time = get_current_time();
  user->trans = nullptr;
  user->fd = fd;
  user->local_port = port->port;
  user->external_port = (port - external_port);  // FIXME: pointer arith

  memcpy(&user->addr, addr, addrlen);
  user->addrlen = addrlen;

  // Command handler
  auto base = evconnlistener_get_base(port->ev_conn);
  user->ev_command = evtimer_new(base, on_user_command, user);

  return user;
}

// Called upon user, when he's finished negotiations , and ready to logon
void on_user_logon(interactive_t *user) {
  set_command_giver(master_ob);
  master_ob->flags |= O_ONCE_INTERACTIVE;
  master_ob->interactive = user;

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
  set_eval(max_eval_cost);
  ret = safe_apply_master_ob(APPLY_CONNECT, 1);
  /* master_ob->interactive can be zero if the master object self
   destructed in the above (don't ask) */
  set_command_giver(nullptr);
  if (ret == nullptr || ret == (svalue_t *)-1 || ret->type != T_OBJECT || !master_ob->interactive) {
    debug_message("Can not accept connection from %s due to error in connect().\n",
                  sockaddr_to_string(reinterpret_cast<sockaddr *>(&user->addr), user->addrlen));
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
  master_ob->interactive = nullptr;
  add_ref(ob, "new_user");

  // start reverse DNS probing.
  query_name_by_addr(ob);

  set_command_giver(ob);

  set_prompt("> ");

  // Call logon() on the object.
  set_eval(max_eval_cost);
  ret = safe_apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
  if (ret == nullptr) {
    debug_message("new_user_handler: logon() on object %s has failed, the user is disconnected.\n",
                  ob->obname);
    remove_interactive(ob, false);
  } else if (ob->flags & O_DESTRUCTED) {
    // logon() may decide not to allow user connect by destroying objects.
    remove_interactive(ob, true);
  }
  set_command_giver(nullptr);
}

/*
 * Initialize new user connection socket.
 */
bool init_user_conn() {
  for (auto &i : external_port) {
#ifdef F_NETWORK_STATS
    i.in_packets = 0;
    i.in_volume = 0;
    i.out_packets = 0;
    i.out_volume = 0;
#endif
    if (!i.port) continue;
#ifdef IPV6
    auto fd = socket(AF_INET6, SOCK_STREAM, 0);
#else
    auto fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (fd == -1) {
      debug_message("socket_create: socket error: %s.\n",
                    evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      return false;
    }
    if (evutil_make_socket_nonblocking(fd) == -1) {
      debug(sockets, "socket_accept: set_socket_nonblocking error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      evutil_closesocket(fd);
      return false;
    }
    if (evutil_make_socket_closeonexec(fd) == -1) {
      debug(sockets, "socket_accept: make_socket_closeonexec error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      evutil_closesocket(fd);
      return false;
    }
    {
      int one = 1;
      if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
#ifndef _WIN32
                     (void *)&one,
#else
                     (const char *)&one,
#endif
                     sizeof(one)) < 0) {
        evutil_closesocket(fd);
        return false;
      }
    }
    if (evutil_make_listen_socket_reuseable(fd) < 0) {
      evutil_closesocket(fd);
      return false;
    }
#ifdef __CYGWIN__
#ifdef IPV6
    // On windows, IPv6 sockets are IPv6 only by default. We have to change it.
    {
      auto zero = 0;
      if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&zero, sizeof(zero)) == -1) {
        debug_message("socket_create: setsockopt error: %s.\n",
                      evutil_socket_error_to_string(evutil_socket_geterror(fd)));
        evutil_closesocket(fd);
        return false;
      }
    }
#endif
#endif
    {
      /*
       * fill in socket address information.
       */
      struct addrinfo *res;

      char service[NI_MAXSERV];
      snprintf(service, sizeof(service), "%u", i.port);

      // Must be initialized to all zero.
      struct addrinfo hints = {0};
#ifdef IPV6
      hints.ai_family = AF_INET6;
      hints.ai_flags |= AI_V4MAPPED;
#else
      hints.ai_family = AF_INET;
#endif
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags |= AI_PASSIVE | AI_NUMERICSERV;

      int ret;

      auto mudip = CONFIG_STR(__MUD_IP__);
      if (mudip != nullptr && strlen(mudip) > 0) {
        ret = evutil_getaddrinfo(mudip, service, &hints, &res);
      } else {
        ret = evutil_getaddrinfo(nullptr, service, &hints, &res);
      }
      if (ret) {
        debug_message("init_user_conn: getaddrinfo error: %s \n", evutil_gai_strerror(ret));
        return false;
      }

      if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        debug_message("init_user_conn: bind error: %s.\n",
                      evutil_socket_error_to_string(evutil_socket_geterror(fd)));
        evutil_closesocket(fd);
        evutil_freeaddrinfo(res);
        return false;
      }
      debug_message("Accepting %s connections on %s.\n", port_kind_name(i.kind),
                    sockaddr_to_string(res->ai_addr, res->ai_addrlen));
      evutil_freeaddrinfo(res);
    }
    // Listen on connection event
    auto conn = evconnlistener_new(
        g_event_base, new_user_handler, &i,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC, 1024, fd);
    if (conn == nullptr) {
      debug_message("listening failed: %s !", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
      return false;
    }
    i.ev_conn = conn;
    i.fd = fd;
    if (i.kind == PORT_WEBSOCKET) {
      i.lws_context = init_websocket_context(g_event_base, &i);
    }
  }
  return true;
}

/*
 * Shut down new user accept file descriptor.
 */
void shutdown_external_ports() {
  for (auto &port : external_port) {
    if (!port.port) {
      continue;
    }
    // will also close the FD.
    if (port.ev_conn) evconnlistener_free(port.ev_conn);
    if (port.lws_context) close_websocket_context(port.lws_context);
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
#if !defined(NO_SHADOWS)
#define SHADOW_CATCH_MESSAGE
#endif

#ifdef SHADOW_CATCH_MESSAGE
static int shadow_catch_message(object_t *ob, const char *str) {
  if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
    return 0;
  }
  if (!ob->shadowed) {
    return 0;
  }
  while (ob->shadowed != nullptr && ob->shadowed != current_object) {
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
    if (CONFIG_INT(__RC_NONINTERACTIVE_STDERR_WRITE__)) {
      putc(']', stderr);
      fwrite(data, len, 1, stderr);
    }
    return;
  }

  inet_packets++;

  auto ip = who->interactive;
  switch (ip->connection_type) {
    case PORT_ASCII:
    case PORT_TELNET: {
      // Handle charset transcoding
      auto transdata = const_cast<char *>(data);
      auto translen = len;

      if (ip->trans) {
        UErrorCode error_code = U_ZERO_ERROR;

        auto required = ucnv_fromAlgorithmic(ip->trans, UConverterType::UCNV_UTF8, nullptr, 0, data,
                                             len, &error_code);
        if (error_code == U_BUFFER_OVERFLOW_ERROR) {
          translen = required;
          transdata = (char *)DMALLOC(translen, TAG_TEMPORARY, "add_message (translate)");

          error_code = U_ZERO_ERROR;
          auto written = ucnv_fromAlgorithmic(ip->trans, UConverterType::UCNV_UTF8, transdata,
                                              translen, data, len, &error_code);
          DEBUG_CHECK(written != translen, "Bug: translation buffer size calculation error");
          if (U_FAILURE(error_code)) {
            debug_message("add_message: Translation failed!");
            transdata = const_cast<char *>(data);
            translen = len;
          };
        }
      }

      inet_volume += translen;
      if (ip->connection_type == PORT_TELNET) {
        telnet_send_text(ip->telnet, transdata, translen);
      } else {
        bufferevent_write(ip->ev_buffer, data, len);
      }

      if (transdata != data) {
        FREE(transdata);
      }
    } break;
    case PORT_WEBSOCKET: {
      if (ip->iflags & HANDSHAKE_COMPLETE) {
        websocket_send_text(ip->lws, data, len);
      } else {
        debug_message("User hasn't completed websocket upgrade! can't send message.\n");
      }
      break;
    }
    default: {
      inet_volume += len;
      bufferevent_write(ip->ev_buffer, data, len);
      break;
    }
  }

#ifdef SHADOW_CATCH_MESSAGE
  /*
   * shadow handling.
   */
  if (shadow_catch_message(who, data)) {
    if (CONFIG_INT(__RC_SNOOP_SHADOWED__)) {
      handle_snoop(data, len, ip);
    }
    return;
  }
#endif /* NO_SHADOWS */
  handle_snoop(data, len, ip);

  add_message_calls++;
} /* add_message() */

void add_vmessage(object_t *who, const char *format, ...) {
  va_list args, args2;
  va_start(args, format);
  va_copy(args2, args);
  static char buf[LARGEST_PRINTABLE_STRING + 1];
  do {
    auto result = vsnprintf(buf, sizeof(buf), format, args);
    if (result < 0) {
      DEBUG_CHECK(result < 0, "Invalid format string: add_vmessage");
      break;
    }
    if (result <= sizeof(buf)) {
      add_message(who, buf, result);
    } else {
      std::unique_ptr<char[]> msg(new char[result + 1]);
      result = vsnprintf(msg.get(), result + 1, format, args2);
      if (result < 0) break;
      add_message(who, msg.get(), result);
    }
  } while (false);
  va_end(args2);
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

  // Flush things normally
  if (ip->ev_buffer) {
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

  text_space = sizeof(buf);

  debug(connections, "get_user_data: USER %d\n", ip->fd);

  /* compute how much data we can read right now */
  switch (ip->connection_type) {
    case PORT_WEBSOCKET:
      // Impossible, we don't handle it here.
      break;
    case PORT_TELNET:
      text_space = sizeof(ip->text) - ip->text_end;

      /* check if we need more space */
      if (text_space < sizeof(ip->text) / 16) {
        if (ip->text_start > 0) {
          memmove(ip->text, ip->text + ip->text_start, ip->text_end - ip->text_start);
          text_space += ip->text_start;
          ip->text_end -= ip->text_start;
          ip->text_start = 0;
        }
        if (text_space < sizeof(ip->text) / 16) {
          ip->iflags |= SKIP_COMMAND;
          ip->text_start = ip->text_end = 0;
          text_space = sizeof(ip->text);
        }
      }
      break;

    case PORT_MUD:
      if (ip->text_end < 4) {
        text_space = 4 - ip->text_end;
      } else {
        text_space = *reinterpret_cast<volatile int *>(ip->text) - ip->text_end + 4;
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
      // Impossible, we don't handle it here
      break;
    case PORT_TELNET: {
      int start = ip->text_end;

      // this will read data into ip->text
      telnet_recv(ip->telnet, reinterpret_cast<const char *>(&buf[0]), num_bytes);

      // If we read something
      if (ip->text_end > start) {
        /* handle snooping - snooper does not see type-ahead due to
         telnet being in linemode */
        if (!(ip->iflags & NOECHO)) {
          handle_snoop(ip->text + start, ip->text_end - start, ip);
        }

        // search for command.
        if (cmd_in_buf(ip)) {
          ip->iflags |= CMD_IN_BUF;
          struct timeval zero_sec = {0, 0};
          evtimer_del(ip->ev_command);
          evtimer_add(ip->ev_command, &zero_sec);
        }
      }
      break;
    }
    case PORT_MUD:
      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      if (num_bytes == text_space) {
        if (ip->text_end == 4) {
          *reinterpret_cast<volatile int *>(ip->text) = ntohl(*reinterpret_cast<int *>(ip->text));
          if (*reinterpret_cast<volatile int *>(ip->text) > MAX_TEXT - 5) {
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
          set_eval(max_eval_cost);
          safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
        }
      }
      break;

    case PORT_ASCII: {
      char *nl, *p;

      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      p = ip->text + ip->text_start;
      while ((nl = reinterpret_cast<char *>(memchr(p, '\n', ip->text_end - ip->text_start)))) {
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
          set_eval(max_eval_cost);
          safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
        }

        if (ip->text_start == ip->text_end) {
          ip->text_start = ip->text_end = 0;
          break;
        }

        p = nl + 1;
      }
    } break;

    case PORT_BINARY: {
      buffer_t *buffer;

      buffer = allocate_buffer(num_bytes);
      memcpy(buffer->item, buf, num_bytes);

      push_refed_buffer(buffer);
      set_eval(max_eval_cost);
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
    } break;
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

void on_user_websocket_received(interactive_t *ip, const char *data, size_t len) {
  if (!len) {
    return;
  }

  auto text_space = sizeof(ip->text) - ip->text_end;

  /* check if we need more space */
  if (text_space < len) {
    if (ip->text_start > 0) {
      memmove(ip->text, ip->text + ip->text_start, ip->text_end - ip->text_start);
      text_space += ip->text_start;
      ip->text_end -= ip->text_start;
      ip->text_start = 0;
    }
    if (text_space < len) {
      ip->iflags |= SKIP_COMMAND;
      ip->text_start = ip->text_end = 0;
      text_space = sizeof(ip->text);
    }
  }

  on_user_input(ip, data, len);

  if (cmd_in_buf(ip)) {
    ip->iflags |= CMD_IN_BUF;

    maybe_schedule_user_command(ip);
  }
}

// ANSI
static const int ANSI_SUBSTITUTE = 0x20;

// Used by both telnet and ws_ascii, in case of telnet, default is linemode, which means
// client will actually send entire line. In ascii mode, we will get an single char input
// each time.
void on_user_input(interactive_t *ip, const char *data, size_t len) {
  for (int i = 0; i < len; i++) {
    if (ip->text_end == sizeof(ip->text) - 1) {
      // No more space
      break;
    }
    auto c = static_cast<unsigned char>(data[i]);
    switch (c) {
      case 0x08:  // BACKSPACE
      case 0x7f:  // DEL
        if (ip->iflags & SINGLE_CHAR) {
          ip->text[ip->text_end++] = c;
        } else {
          if (ip->text_end > 0) {
            ip->text_end--;
          }
        }
        break;
      case 0x1b:
        if (CONFIG_INT(__RC_NO_ANSI__) && CONFIG_INT(__RC_STRIP_BEFORE_PROCESS_INPUT__)) {
          ip->text[ip->text_end++] = ANSI_SUBSTITUTE;
          break;
        }
        // fallthrough
      default:
        ip->text[ip->text_end++] = c;
        break;
    }
  }
}

// Also used by ws_ascii.
int cmd_in_buf(interactive_t *ip) {
  char *p;

  /* do standard input buffer cleanup */
  if (!clean_buf(ip)) {
    return 0;
  }

  /* if we're in single character mode, we've got input */
  if (ip->iflags & SINGLE_CHAR) {
    return 1;
  }

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
    return nullptr;
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
  while (ip->text_start < ip->text_end && ip->text[ip->text_start] != '\n' &&
         ip->text[ip->text_start] != '\r') {
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
  char *user_command = nullptr;

  if (!ip || !ip->ob || (ip->ob->flags & O_DESTRUCTED)) {
    return nullptr;
  }

  /* if there's a command in the buffer, pull it out! */
  if (ip->iflags & CMD_IN_BUF) {
    user_command = first_cmd_in_buf(ip);
  }

  /* no command found - return NULL */
  if (!user_command) {
    return nullptr;
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

static int escape_command(interactive_t *ip, char *user_command) {
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
    auto command = string_copy(ret->u.string, __CURRENT_FILE_LINE__);
    DEFER { FREE_MSTR(command); };
    parse_command(command, command_giver);
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
    DEBUG_FATAL("BUG: process_user_command.");
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

  if (ip->iflags & USING_MXP) {
    if (user_command[0] == ' ' && user_command[1] == '[' && user_command[3] == 'z') {
      if (!on_receive_mxp_tag(ip, user_command)) {
        goto exit;
      }
    }
  }

  if (escape_command(ip, user_command)) {
    if (ip->iflags & SINGLE_CHAR) {
      /* only 1 char ... switch to line buffer mode */
      ip->iflags |= WAS_SINGLE_CHAR;
      ip->iflags &= ~SINGLE_CHAR;
      ip->text_start = ip->text_end = *ip->text = 0;
      set_linemode(ip, true);
    } else {
      if (ip->iflags & WAS_SINGLE_CHAR) {
        /* we now have a string ... switch back to char mode */
        ip->iflags &= ~WAS_SINGLE_CHAR;
        ip->iflags |= SINGLE_CHAR;
        set_charmode(ip, true);
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
    maybe_schedule_user_command(ip);
  }

  current_interactive = nullptr;
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
    set_eval(max_eval_cost);
    safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);
    restore_command_giver();
  }

#ifndef NO_SNOOP
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
    ip->snooped_by = nullptr;
  }
#endif

  // Cleanup events
  if (ip->ev_buffer != nullptr) {
    bufferevent_free(ip->ev_buffer);
    ip->ev_buffer = nullptr;
  }
  if (ip->ev_command != nullptr) {
    evtimer_del(ip->ev_command);
    event_free(ip->ev_command);
    ip->ev_command = nullptr;
  }

  // Free telnet handle
  if (ip->telnet != nullptr) {
    telnet_free(ip->telnet);
    ip->telnet = nullptr;
  }

  // Free LWS handle
  if (ip->lws != nullptr) {
    close_user_websocket(ip->lws);
    ip->lws = nullptr;
  }

  // Free translator
  if (ip->trans != nullptr) {
    ucnv_close(ip->trans);
    ip->trans = nullptr;
  }

  clear_notify(ip->ob);

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to) {
    free_object(&ip->input_to->ob, "remove_interactive");
    free_sentence(ip->input_to);
    if (ip->num_carry > 0) {
      free_some_svalues(ip->carryover, ip->num_carry);
    }
    ip->carryover = nullptr;
    ip->num_carry = 0;
    ip->input_to = nullptr;
  }
#endif

  user_del(ip);
  FREE(ip);
  ob->interactive = nullptr;
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
    i->input_to = nullptr;
    if (i->num_carry) {
      free_some_svalues(i->carryover, i->num_carry);
    }
    i->carryover = nullptr;
    i->num_carry = 0;
    i->input_to = nullptr;
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
    function = nullptr;
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
    i->carryover = nullptr;
  } else {
    args = nullptr;
  }

  i->input_to = nullptr;
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

  // FIXME: this logic can be combined with above.
  if (was_single && !(i->iflags & SINGLE_CHAR)) {
    i->text_start = i->text_end = 0;
    i->text[0] = '\0';
    i->iflags &= ~CMD_IN_BUF;
    set_linemode(i, true);
  }
  if (was_noecho && !(i->iflags & NOECHO)) {
    set_localecho(i, true);
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
  // NOTE: we can't use "i" here anymore, it is possible that it
  // has been freed.
  pop_stack(); /* remove `function' from stack */
  return (1);
} /* call_function_interactive() */

int set_call(object_t *ob, sentence_t *sent, int flags) {
  if (ob == nullptr || sent == nullptr) {
    return (0);
  }
  auto ip = ob->interactive;
  if (ip == nullptr || ip->input_to) {
    return (0);
  }
  ip->input_to = sent;
  ip->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
  if (flags & I_NOECHO) {
    set_localecho(ip, false);
  }
  if (flags & I_SINGLE_CHAR) {
    set_charmode(ip);
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
  if (!ip || !ip->ob || !IP_VALID(ip, ip->ob)) {
    return;
  }

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to != nullptr) {
    // while in single char mode there won't e any prompt.
    return;
  }
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
    ip->iflags &= ~HAS_WRITE_PROMPT;
    tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
  }
  // Stavros: A lot of clients use this TELNET_GA to differentiate
  // prompts from other text
  if (ip->telnet && (ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
    telnet_send_ga(ip->telnet);
  }
} /* print_prompt() */

#ifndef NO_SNOOP
static void receive_snoop(const char *buf, int len, object_t *snooper) {
  /* command giver no longer set to snooper */
  if (CONFIG_INT(__RC_RECEIVE_SNOOP__)) {
    char *str;

    str = new_string(len, "receive_snoop");
    memcpy(str, buf, len);
    str[len] = 0;
    push_malloced_string(str);
    set_eval(max_eval_cost);
    safe_apply(APPLY_RECEIVE_SNOOP, snooper, 1, ORIGIN_DRIVER);
  } else {
    /* snoop output is now % in all cases */
    add_message(snooper, "%", 1);
    add_message(snooper, buf, len);
  }
}
#endif

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
          user->snooped_by = nullptr;
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
    tmp = (tmp->interactive ? tmp->interactive->snooped_by : nullptr);
  }

  /*
   * Terminate previous snoop, if any.
   */
  new_set_snoop(by, nullptr);

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
    return nullptr;
  }
  return ob->interactive->snooped_by;
} /* query_snoop() */

object_t *query_snooping(object_t *ob) {
  if (!(ob->flags & O_SNOOP)) {
    return nullptr;
  }
  for (auto &user : users()) {
    if (user->snooped_by == ob) {
      return user->ob;
    }
  }
  DEBUG_FATAL("couldn't find snoop target.\n");
  return nullptr;
} /* query_snooping() */
#endif

int query_idle(object_t *ob) {
  if (!ob->interactive) {
    error("query_idle() of non-interactive object.\n");
  }
  return (get_current_time() - ob->interactive->last_time);
} /* query_idle() */

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

  snprintf(result, sizeof(result), strchr(host, ':') != nullptr ? "[%s]:%s" : "%s:%s", host,
           service);

  return result;
}
