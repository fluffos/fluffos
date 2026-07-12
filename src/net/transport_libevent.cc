/*
 * transport_libevent.cc -- the native socket transports.
 *
 * Implements net/transport.h over libevent for the two native connection
 * flavours (plain/TLS sockets via bufferevent, websockets via
 * libwebsockets), plus everything that creates such connections: the
 * listening ports (init_user_conn), the accept handler, the per-user
 * command timer, and the socket read path.
 *
 * comm.cc contains the transport-agnostic half of connection handling;
 * the WASM build compiles wasm/comm_wasm.cc instead of this file.
 */

#include "base/std.h"

#include "comm.h"
#include "net/transport_native.h"

#include <event2/buffer.h>       // for evbuffer_freeze, etc
#include <event2/bufferevent.h>  // for bufferevent_enable, etc
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>     // for EV_TIMEOUT, etc
#include <event2/listener.h>  // for evconnlistener_free, etc
#include <event2/util.h>      // for evutil_closesocket, etc
#include <cstring>            // for memcpy
// Network stuff
#ifndef _WIN32
#include <netdb.h>        // for addrinfo, freeaddrinfo, etc
#include <netinet/in.h>   // for ntohl, IPPROTO_TCP
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <sys/socket.h>   // for SOCK_STREAM
#else
#include <ws2tcpip.h>
#endif

#include "backend.h"
#include "interactive.h"
#include "net/telnet.h"
#include "net/tls.h"
#include "net/transport.h"
#include "net/websocket.h"
#include "user.h"
#include "vm/vm.h"

#include "ghc/filesystem.hpp"
namespace fs = ghc::filesystem;

namespace {

void get_user_data(interactive_t* ip);

void on_user_command(evutil_socket_t fd, short what, void* arg) {
  debug(event, "User has an full command ready: %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");
  auto* user = reinterpret_cast<interactive_t*>(arg);

  if (user == nullptr) {
    DEBUG_FATAL("on_user_command: user == NULL, Driver BUG.");
    return;
  }

  // NOTE: It is important to only execute one command here, then schedule next
  // command at the tail, This ensure users have a fair chance that no one can
  // keep running commands.
  //
  // currently command scehduling is done inside process_user_command().
  comm_run_scheduled_command(user);
}

/*
 * Common base for the libevent-backed transports: both use the per-user
 * zero-delay command timer (ip->ev_command) to schedule command
 * execution on the main loop.
 */
class LibeventTransportBase : public Transport {
 public:
  explicit LibeventTransportBase(interactive_t* ip) : ip_(ip) {}

  void schedule_command() override {
    struct timeval zero_sec = {0, 0};
    evtimer_del(ip_->ev_command);
    evtimer_add(ip_->ev_command, &zero_sec);
  }

  void close() override {
    if (ip_->ev_command != nullptr) {
      evtimer_del(ip_->ev_command);
      event_free(ip_->ev_command);
      ip_->ev_command = nullptr;
    }
  }

 protected:
  interactive_t* ip_;
};

// Plain or TLS socket, through a libevent bufferevent.
class SocketTransport final : public LibeventTransportBase {
 public:
  using LibeventTransportBase::LibeventTransportBase;

  void write(const char* data, size_t len) override {
    bufferevent_write(ip_->ev_buffer, data, len);
  }

  int flush() override {
    if (!ip_->ev_buffer) {
      return 0;
    }
    // Try to flush things normally
    if (bufferevent_flush(ip_->ev_buffer, EV_WRITE, BEV_FLUSH) == -1) return 0;

    // For socket based bufferevent, bufferevent_flush is actually a no-op, thus we have to
    // implement our own.
    if (ip_->ssl) {
      auto* ssl = bufferevent_openssl_get_ssl(ip_->ev_buffer);
      auto* output = bufferevent_get_output(ip_->ev_buffer);
      auto len = evbuffer_get_length(output);
      if (len > 0) {
        evbuffer_freeze(output, 1);
        auto* data = evbuffer_pullup(output, len);
        auto wrote = SSL_write(ssl, data, len);
        // must left unfreezed
        // https://github.com/libevent/libevent/issues/1469
        evbuffer_unfreeze(output, 1);
        if (wrote > 0) {
          evbuffer_drain(output, wrote);
        }
        return wrote > 0;
      }
    } else {
      auto fd = bufferevent_getfd(ip_->ev_buffer);
      if (fd == -1) {
        return 0;
      }
      auto* output = bufferevent_get_output(ip_->ev_buffer);
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

  void close() override {
    // Cleanup events, must happen after ssl.
    if (ip_->ev_buffer != nullptr) {
      // see http://www.wangafu.net/~nickm/libevent-book/Ref6a_advanced_bufferevents.html
      if (ip_->ssl) {
        SSL_set_shutdown(ip_->ssl, SSL_RECEIVED_SHUTDOWN);
        SSL_shutdown(ip_->ssl);
        ip_->ssl = nullptr;
      }
      bufferevent_free(ip_->ev_buffer);
      ip_->ev_buffer = nullptr;
    }
    LibeventTransportBase::close();
  }
};

// Websocket connection, through libwebsockets.
class WebsocketTransport final : public LibeventTransportBase {
 public:
  using LibeventTransportBase::LibeventTransportBase;

  void write(const char* data, size_t len) override {
    if (!(ip_->iflags & HANDSHAKE_COMPLETE)) {
      debug_message("User hasn't completed websocket upgrade! can't send message.\n");
      return;
    }
    websocket_send_text(ip_->lws, data, len);
  }

  int flush() override {
    // libwebsockets writes are scheduled on its own service loop.
    return 0;
  }

  bool compresses_stream() const override {
    // permessage-deflate: the telnet layer must refuse MCCP on top.
    return true;
  }

  void close() override {
    if (ip_->lws != nullptr) {
      close_user_websocket(ip_->lws);
      ip_->lws = nullptr;
    }
    LibeventTransportBase::close();
  }
};

void on_user_read(bufferevent* /*bev*/, void* arg) {
  auto* user = reinterpret_cast<interactive_t*>(arg);

  if (user == nullptr) {
    DEBUG_FATAL("on_user_read: user == NULL, Driver BUG.");
    return;
  }

  // Read user input
  get_user_data(user);

  // TODO: currently get_user_data() will schedule command execution.
  // should probably move it here.
}

void on_user_write(bufferevent* /*bev*/, void* arg) {
  auto* user = reinterpret_cast<interactive_t*>(arg);
  if (user == nullptr) {
    DEBUG_FATAL("on_user_write: user == NULL, Driver BUG.");
    return;
  }
  // nothing to do.
}

void on_user_events(bufferevent* /*bev*/, short events, void* arg) {
  auto* user = reinterpret_cast<interactive_t*>(arg);

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

void new_user_event_listener(event_base* base, interactive_t* user) {
  auto options = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;
  auto* bev = user->ssl ? bufferevent_openssl_socket_new(base, user->fd, user->ssl,
                                                         BUFFEREVENT_SSL_ACCEPTING, options)
                        : bufferevent_socket_new(base, user->fd, options);

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
void new_conn_handler(evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr,
                      int addrlen, void* arg) {
  debug(connections, "New connection from %s.\n", sockaddr_to_string(addr, addrlen));

  // TODO: we don't really need to pass in port, we can figure out by
  // evconnlistener_get_fd and compare it
  auto* port = reinterpret_cast<port_def_t*>(arg);

  {
    int one = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
#ifndef _WIN32
                   &one,
#else
                   (const char*)&one,
#endif
                   sizeof(one)) == -1) {
      debug(connections,
            "new_conn_handler: user fd %" FMT_SOCKET_FD ", set_socket_tcp_nodelay error: %s.\n", fd,
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
    }
  }

  if (port->kind == PORT_TYPE_WEBSOCKET) {
    // For websocket connections, wait until they are handshake finished.
    init_user_websocket(port->lws_context, fd);
    return;
  } else {
    // For other connections go straight to no handshake necessary, schedule to logon.
    auto* base = evconnlistener_get_base(listener);

    auto* user = new_user(port, fd, addr, addrlen);
    new_user_event_listener(base, user);

    if (user->connection_type == PORT_TYPE_TELNET) {
      user->telnet = net_telnet_init(user);
      send_initial_telnet_negotiations(user);
    }

    event_base_once(
        base, -1, EV_TIMEOUT,
        [](evutil_socket_t /*fd*/, short /*what*/, void* arg) {
          auto* user = reinterpret_cast<interactive_t*>(arg);
          on_user_logon(user);
        },
        (void*)user, nullptr);
  }
  debug(connections, ("new_conn_handler: end\n"));
} /* new_conn_handler() */

}  // namespace

// Initialize an new user
interactive_t* new_user(port_def_t* port, evutil_socket_t fd, sockaddr* addr,
                        ev_socklen_t addrlen) {
  /*
   * initialize new user interactive data structure.
   */
  auto* user = user_add();

  user->connection_type = port->kind;
  user->ob = master_ob;
  user->last_time = get_current_time();
  user->trans = nullptr;
  user->fd = fd;
  user->local_port = port->port;
  user->external_port = (port - external_port);  // FIXME: pointer arith
  memcpy(&user->addr, addr, addrlen);
  user->addrlen = addrlen;
  if (port->ssl) {
    user->ssl = tls_get_client_ctx(port->ssl);
  }

  user->transport = (port->kind == PORT_TYPE_WEBSOCKET)
                        ? static_cast<Transport*>(new WebsocketTransport(user))
                        : static_cast<Transport*>(new SocketTransport(user));

  // Command handler
  auto* base = evconnlistener_get_base(port->ev_conn);
  user->ev_command = evtimer_new(base, on_user_command, user);

  return user;
}

namespace {
/*
 * Read pending data for a user into user->interactive->text.
 * This also does telnet negotiation.
 */
void get_user_data(interactive_t* ip) {
  int num_bytes, text_space;
  unsigned char buf[MAX_TEXT];

  text_space = sizeof(buf);

  debug(connections, "get_user_data: USER %d\n", ip->fd);

  /* compute how much data we can read right now */
  switch (ip->connection_type) {
    case PORT_TYPE_WEBSOCKET:
      // Impossible, we don't handle it here.
      break;
    case PORT_TYPE_TELNET:
      text_space = comm_reserve_input_space(ip, sizeof(ip->text) / 16);
      break;

    case PORT_TYPE_MUD:
      if (ip->text_end < 4) {
        text_space = 4 - ip->text_end;
      } else {
        text_space = *reinterpret_cast<volatile int*>(ip->text) - ip->text_end + 4;
      }
      break;

    default:
      text_space = sizeof(buf);
      break;
  }

  /* read the data from the socket */
  debug(connections, "get_user_data: read on fd %d\n", ip->fd);

  // Never read more than buf holds. A malicious PORT_TYPE_MUD length prefix
  // can make text_space negative, which would convert to a huge size_t and
  // overflow buf; clamp to [0, sizeof(buf)] here regardless of protocol.
  if (text_space < 0) {
    text_space = 0;
  }
  if (text_space > static_cast<int>(sizeof(buf))) {
    text_space = sizeof(buf);
  }

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
    case PORT_TYPE_WEBSOCKET:
      // Impossible, we don't handle it here
      break;
    case PORT_TYPE_TELNET:
      // Telnet negotiation, snoop and command detection; shared with the
      // websocket-telnet and WASM console paths.
      comm_telnet_received(ip, reinterpret_cast<const char*>(&buf[0]), num_bytes);
      break;

    case PORT_TYPE_MUD:
      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      if (num_bytes == text_space) {
        if (ip->text_end == 4) {
          *reinterpret_cast<volatile int*>(ip->text) = ntohl(*reinterpret_cast<int*>(ip->text));
          int const msg_len = *reinterpret_cast<volatile int*>(ip->text);
          // Reject non-positive (a negative prefix would drive a negative
          // text_space) and oversized message lengths.
          if (msg_len <= 0 || msg_len > MAX_TEXT - 5) {
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

    case PORT_TYPE_ASCII: {
      char *nl, *p;

      memcpy(ip->text + ip->text_end, buf, num_bytes);
      ip->text_end += num_bytes;

      p = ip->text + ip->text_start;
      while ((nl = reinterpret_cast<char*>(memchr(p, '\n', ip->text_end - ip->text_start)))) {
        ip->text_start = (nl + 1) - ip->text;

        *nl = 0;
        if (*(nl - 1) == '\r') {
          *--nl = 0;
        }

        if (!(ip->ob->flags & O_DESTRUCTED)) {
          char* str;

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

    case PORT_TYPE_BINARY: {
      buffer_t* buffer;

      buffer = allocate_buffer(num_bytes);
      memcpy(buffer->item, buf, num_bytes);

      push_refed_buffer(buffer);
      set_eval(max_eval_cost);
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
    } break;
  }
}
}  // namespace

/*
 * Initialize new user connection socket.
 */
bool init_user_conn() {
  for (auto& port : external_port) {
#ifdef F_NETWORK_STATS
    port.in_packets = 0;
    port.in_volume = 0;
    port.out_packets = 0;
    port.out_volume = 0;
#endif
    if (!port.port) continue;
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
                     (void*)&one,
#else
                     (const char*)&one,
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
      if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&zero, sizeof(zero)) == -1) {
        debug_message("socket_create: setsockopt error: %s.\n",
                      evutil_socket_error_to_string(evutil_socket_geterror(fd)));
        evutil_closesocket(fd);
        return false;
      }
    }
#endif
#endif
    // Enable TLS
    {
      if (!port.tls_cert.empty() && !port.tls_key.empty()) {
        debug_message("Processing TLS config for port %d...\n", port.port);
        auto mudlib_path = fs::u8path(CONFIG_STR(__MUD_LIB_DIR__));
        auto real_cert_path = mudlib_path / port.tls_cert;
        auto real_key_path = mudlib_path / port.tls_key;
        try {
          if (!fs::exists(real_cert_path)) {
            debug_message("cert file missing: %s.\n", real_cert_path.c_str());
            return false;
          }
          if (!fs::exists(real_key_path)) {
            debug_message("key file missing: %s.\n", real_key_path.c_str());
            return false;
          }
          port.tls_cert = fs::absolute(real_cert_path).string();
          port.tls_key = fs::absolute(real_key_path).string();
        } catch (fs::filesystem_error& e) {
          debug_message("Error: %s (%d).\n", e.what(), e.code().value());
          return false;
        }
      }
    }
    {
      /*
       * fill in socket address information.
       */
      struct addrinfo* res;

      char service[NI_MAXSERV];
      snprintf(service, sizeof(service), "%u", port.port);

      // Must be initialized to all zero.
      struct addrinfo hints = {0};
#ifdef IPV6
      hints.ai_family = AF_INET6;
#ifdef AI_V4MAPPED
      hints.ai_flags |= AI_V4MAPPED;
#endif
#else
      hints.ai_family = AF_INET;
#endif
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags |= AI_PASSIVE | AI_NUMERICSERV;

      int ret;

      auto* mudip = CONFIG_STR(__MUD_IP__);
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

      // Websocket TLS is handled in init_websocket_context
      if (!port.tls_cert.empty() && port.kind != PORT_TYPE_WEBSOCKET) {
        SSL_CTX* ctx = tls_server_init(port.tls_cert, port.tls_key);
        if (!ctx) {
          debug_message("Unable to create TLS context.\n");
          evutil_closesocket(fd);
          return false;
        }
        port.ssl = ctx;
      }

      debug_message("Accepting %s%s connections on %s.\n", port_kind_name(port.kind),
                    !port.tls_cert.empty() ? "(TLS)" : "",
                    sockaddr_to_string(res->ai_addr, res->ai_addrlen));
      evutil_freeaddrinfo(res);
    }

    // Listen on connection event
    auto* conn = evconnlistener_new(
        g_event_base, new_conn_handler, &port,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC, 1024, fd);
    if (conn == nullptr) {
      debug_message("listening failed: %s !", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
      return false;
    }
    port.ev_conn = conn;
    port.fd = fd;
    if (port.kind == PORT_TYPE_WEBSOCKET) {
      port.lws_context = init_websocket_context(g_event_base, &port);
    }
  }
  return true;
}

/*
 * Shut down new user accept file descriptor.
 */
void shutdown_external_ports() {
  for (auto& port : external_port) {
    if (!port.port) {
      continue;
    }
    if (port.ssl) tls_server_close(port.ssl);
    // will also close the FD.
    if (port.ev_conn) evconnlistener_free(port.ev_conn);
    if (port.lws_context) close_websocket_context(port.lws_context);
  }

  debug_message("closed external ports\n");
}
