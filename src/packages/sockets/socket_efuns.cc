
/*
 * socket_efun.c -- socket efuns for MudOS.
 *    5-92 : Dwayne Fontenot (Jacques@TMI) : original coding.
 *   10-92 : Dave Richards (Cynosure) : less original coding.
 */

#include "base/package_api.h"

#include "packages/sockets/socket_efuns.h"

#include <event2/event.h>
#include <event2/util.h>
#include <deque>
#include <string>
#include <unistd.h>  // for close

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define ERR(e) WSA##e
#else
#include <errno.h>
#define ERR(e) e
#endif

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)

struct lpc_socket_event_data {
  int idx;
};

namespace {
// Hold all the LPC socks
std::deque<lpc_socket_t> lpc_socks;

void on_lpc_sock_read(evutil_socket_t fd, short what, void *arg) {
  debug(event, "Got an event on socket %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  auto data = reinterpret_cast<lpc_socket_event_data *>(arg);
  socket_read_select_handler(data->idx);
}
void on_lpc_sock_write(evutil_socket_t fd, short what, void *arg) {
  debug(event, "Got an event on socket %d:%s%s%s%s \n", (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "", (what & EV_SIGNAL) ? " signal" : "");

  auto data = reinterpret_cast<lpc_socket_event_data *>(arg);
  socket_write_select_handler(data->idx);
}

}  // namespace

// Initialize LPC socket data structure and register events
void new_lpc_socket_event_listener(int idx, lpc_socket_t *sock, evutil_socket_t real_fd) {
  auto data = new lpc_socket_event_data;
  data->idx = idx;
  sock->ev_read = event_new(g_event_base, real_fd, EV_READ | EV_PERSIST, on_lpc_sock_read, data);
  sock->ev_write = event_new(g_event_base, real_fd, EV_WRITE, on_lpc_sock_write, data);
  sock->ev_data = data;
}

/* flags for socket_close */
#define SC_FORCE 1u
#define SC_DO_CALLBACK 2u
#define SC_FINAL_CLOSE 4u

#ifdef PACKAGE_SOCKETS
static char *sockaddr_to_lpcaddr(struct sockaddr *addr /*addr*/, ev_socklen_t /*len*/ len);
#endif

const char *error_strings[ERROR_STRINGS] = {"Problem creating socket",
                                            "Problem with setsockopt",
                                            "Problem setting non-blocking mode",
                                            "No more available efun sockets",
                                            "Descriptor out of range",
                                            "Socket is closed",
                                            "Security violation attempted",
                                            "Socket is already bound",
                                            "Address already in use",
                                            "Problem with bind",
                                            "Problem with getsockname",
                                            "Socket mode not supported",
                                            "Socket not bound to an address",
                                            "Socket is already connected",
                                            "Problem with listen",
                                            "Socket not listening",
                                            "Operation would block",
                                            "Interrupted system call",
                                            "Problem with accept",
                                            "Socket is listening",
                                            "Problem with address format",
                                            "Operation already in progress",
                                            "Connection refused",
                                            "Problem with connect",
                                            "Socket not connected",
                                            "Object type not supported",
                                            "Problem with sendto",
                                            "Problem with send",
                                            "Wait for callback",
                                            "Socket already released",
                                            "Socket not released",
                                            "Data nested too deeply"};

/*
 * Convert a string representation of an address to a sockaddr_in
 * The format of the string is "ip port", the delimiter is a whitespace.
 */
static bool lpcaddr_to_sockaddr(const char *name, struct sockaddr *addr, ev_socklen_t *len) {
  const char *cp = strchr(name, ' ');
  if (cp == nullptr) {
    debug(sockets, "lpcaddr_to_sockaddr: malformed address: %s", name);
    return false;
  }

  char host[NI_MAXHOST], service[NI_MAXSERV];

  memset(host, 0, sizeof(host));
  memset(service, 0, sizeof(service));

  memcpy(host, name, cp - name);
  strncpy(service, cp + 1, sizeof(service) - 1);

  struct addrinfo hints = {0}, *res;
#ifdef IPV6
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags |= AI_V4MAPPED;
#else
  hints.ai_family = AF_INET;
#endif
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  // LPC name resolution is done through other efun.
  // this efun only support numeric address and service
  hints.ai_flags |= EVUTIL_AI_NUMERICHOST | EVUTIL_AI_NUMERICSERV;
  // Make sure we only see address that the system supports.
  hints.ai_flags |= EVUTIL_AI_ADDRCONFIG;

  int ret;
  if ((ret = evutil_getaddrinfo(host, service, &hints, &res))) {
    debug(sockets, "lpcaddr_to_sockaddr: getaddrinfo error: %s.\n", evutil_gai_strerror(ret));
    return false;
  }

  // Just take first result, ignore rest.
  memcpy(addr, res->ai_addr, res->ai_addrlen);
  if (len != nullptr) *len = res->ai_addrlen;

  evutil_freeaddrinfo(res);

  return true;
}

/*
 * check permission
 */
int check_valid_socket(const char *const what, int fd, object_t *owner, const char *const addr,
                       int port) {
  array_t *info;
  svalue_t *mret;

  info = allocate_empty_array(4);
  info->item[0].type = T_NUMBER;
  info->item[0].u.number = fd;
  assign_socket_owner(&info->item[1], owner);
  info->item[2].type = T_STRING;
  info->item[2].subtype = STRING_SHARED;
  info->item[2].u.string = make_shared_string(addr);
  info->item[3].type = T_NUMBER;
  info->item[3].u.number = port;

  push_object(current_object);
  push_constant_string(what);
  push_refed_array(info);

  mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
  return MASTER_APPROVED(mret);
}

static void clear_socket(int which, int dofree) {
  if (dofree) {
    set_read_callback(which, nullptr);
    set_write_callback(which, nullptr);
    set_close_callback(which, nullptr);
  }

  lpc_socks[which].fd = -1;
  lpc_socks[which].flags = 0;
  lpc_socks[which].mode = MUD;
  lpc_socks[which].state = STATE_CLOSED;
  memset(reinterpret_cast<char *>(&lpc_socks[which].l_addr), 0, sizeof(lpc_socks[which].l_addr));
  memset(reinterpret_cast<char *>(&lpc_socks[which].r_addr), 0, sizeof(lpc_socks[which].r_addr));
  lpc_socks[which].l_addrlen = 0;
  lpc_socks[which].r_addrlen = 0;
  lpc_socks[which].owner_ob = nullptr;
  lpc_socks[which].release_ob = nullptr;
  lpc_socks[which].read_callback.s = nullptr;
  lpc_socks[which].write_callback.s = nullptr;
  lpc_socks[which].close_callback.s = nullptr;
  lpc_socks[which].r_buf = nullptr;
  lpc_socks[which].r_off = 0;
  lpc_socks[which].r_len = 0;
  lpc_socks[which].w_buf = nullptr;
  lpc_socks[which].w_off = 0;
  lpc_socks[which].w_len = 0;
  lpc_socks[which].ev_read = nullptr;
  lpc_socks[which].ev_write = nullptr;
}

/*
 * Set the callbacks for a socket
 */
void set_read_callback(int which, svalue_t *cb) {
  char *s;

  if (lpc_socks[which].flags & S_READ_FP) {
    free_funp(lpc_socks[which].read_callback.f);
    lpc_socks[which].flags &= ~S_READ_FP;
  } else if ((s = lpc_socks[which].read_callback.s)) {
    free_string(s);
  }

  if (cb) {
    if (cb->type == T_FUNCTION) {
      lpc_socks[which].flags |= S_READ_FP;
      lpc_socks[which].read_callback.f = cb->u.fp;
      cb->u.fp->hdr.ref++;
    } else {
      lpc_socks[which].read_callback.s = make_shared_string(cb->u.string);
    }
  } else {
    lpc_socks[which].read_callback.s = nullptr;
  }
}

void set_write_callback(int which, svalue_t *cb) {
  char *s;

  if (lpc_socks[which].flags & S_WRITE_FP) {
    free_funp(lpc_socks[which].write_callback.f);
    lpc_socks[which].flags &= ~S_WRITE_FP;
  } else if ((s = lpc_socks[which].write_callback.s)) {
    free_string(s);
  }

  if (cb) {
    if (cb->type == T_FUNCTION) {
      lpc_socks[which].flags |= S_WRITE_FP;
      lpc_socks[which].write_callback.f = cb->u.fp;
      cb->u.fp->hdr.ref++;
    } else {
      lpc_socks[which].write_callback.s = make_shared_string(cb->u.string);
    }
  } else {
    lpc_socks[which].write_callback.s = nullptr;
  }
}

void set_close_callback(int which, svalue_t *cb) {
  char *s;

  if (lpc_socks[which].flags & S_CLOSE_FP) {
    free_funp(lpc_socks[which].close_callback.f);
    lpc_socks[which].flags &= ~S_CLOSE_FP;
  } else if ((s = lpc_socks[which].close_callback.s)) {
    free_string(s);
  }

  if (cb) {
    if (cb->type == T_FUNCTION) {
      lpc_socks[which].flags |= S_CLOSE_FP;
      lpc_socks[which].close_callback.f = cb->u.fp;
      cb->u.fp->hdr.ref++;
    } else {
      lpc_socks[which].close_callback.s = make_shared_string(cb->u.string);
    }
  } else {
    lpc_socks[which].close_callback.s = nullptr;
  }
}

#ifdef PACKAGE_SOCKETS

static void copy_close_callback(int to, int from) {
  char *s;

  if (lpc_socks[to].flags & S_CLOSE_FP) {
    free_funp(lpc_socks[to].close_callback.f);
  } else if ((s = lpc_socks[to].close_callback.s)) {
    free_string(s);
  }

  if (lpc_socks[from].flags & S_CLOSE_FP) {
    lpc_socks[to].flags |= S_CLOSE_FP;
    lpc_socks[to].close_callback.f = lpc_socks[from].close_callback.f;
    lpc_socks[to].close_callback.f->hdr.ref++;
  } else {
    lpc_socks[to].flags &= ~S_CLOSE_FP;
    s = lpc_socks[to].close_callback.s = lpc_socks[from].close_callback.s;
    if (s) {
      ref_string(s);
    }
  }
}

#endif

int find_new_socket() {
  for (int i = 0; i < lpc_socks.size(); i++) {
    if (lpc_socks[i].state == STATE_CLOSED) {
      return i;
    }
  }
  lpc_socks.emplace_back(lpc_socket_t{0});
  return lpc_socks.size() - 1;
}

#ifdef PACKAGE_SOCKETS
/*
 * Create an LPC efun socket
 */
int socket_create(enum socket_mode mode, svalue_t *read_callback, svalue_t *close_callback) {
  int type, i, fd;
  int binary = 0;

  if (mode == STREAM_BINARY) {
    binary = 1;
    mode = STREAM;
  } else if (mode == DATAGRAM_BINARY) {
    binary = 1;
    mode = DATAGRAM;
  }
  switch (mode) {
    case MUD:
    case STREAM:
      type = SOCK_STREAM;
      break;
    case DATAGRAM:
      type = SOCK_DGRAM;
      break;

    default:
      return EEMODENOTSUPP;
  }
  i = find_new_socket();
  if (i >= 0) {
#ifdef IPV6
    fd = socket(AF_INET6, type, 0);
#else
    fd = socket(AF_INET, type, 0);
#endif
    if (fd == -1) {
      debug(sockets, "socket_create: socket error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      return EESOCKET;
    }
    if (evutil_make_listen_socket_reuseable(fd) == -1) {
      debug(sockets, "socket_create: setsockopt error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      evutil_closesocket(fd);
      return EESETSOCKOPT;
    }
    if (evutil_make_socket_nonblocking(fd) == -1) {
      debug(sockets, "socket_create: set_socket_nonblocking error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      evutil_closesocket(fd);
      return EENONBLOCK;
    }
    if (evutil_make_socket_closeonexec(fd) == -1) {
      debug(sockets, "socket_create: make_socket_closeonexec error: %s.\n",
            evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      evutil_closesocket(fd);
      return EESETSOCKOPT;
    }

#ifndef _WIN32
    // Set send buffer to 256K
    {
      int sendbuf = 256 * 1024;
      if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf)) == -1) {
        debug(sockets, "socket_create: setsockopt so_sndbuf error: %s.\n",
              evutil_socket_error_to_string(evutil_socket_geterror(fd)));
      }
    }
#endif

    new_lpc_socket_event_listener(i, &lpc_socks[i], fd);

    lpc_socks[i].fd = fd;
    lpc_socks[i].flags = S_HEADER;

    if (type == SOCK_DGRAM) {
      close_callback = nullptr;
    }
    set_read_callback(i, read_callback);
    set_write_callback(i, nullptr);
    set_close_callback(i, close_callback);

    if (binary) {
      lpc_socks[i].flags |= S_BINARY;
    }
    lpc_socks[i].mode = mode;
    lpc_socks[i].state = STATE_UNBOUND;
    lpc_socks[i].owner_ob = current_object;
    current_object->flags |= O_EFUN_SOCKET;

    debug(sockets, "socket_create: created lpc socket %d (real fd %d) mode %d\n", i, fd, mode);
  }

  return i;
}

/*
 * Bind an address to an LPC efun socket
 */
int socket_bind(int fd, int port, const char *addr) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].state != STATE_UNBOUND) {
    return EEISBOUND;
  }

  struct sockaddr_storage sockaddr = {0};
  socklen_t len = sizeof(sockaddr);

  if (addr != nullptr) {
    if (!lpcaddr_to_sockaddr(addr, reinterpret_cast<struct sockaddr *>(&sockaddr), &len)) {
      debug(sockets, "socket_bind: unable to parse: '%s'.\n", addr);
      return EEBADADDR;
    }
  } else {
    char service[NI_MAXSERV];
    snprintf(service, sizeof(service), "%u", port);

    // Must be initialized to all zero.
    struct addrinfo hints = {0};
#ifdef IPV6
    hints.ai_family = AF_INET6;
#else
    hints.ai_family = AF_INET;
#endif
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_V4MAPPED;
    hints.ai_protocol = 0; /* Any protocol */

    int ret;
    struct addrinfo *result = nullptr;

    auto mudip = CONFIG_STR(__MUD_IP__);
    if (mudip != nullptr && strlen(mudip) > 0) {
      debug(sockets, "socket_bind: binding to mud ip: %s.\n", mudip);
      ret = evutil_getaddrinfo(mudip, service, &hints, &result);
    } else {
      debug(sockets, "socket_bind: binding to any address.\n");
      ret = evutil_getaddrinfo(nullptr, service, &hints, &result);
    }
    if (ret) {
      debug(sockets, "socket_bind: error %s \n", evutil_gai_strerror(ret));
      return EEBADADDR;
    }

    memcpy(&sockaddr, result->ai_addr, result->ai_addrlen);
    len = result->ai_addrlen;
    evutil_freeaddrinfo(result);
  }

  if (bind(lpc_socks[fd].fd, reinterpret_cast<struct sockaddr *>(&sockaddr), len) == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    switch (e) {
      case ERR(EADDRINUSE):
        debug(sockets, "socket_bind: address is in use.");
        return EEADDRINUSE;
      default:
        debug(sockets, "socket_bind: bind error: %d(%s).\n", e, evutil_socket_error_to_string(e));
        return EEBIND;
    }
  }

  // fill-in socket information.
  lpc_socks[fd].l_addrlen = sizeof(lpc_socks[fd].l_addr);
  if (getsockname(lpc_socks[fd].fd, reinterpret_cast<struct sockaddr *>(&lpc_socks[fd].l_addr),
                  &lpc_socks[fd].l_addrlen) == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    debug(sockets, "socket_bind: getsockname error: %s.\n", evutil_socket_error_to_string(e));
    return EEGETSOCKNAME;
  }

  lpc_socks[fd].state = STATE_BOUND;

  debug(sockets, "socket_bind: bound lpc socket %d (real fd %lld) to %s.\n", fd, lpc_socks[fd].fd,
        sockaddr_to_string((struct sockaddr *)&lpc_socks[fd].l_addr, lpc_socks[fd].l_addrlen));

  // register read event.
  if (lpc_socks[fd].mode == DATAGRAM || lpc_socks[fd].mode == DATAGRAM_BINARY) {
    event_add(lpc_socks[fd].ev_read, nullptr);
  }

  return EESUCCESS;
}

/*
 * Listen for connections on an LPC efun socket
 */
int socket_listen(int fd, svalue_t *callback) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].mode == DATAGRAM) {
    return EEMODENOTSUPP;
  }
  if (lpc_socks[fd].state == STATE_UNBOUND) {
    return EENOADDR;
  }
  if (lpc_socks[fd].state != STATE_BOUND) {
    return EEISCONN;
  }

  if (listen(lpc_socks[fd].fd, 5) == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    debug(sockets, "socket_listen: %d (real fd %lld) listen error: %d (%s).\n", fd,
          lpc_socks[fd].fd, e, evutil_socket_error_to_string(e));
    return EELISTEN;
  }
  lpc_socks[fd].state = STATE_LISTEN;
  set_read_callback(fd, callback);

  current_object->flags |= O_EFUN_SOCKET;

  event_add(lpc_socks[fd].ev_read, nullptr);

  debug(sockets, "socket_listen: listen on socket %d\n", fd);

  return EESUCCESS;
}

/*
 * Accept a connection on an LPC efun socket
 */
int socket_accept(int fd, svalue_t *read_callback, svalue_t *write_callback) {
  int accept_fd, i;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].mode == DATAGRAM) {
    return EEMODENOTSUPP;
  }
  if (lpc_socks[fd].state != STATE_LISTEN) {
    return EENOTLISTN;
  }

  lpc_socks[fd].flags &= ~S_WACCEPT;

  addrlen = sizeof(addr);
  accept_fd = accept(lpc_socks[fd].fd, reinterpret_cast<struct sockaddr *>(&addr), &addrlen);
  if (accept_fd == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    switch (e) {
      case ERR(EWOULDBLOCK):
        return EEWOULDBLOCK;
      case ERR(EINTR):
        return EEINTR;
      default:
        debug(sockets, "socket_accept: accept error: %s.\n", evutil_socket_error_to_string(e));
        return EEACCEPT;
    }
  }

  if (evutil_make_socket_nonblocking(accept_fd) == -1) {
    debug(sockets, "socket_accept: set_socket_nonblocking 1 error: %s.\n",
          evutil_socket_error_to_string(evutil_socket_geterror(accept_fd)));
    close(accept_fd);
    return EENONBLOCK;
  }

  if (evutil_make_socket_closeonexec(fd) == -1) {
    debug(sockets, "socket_accept: make_socket_closeonexec error: %s.\n",
          evutil_socket_error_to_string(evutil_socket_geterror(fd)));
    close(fd);
    return EESETSOCKOPT;
  }

  i = find_new_socket();
  if (i >= 0) {
    new_lpc_socket_event_listener(i, &lpc_socks[i], accept_fd);

    lpc_socks[i].fd = accept_fd;
    lpc_socks[i].flags = S_HEADER | (lpc_socks[fd].flags & S_BINARY);

    lpc_socks[i].mode = lpc_socks[fd].mode;
    lpc_socks[i].state = STATE_DATA_XFER;

    lpc_socks[i].l_addrlen = sizeof(lpc_socks[i].l_addr);
    if (getsockname(lpc_socks[i].fd, reinterpret_cast<struct sockaddr *>(&lpc_socks[i].l_addr),
                    &lpc_socks[i].l_addrlen) == -1) {
      lpc_socks[i].l_addr = lpc_socks[fd].l_addr;
      lpc_socks[i].l_addrlen = lpc_socks[fd].l_addrlen;
    }

    lpc_socks[i].r_addr = addr;
    lpc_socks[i].r_addrlen = addrlen;
    lpc_socks[i].owner_ob = current_object;
    set_read_callback(i, read_callback);
    set_write_callback(i, write_callback);
    copy_close_callback(i, fd);

    current_object->flags |= O_EFUN_SOCKET;

    event_add(lpc_socks[i].ev_read, nullptr);
    event_add(lpc_socks[i].ev_write, nullptr);

    debug(sockets, "socket_accept: accept on socket %d\n", fd);
    debug(sockets, "socket_accept: new socket %d on fd %d\n", i, accept_fd);
  } else {
    close(accept_fd);
  }

  return i;
}

/*
 * Connect an LPC efun socket
 */
int socket_connect(int fd, const char *name, svalue_t *read_callback, svalue_t *write_callback) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].mode == DATAGRAM) {
    return EEMODENOTSUPP;
  }
  switch (lpc_socks[fd].state) {
    case STATE_CLOSED:
    case STATE_FLUSHING:
    case STATE_UNBOUND:
    case STATE_BOUND:
      break;
    case STATE_LISTEN:
      return EEISLISTEN;
    case STATE_DATA_XFER:
      return EEISCONN;
  }

  if (!lpcaddr_to_sockaddr(name, reinterpret_cast<sockaddr *>(&lpc_socks[fd].r_addr),
                           &lpc_socks[fd].r_addrlen)) {
    debug(sockets, "socket_connect: bad address: %s.\n", name);
    return EEBADADDR;
  }

  set_read_callback(fd, read_callback);
  set_write_callback(fd, write_callback);

  current_object->flags |= O_EFUN_SOCKET;

  if (connect(lpc_socks[fd].fd, reinterpret_cast<struct sockaddr *>(&lpc_socks[fd].r_addr),
              lpc_socks[fd].r_addrlen) == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    switch (e) {
      case ERR(EINTR):
        return EEINTR;
      case ERR(EADDRINUSE):
        return EEADDRINUSE;
      case ERR(EALREADY):
        return EEALREADY;
      case ERR(ECONNREFUSED):
        return EECONNREFUSED;
      case ERR(EWOULDBLOCK):
      case ERR(EINPROGRESS):
        break;
      default:
        debug(sockets, "socket_connect: lpc socket %d (real fd %lld) connect error: %s.\n", fd,
              lpc_socks[fd].fd, evutil_socket_error_to_string(e));
        return EECONNECT;
    }
  }

  // fill-in socket information.
  if (lpc_socks[fd].l_addrlen == 0) {
    lpc_socks[fd].l_addrlen = sizeof(lpc_socks[fd].l_addr);
    if (getsockname(lpc_socks[fd].fd, reinterpret_cast<struct sockaddr *>(&lpc_socks[fd].l_addr),
                    &lpc_socks[fd].l_addrlen) == -1) {
      auto e = evutil_socket_geterror(lpc_socks[fd].fd);
      debug(sockets, "socket_connect: getsockname error: %s.\n", evutil_socket_error_to_string(e));
    }
  }

  lpc_socks[fd].state = STATE_DATA_XFER;
  lpc_socks[fd].flags |= S_BLOCKED;

  debug(sockets, "socket_connect: lpc socket %d (real fd %lld) connecting.\n", fd,
        lpc_socks[fd].fd);

  event_add(lpc_socks[fd].ev_read, nullptr);
  event_add(lpc_socks[fd].ev_write, nullptr);

  return EESUCCESS;
}

/*
 * Write a message on an LPC efun socket
 *
 * NOTE: name format is "ip port", whitespace as delimiter.
 */
int socket_write(int fd, svalue_t *message, const char *name) {
  int len, off;
  char *buf, *p;

  struct sockaddr_storage addr;
  socklen_t addrlen;

  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].mode == DATAGRAM) {
    if (name == nullptr) {
      return EENOADDR;
    }
    if (!lpcaddr_to_sockaddr(name, reinterpret_cast<sockaddr *>(&addr), &addrlen)) {
      return EEBADADDR;
    }
  } else {
    if (lpc_socks[fd].state != STATE_DATA_XFER) {
      return EENOTCONN;
    }
    if (name != nullptr) {
      return EEBADADDR;
    }
    if (lpc_socks[fd].flags & S_BLOCKED) {
      return EEALREADY;
    }
  }

  switch (lpc_socks[fd].mode) {
    case MUD:
      debug(sockets, "socket_write: sending tcp message to %s\n",
            sockaddr_to_string((struct sockaddr *)&lpc_socks[fd].r_addr, lpc_socks[fd].r_addrlen));
      switch (message->type) {
        case T_OBJECT:
          return EETYPENOTSUPP;

        default:
          save_svalue_depth = 0;
          len = svalue_save_size(message);
          if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
            return EEBADDATA;
          }
          buf = reinterpret_cast<char *>(DMALLOC(len + 5, TAG_TEMPORARY, "socket_write: default"));
          if (buf == nullptr) {
            fatal("Out of memory");
          }
          *reinterpret_cast<uint32_t *>(buf) = htonl(static_cast<uint32_t>(len));
          len += 4;
          buf[4] = '\0';
          p = buf + 4;
          save_svalue(message, &p);
          break;
      }
      break;

    case STREAM:
      debug(sockets, "socket_write: sending tcp message to %s\n",
            sockaddr_to_string((struct sockaddr *)&lpc_socks[fd].r_addr, lpc_socks[fd].r_addrlen));
      switch (message->type) {
        case T_BUFFER:
          len = message->u.buf->size;
          if (len == 0) {
            debug(sockets, "socket_write: trying to send 0 length buffer, ignored.\n");
            return EESUCCESS;
          }
          buf = reinterpret_cast<char *>(DMALLOC(len, TAG_TEMPORARY, "socket_write: T_BUFFER"));
          if (buf == nullptr) {
            fatal("Out of memory");
          }
          memcpy(buf, message->u.buf->item, len);
          break;
        case T_STRING:
          len = SVALUE_STRLEN(message);
          if (len == 0) {
            debug(sockets, "socket_write: trying to send 0 length string, ignored.\n");
            return EESUCCESS;
          }
          buf = reinterpret_cast<char *>(DMALLOC(len + 1, TAG_TEMPORARY, "socket_write: T_STRING"));
          if (buf == nullptr) {
            fatal("Out of memory");
          }
          strcpy(buf, message->u.string);
          break;
        case T_ARRAY: {
          int i, limit;
          svalue_t *el;

          len = message->u.arr->size * sizeof(int);
          if (len == 0) {
            debug(sockets, "socket_write: trying to send 0 length array, ignored.\n");
            return EESUCCESS;
          }
          buf = reinterpret_cast<char *>(DMALLOC(len + 1, TAG_TEMPORARY, "socket_write: T_ARRAY"));
          if (buf == nullptr) {
            fatal("Out of memory");
          }
          el = message->u.arr->item;
          limit = len / sizeof(int);
          for (i = 0; i < limit; i++) {
            switch (el[i].type) {
              case T_NUMBER:
                memcpy(&buf[i * sizeof(int)], reinterpret_cast<char *>(&el[i].u.number),
                       sizeof(int));
                break;
              case T_REAL:
                memcpy(&buf[i * sizeof(int)], reinterpret_cast<char *>(&el[i].u.real), sizeof(int));
                break;
              default:
                break;
            }
          }
          break;
        }
        default:
          return EETYPENOTSUPP;
      }
      break;

    case DATAGRAM:
      debug(sockets, "socket_write: sending udp message to %s\n",
            sockaddr_to_string((struct sockaddr *)&addr, addrlen));
      switch (message->type) {
        case T_STRING:
          if ((off = sendto(lpc_socks[fd].fd, const_cast<char *>(message->u.string),
                            SVALUE_STRLEN(message), 0, reinterpret_cast<struct sockaddr *>(&addr),
                            addrlen)) == -1) {
            debug(sockets, "socket_write: sendto error: %s.\n",
                  evutil_socket_error_to_string(evutil_socket_geterror(lpc_socks[fd].fd)));
            return EESENDTO;
          }
          break;

        case T_BUFFER:
          if ((off = sendto(lpc_socks[fd].fd, reinterpret_cast<char *>(message->u.buf->item),
                            message->u.buf->size, 0, reinterpret_cast<struct sockaddr *>(&addr),
                            addrlen)) == -1) {
            debug(sockets, "socket_write: sendto error: %s.\n",
                  evutil_socket_error_to_string(evutil_socket_geterror(lpc_socks[fd].fd)));
            return EESENDTO;
          }
          break;

        default:
          return EETYPENOTSUPP;
      }

#ifdef F_NETWORK_STATS
      if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
        inet_out_packets++;
        inet_out_volume += off;
        inet_socket_out_packets++;
        inet_socket_out_volume += off;
      }
#endif
      return EESUCCESS;

    default:
      return EEMODENOTSUPP;
  }

  if (!len) {
    FREE(buf);
    return EESUCCESS;
  }
  debug(sockets, "socket_write: message size %d.\n", len);
  off = send(lpc_socks[fd].fd, buf, len, 0);
  if (off <= 0) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    FREE(buf);
    if (off == -1) {
      switch (e) {
        case ERR(EWOULDBLOCK): {
          debug(sockets, "socket_write: write would block.\n");
          return EEWOULDBLOCK;
        }
        case ERR(EINTR): {
          debug(sockets, "socket_write: write interrupted.\n");
          return EEINTR;
        }
      }
    }
    debug(sockets, "socket_write: lpc socket %d (real fd %lld) send error: %s.\n", fd,
          lpc_socks[fd].fd, evutil_socket_error_to_string(e));
    lpc_socks[fd].flags |= S_LINKDEAD;
    socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
    return EESEND;
  }

#ifdef F_NETWORK_STATS
  if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
    inet_out_packets++;
    inet_out_volume += off;
    inet_socket_out_packets++;
    inet_socket_out_volume += off;
  }
#endif

  if (off < len) {
    lpc_socks[fd].flags |= S_BLOCKED;
    lpc_socks[fd].w_buf = buf;
    lpc_socks[fd].w_off = off;
    lpc_socks[fd].w_len = len - off;
    debug(sockets, "socket_write: wrote %d out of %d bytes, will call back.\n", off, len);
    event_add(lpc_socks[fd].ev_write, nullptr);
    return EECALLBACK;
  }
  FREE(buf);

  return EESUCCESS;
}

#endif /* PACKAGE_SOCKETS */

static void call_callback(int fd, int what, int num_arg) {
  union string_or_func callback;

  switch (what) {
    case S_READ_FP:
      callback = lpc_socks[fd].read_callback;
      break;
    case S_WRITE_FP:
      callback = lpc_socks[fd].write_callback;
      break;
    case S_CLOSE_FP:
      callback = lpc_socks[fd].close_callback;
      break;
  }

  set_eval(max_eval_cost);

  if (lpc_socks[fd].flags & what) {
    safe_call_function_pointer(callback.f, num_arg);
  } else if (callback.s) {
    if (callback.s[0] == APPLY___INIT_SPECIAL_CHAR) {
      error("Illegal function name.\n");
    }
    safe_apply(callback.s, lpc_socks[fd].owner_ob, num_arg, ORIGIN_INTERNAL);
  }
}

/*
 * Handle LPC efun socket read select events
 */
void socket_read_select_handler(int fd) {
  const auto max_byte_transfer = CONFIG_INT(__MAX_BYTE_TRANSFER__);

  int cc = 0;
  char buf[BUF_SIZE];
  svalue_t value;

  struct sockaddr_storage sockaddr;
  socklen_t addrlen;

  debug(sockets, "read_socket_handler: fd %d state %d\n", fd, lpc_socks[fd].state);

  switch (lpc_socks[fd].state) {
    case STATE_CLOSED:
    case STATE_FLUSHING:
      return;

    case STATE_UNBOUND:
      debug_message("socket_read_select_handler: read on unbound socket %i\n", fd);
      break;

    case STATE_BOUND:
      switch (lpc_socks[fd].mode) {
        case MUD:
        case STREAM:
          break;

        case DATAGRAM: {
          char addr[NI_MAXHOST + NI_MAXSERV];
          debug(sockets, ("read_socket_handler: DATA_XFER DATAGRAM\n"));
          addrlen = sizeof(sockaddr);
          cc = recvfrom(lpc_socks[fd].fd, buf, sizeof(buf) - 1, 0,
                        reinterpret_cast<struct sockaddr *>(&sockaddr), &addrlen);
          if (cc <= 0) {
            break;
          }
#ifdef F_NETWORK_STATS
          if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
            inet_in_packets++;
            inet_in_volume += cc;
            inet_socket_in_packets++;
            inet_socket_in_volume++;
          }
#endif
          debug(sockets, "read_socket_handler: read %d bytes\n", cc);
          buf[cc] = '\0';
          // Translate socket address into "address port" format.
          {
            char host[NI_MAXHOST], service[NI_MAXSERV];
            int ret = getnameinfo(reinterpret_cast<struct sockaddr *>(&sockaddr), addrlen, host,
                                  sizeof(host), service, sizeof(service),
                                  NI_NUMERICHOST | NI_NUMERICSERV);
            if (ret) {
              debug(sockets, "socket_read_select_handler: bad addr: %s", evutil_gai_strerror(ret));
              addr[0] = '\0';
            } else {
              sprintf(addr, "%s %s", host, service);
            }
          }
          push_number(fd);
          if (lpc_socks[fd].flags & S_BINARY || !u8_validate(buf)) {
            buffer_t *b;

            b = allocate_buffer(cc);
            if (b) {
              memcpy(b->item, buf, cc);
              push_refed_buffer(b);
            } else {
              push_number(0);
            }
          } else {
            auto res = u8_sanitize(buf);
            copy_and_push_string(res.c_str());
          }
          copy_and_push_string(addr);
          debug(sockets, ("read_socket_handler: apply\n"));
          call_callback(fd, S_READ_FP, 3);
          return;
        }
        case STREAM_BINARY:
        case DATAGRAM_BINARY:;
      }
      break;

    case STATE_LISTEN:
      debug(sockets, ("read_socket_handler: apply read callback\n"));
      lpc_socks[fd].flags |= S_WACCEPT;
      push_number(fd);
      call_callback(fd, S_READ_FP, 1);
      return;

    case STATE_DATA_XFER:
      switch (lpc_socks[fd].mode) {
        case DATAGRAM:
          break;

        case MUD:
          debug(sockets, ("read_socket_handler: DATA_XFER MUD\n"));
          if (lpc_socks[fd].flags & S_HEADER) {
            cc = recv(lpc_socks[fd].fd,
                      reinterpret_cast<char *>(&lpc_socks[fd].r_len) + lpc_socks[fd].r_off,
                      4 - lpc_socks[fd].r_off, 0);
            if (cc <= 0) {
              break;
            }
#ifdef F_NETWORK_STATS
            if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
              inet_in_packets++;
              inet_in_volume += cc;
              inet_socket_in_packets++;
              inet_socket_in_volume += cc;
            }
#endif
            debug(sockets, "read_socket_handler: read %d bytes\n", cc);
            lpc_socks[fd].r_off += cc;
            if (lpc_socks[fd].r_off != 4) {
              return;
            }
            debug(sockets, ("read_socket_handler: read header\n"));
            lpc_socks[fd].flags &= ~S_HEADER;
            lpc_socks[fd].r_off = 0;
            lpc_socks[fd].r_len = ntohl(lpc_socks[fd].r_len);
            if (lpc_socks[fd].r_len <= 0 || lpc_socks[fd].r_len > max_byte_transfer) {
              break;
            }
            lpc_socks[fd].r_buf = reinterpret_cast<char *>(
                DMALLOC(lpc_socks[fd].r_len + 1, TAG_TEMPORARY, "socket_read_select_handler"));
            if (lpc_socks[fd].r_buf == nullptr) {
              fatal("Out of memory");
            }
            debug(sockets, "read_socket_handler: svalue len is %d.\n", lpc_socks[fd].r_len);
          }
          if (lpc_socks[fd].r_off < lpc_socks[fd].r_len) {
            cc = recv(lpc_socks[fd].fd, lpc_socks[fd].r_buf + lpc_socks[fd].r_off,
                      lpc_socks[fd].r_len - lpc_socks[fd].r_off, 0);
            if (cc <= 0) {
              break;
            }
#ifdef F_NETWORK_STATS
            if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
              inet_in_packets++;
              inet_in_volume += cc;
              inet_socket_in_packets++;
              inet_socket_in_volume += cc;
            }
#endif
            debug(sockets, "read_socket_handler: read %d bytes\n", cc);
            lpc_socks[fd].r_off += cc;
            if (lpc_socks[fd].r_off != lpc_socks[fd].r_len) {
              return;
            }
            debug(sockets, ("read_socket_handler: read svalue\n"));
          }
          lpc_socks[fd].r_buf[lpc_socks[fd].r_len] = '\0';
          value = const0;
          push_number(fd);
          if (restore_svalue(lpc_socks[fd].r_buf, &value) == 0) {
            STACK_INC;
            *sp = value;
          } else {
            push_undefined();
          }
          FREE(lpc_socks[fd].r_buf);
          lpc_socks[fd].flags |= S_HEADER;
          lpc_socks[fd].r_buf = nullptr;
          lpc_socks[fd].r_off = 0;
          lpc_socks[fd].r_len = 0;
          debug(sockets, ("read_socket_handler: apply read callback\n"));
          call_callback(fd, S_READ_FP, 2);
          return;

        case STREAM:
          debug(sockets, ("read_socket_handler: DATA_XFER STREAM\n"));
          cc = recv(lpc_socks[fd].fd, buf, sizeof(buf) - 1, 0);
          if (cc <= 0) {
            break;
          }
#ifdef F_NETWORK_STATS
          if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
            inet_in_packets++;
            inet_in_volume += cc;
            inet_socket_in_packets++;
            inet_socket_in_volume += cc;
          }
#endif
          debug(sockets, "read_socket_handler: read %d bytes\n", cc);
          buf[cc] = '\0';
          push_number(fd);
          if (lpc_socks[fd].flags & S_BINARY) {
            buffer_t *b;

            b = allocate_buffer(cc);
            if (b) {
              b->ref--;
              memcpy(b->item, buf, cc);
              push_buffer(b);
            } else {
              push_number(0);
            }
          } else {
            auto res = u8_sanitize(buf);
            copy_and_push_string(res.c_str());
          }
          debug(sockets, ("read_socket_handler: apply read callback\n"));
          call_callback(fd, S_READ_FP, 2);
          return;
        case STREAM_BINARY:
        case DATAGRAM_BINARY:;
      }
      break;
  }
  if (cc == -1) {
    auto e = evutil_socket_geterror(lpc_socks[fd].fd);
    switch (e) {
      case ERR(ECONNREFUSED):
        /* Evidentally, on Linux 1.2.1, ECONNREFUSED gets returned
         * if an ICMP_PORT_UNREACHED error happens internally.  Why
         * they use this error message, I have no idea, but this seems
         * to work.
         */
        if (lpc_socks[fd].state == STATE_BOUND && lpc_socks[fd].mode == DATAGRAM) {
          return;
        }
        break;
      case ERR(EINTR):
      case ERR(EWOULDBLOCK):
        return;
      default:
        break;
    }
  }

  lpc_socks[fd].flags |= S_LINKDEAD;
  socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
}

/*
 * Handle LPC efun socket write select events
 */
void socket_write_select_handler(int fd) {
  int cc;

  debug(sockets, "write_socket_handler: lpc socket %d (real fd %lld) state %d\n", fd,
        lpc_socks[fd].fd, lpc_socks[fd].state);

  /* if the socket isn't blocked, we've got nothing to send */
  /* if the socket is linkdead, don't send -- could block */
  if (!(lpc_socks[fd].flags & S_BLOCKED) || (lpc_socks[fd].flags & S_LINKDEAD)) {
    return;
  }

  if (lpc_socks[fd].flags & S_BLOCKED) {
    int optval = 0;
    ev_socklen_t optlen = sizeof(optval);
    auto e = getsockopt(lpc_socks[fd].fd, SOL_SOCKET, SO_ERROR,
#ifndef _WIN32
                        (void *)&optval,
#else
                        (char *)&optval,
#endif
                        &optlen);
    if (e == -1) {
      debug(sockets, "write_socket_handler: getsockopt failure: %s.\n",
            evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
      return;
    }

    if (optval) {
      debug(sockets, "write_socket_handler: getsockopt : %d (%s).\n", optval,
            evutil_socket_error_to_string(optval));

      if (optval == ERR(EINTR) || optval == ERR(EINPROGRESS) || optval == ERR(EWOULDBLOCK)) {
        // still connecting
        return;
      }
      lpc_socks[fd].flags |= S_LINKDEAD;
      socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
      return;
    }
  }

  if (lpc_socks[fd].w_buf != nullptr) {
    cc = send(lpc_socks[fd].fd, lpc_socks[fd].w_buf + lpc_socks[fd].w_off, lpc_socks[fd].w_len, 0);
    if (cc < 0) {
      auto e = evutil_socket_geterror(lpc_socks[fd].fd);
      if (cc == -1 && (e == ERR(EWOULDBLOCK) || e == EAGAIN /* linux only */ || e == ERR(EINTR))) {
        event_add(lpc_socks[fd].ev_write, nullptr);
        return;
      }
      debug(sockets,
            "write_socket_handler: lpc_socket %d (real fd %lld) write failed: "
            "%s, connection dead.\n",
            fd, lpc_socks[fd].fd, evutil_socket_error_to_string(e));
      lpc_socks[fd].flags |= S_LINKDEAD;
      if (lpc_socks[fd].state == STATE_FLUSHING) {
        lpc_socks[fd].flags &= ~S_BLOCKED;
        socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
        return;
      }
      socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
      return;
    }
#ifdef F_NETWORK_STATS
    if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
      inet_out_packets++;
      inet_out_volume += cc;
      inet_socket_out_packets++;
      inet_socket_out_volume += cc;
    }
#endif
    lpc_socks[fd].w_off += cc;
    lpc_socks[fd].w_len -= cc;
    if (lpc_socks[fd].w_len != 0) {
      return;
    }
    FREE(lpc_socks[fd].w_buf);
    lpc_socks[fd].w_buf = nullptr;
    lpc_socks[fd].w_off = 0;
  }
  lpc_socks[fd].flags &= ~S_BLOCKED;
  if (lpc_socks[fd].state == STATE_FLUSHING) {
    socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
    return;
  }

  debug(sockets, ("write_socket_handler: apply write_callback\n"));

  push_number(fd);
  call_callback(fd, S_WRITE_FP, 1);
}

/*
 * Close an LPC efun socket
 */
int socket_close(int fd, int flags) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED) {
    return EEBADF;
  }
  if (lpc_socks[fd].state == STATE_FLUSHING && !(flags & SC_FINAL_CLOSE)) {
    return EEBADF;
  }
  if (!(flags & SC_FORCE) && lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }

  if (flags & SC_DO_CALLBACK) {
    debug(sockets, ("socket_close: apply close callback\n"));
    push_number(fd);
    call_callback(fd, S_CLOSE_FP, 1);
  }

  set_read_callback(fd, nullptr);
  set_write_callback(fd, nullptr);
  set_close_callback(fd, nullptr);

  /* if we're linkdead, we'll never flush, so don't even try :-) */
  if ((lpc_socks[fd].flags & S_BLOCKED) && !(lpc_socks[fd].flags & S_LINKDEAD)) {
    /* Can't close now; we still have data to write.  Tell the mudlib
     * it is closed, but we really finish up later.
     */
    lpc_socks[fd].state = STATE_FLUSHING;
    return EESUCCESS;
  }

  // cleanup event listeners
  if (lpc_socks[fd].ev_read != nullptr) {
    event_free(lpc_socks[fd].ev_read);
    lpc_socks[fd].ev_read = nullptr;
  }
  if (lpc_socks[fd].ev_write != nullptr) {
    event_free(lpc_socks[fd].ev_write);
    lpc_socks[fd].ev_write = nullptr;
  }
  if (lpc_socks[fd].ev_data != nullptr) {
    delete lpc_socks[fd].ev_data;
    lpc_socks[fd].ev_data = nullptr;
  }
  evutil_closesocket(lpc_socks[fd].fd);
  if (lpc_socks[fd].r_buf != nullptr) {
    FREE(lpc_socks[fd].r_buf);
  }
  if (lpc_socks[fd].w_buf != nullptr) {
    FREE(lpc_socks[fd].w_buf);
  }
  clear_socket(fd, 1);

  debug(sockets, "socket_close: closed lpc fd %d\n", fd);
  return EESUCCESS;
}

#ifdef PACKAGE_SOCKETS

/*
 * Release an LPC efun socket to another object
 */
int socket_release(int fd, object_t *ob, svalue_t *callback) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if (lpc_socks[fd].owner_ob != current_object) {
    return EESECURITY;
  }
  if (lpc_socks[fd].flags & S_RELEASE) {
    return EESOCKRLSD;
  }

  lpc_socks[fd].flags |= S_RELEASE;
  lpc_socks[fd].release_ob = ob;

  push_number(fd);
  push_object(ob);
  set_eval(max_eval_cost);

  if (callback->type == T_FUNCTION) {
    safe_call_function_pointer(callback->u.fp, 2);
  } else {
    safe_apply(callback->u.string, ob, 2, ORIGIN_INTERNAL);
  }

  if ((lpc_socks[fd].flags & S_RELEASE) == 0) {
    return EESUCCESS;
  }

  lpc_socks[fd].flags &= ~S_RELEASE;
  lpc_socks[fd].release_ob = nullptr;

  return EESOCKNOTRLSD;
}

/*
 * Aquire an LPC efun socket from another object
 */
int socket_acquire(int fd, svalue_t *read_callback, svalue_t *write_callback,
                   svalue_t *close_callback) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return EEBADF;
  }
  if ((lpc_socks[fd].flags & S_RELEASE) == 0) {
    return EESOCKNOTRLSD;
  }
  if (lpc_socks[fd].release_ob != current_object) {
    return EESECURITY;
  }

  lpc_socks[fd].flags &= ~S_RELEASE;
  lpc_socks[fd].owner_ob = current_object;
  lpc_socks[fd].release_ob = nullptr;

  set_read_callback(fd, read_callback);
  set_write_callback(fd, write_callback);
  set_close_callback(fd, close_callback);

  return EESUCCESS;
}

/*
 * Return the string representation of a socket error
 */
const char *socket_error(int error) {
  error = -(error + 1);
  if (error < 0 || error >= ERROR_STRINGS - 1) {
    return "socket_error: invalid error number";
  }
  return error_strings[error];
}

/*
 * Return the remote address for an LPC efun socket
 * NOTE: Due to the way package/sockets.c is written, this function can be
 * called for unbounded socket, on this case, this should return "::" or
 * "0.0.0.0".
 */
int get_socket_address(int fd, char *addr, int *port, int local) {
  struct sockaddr *sockaddr;
  socklen_t addrlen;

  addr[0] = '\0';
  *port = 0;

  if (fd < 0 || fd >= lpc_socks.size()) {
    return EEFDRANGE;
  }
  sockaddr = (local ? reinterpret_cast<struct sockaddr *>(&lpc_socks[fd].l_addr)
                    : reinterpret_cast<struct sockaddr *>(&lpc_socks[fd].r_addr));
  addrlen = (local ? lpc_socks[fd].l_addrlen : lpc_socks[fd].r_addrlen);

  char host[NI_MAXHOST], service[NI_MAXSERV];
  int ret = getnameinfo(sockaddr, addrlen, host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV);
  if (ret) {
#ifdef IPV6
    strcpy(addr, "::");
#else
    strcpy(addr, "0.0.0.0");
#endif
  } else {
    strcpy(addr, host);
    *port = atoi(service);
  }
  return EESUCCESS;
}

/*
 * Return the current socket owner
 */
object_t *get_socket_owner(int fd) {
  if (fd < 0 || fd >= lpc_socks.size()) {
    return (object_t *)nullptr;
  }
  if (lpc_socks[fd].state == STATE_CLOSED || lpc_socks[fd].state == STATE_FLUSHING) {
    return (object_t *)nullptr;
  }
  return lpc_socks[fd].owner_ob;
}

#endif /* PACKAGE_SOCKETS */

/*
 * Initialize a T_OBJECT svalue
 */
void assign_socket_owner(svalue_t *sv, object_t *ob) {
  if (ob != nullptr) {
    sv->type = T_OBJECT;
    sv->u.ob = ob;
    add_ref(ob, "assign_socket_owner");
  } else {
    assign_svalue_no_free(sv, &const0u);
  }
}

/*
 * Close any sockets owned by ob
 */
void close_referencing_sockets(object_t *ob) {
  int i;

  for (i = 0; i < lpc_socks.size(); i++) {
    if (lpc_socks[i].owner_ob == ob && lpc_socks[i].state != STATE_CLOSED &&
        lpc_socks[i].state != STATE_FLUSHING) {
      socket_close(i, SC_FORCE);
    }
  }
}

#ifdef PACKAGE_SOCKETS

/*
 * Return the string representation of a sockaddr.
 * NOTE: this special format is exposed to userland and should be preserved.
 */
static char *sockaddr_to_lpcaddr(struct sockaddr *addr, socklen_t len) {
  static char result[NI_MAXHOST + NI_MAXSERV];

  memset(result, 0, sizeof(result));

  if (!len) {
    return result;
  }

  char host[NI_MAXHOST], service[NI_MAXSERV];
  int ret = getnameinfo(addr, len, host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV);

  if (ret) {
    debug(sockets, "sockaddr_to_string: %s.\n", evutil_gai_strerror(ret));
    return result;
  }

  snprintf(result, sizeof(result), "%s.%s",
           ((strcmp(host, "::") == 0 || strcmp(host, "0.0.0.0") == 0) ? "*" : host),
           ((strcmp(service, "0") == 0) ? "*" : service));

  return result;
}

const char *socket_modes[] = {"MUD", "STREAM", "DATAGRAM", "STREAM_BINARY", "DATAGRAM_BINARY"};

const char *socket_states[] = {"CLOSED", "CLOSING", "UNBOUND", "BOUND", "LISTEN", "DATA_XFER"};

/*
 * Return an array containing info for a socket
 */
array_t *socket_status(int which) {
  array_t *ret;

  if (which < 0 || which >= lpc_socks.size()) {
    return nullptr;
  }

  ret = allocate_empty_array(6);

  ret->item[0].type = T_NUMBER;
  ret->item[0].subtype = 0;
  ret->item[0].u.number = lpc_socks[which].fd;

  ret->item[1].type = T_STRING;
  ret->item[1].subtype = STRING_CONSTANT;
  ret->item[1].u.string = socket_states[lpc_socks[which].state];

  ret->item[2].type = T_STRING;
  ret->item[2].subtype = STRING_CONSTANT;
  ret->item[2].u.string = socket_modes[lpc_socks[which].mode];

  ret->item[3].type = T_STRING;
  ret->item[3].subtype = STRING_MALLOC;
  ret->item[3].u.string = string_copy(
      sockaddr_to_lpcaddr((struct sockaddr *)&lpc_socks[which].l_addr, lpc_socks[which].l_addrlen),
      "socket_status");

  ret->item[4].type = T_STRING;
  ret->item[4].subtype = STRING_MALLOC;
  ret->item[4].u.string = string_copy(
      sockaddr_to_lpcaddr((struct sockaddr *)&lpc_socks[which].r_addr, lpc_socks[which].r_addrlen),
      "socket_status");

  if (!(lpc_socks[which].flags & STATE_FLUSHING) && lpc_socks[which].owner_ob &&
      !(lpc_socks[which].owner_ob->flags & O_DESTRUCTED)) {
    ret->item[5].type = T_OBJECT;
    ret->item[5].u.ob = lpc_socks[which].owner_ob;
    add_ref(lpc_socks[which].owner_ob, "socket_status");
  } else {
    ret->item[5] = const0u;
  }

  return ret;
}

int lpc_socks_num() { return lpc_socks.size(); }

lpc_socket_t *lpc_socks_get(int i) {
  DEBUG_CHECK((i < 0 || i > lpc_socks_num()), "Driver BUG: lpc_socks_get");
  return &lpc_socks[i];
}

#ifdef DEBUGMALLOC_EXTENSIONS
// Mark all lpc sockets.
void mark_sockets(void) {
  int i;
  char *s;

  for (i = 0; i < lpc_socks.size(); i++) {
    if (lpc_socks[i].flags & S_READ_FP) {
      lpc_socks[i].read_callback.f->hdr.extra_ref++;
    } else if ((s = lpc_socks[i].read_callback.s)) {
      EXTRA_REF(BLOCK(s))++;
    }
    if (lpc_socks[i].flags & S_WRITE_FP) {
      lpc_socks[i].write_callback.f->hdr.extra_ref++;
    } else if ((s = lpc_socks[i].write_callback.s)) {
      EXTRA_REF(BLOCK(s))++;
    }
    if (lpc_socks[i].flags & S_CLOSE_FP) {
      lpc_socks[i].close_callback.f->hdr.extra_ref++;
    } else if ((s = lpc_socks[i].close_callback.s)) {
      EXTRA_REF(BLOCK(s))++;
    }
  }
}
#endif

void lpc_socks_closeall() {
  for (auto sock : lpc_socks) {
    if (sock.state == STATE_CLOSED) {
      continue;
    }
    if (sock.fd != -1) {
      evutil_closesocket(sock.fd);
    }
  }
}

#endif /* PACKAGE_SOCKETS */

#endif /* SOCKET_EFUNS */
