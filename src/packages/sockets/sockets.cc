/*
  efuns_sock.c: this file contains the socket efunctions called from
  inside eval_instruction() in interpret.c.
*/

#include "base/package_api.h"

#include "packages/sockets/socket_efuns.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#define VALID_SOCKET(x) check_valid_socket((x), fd, get_socket_owner(fd), addr, port)

#ifdef F_SOCKET_CREATE
void f_socket_create(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int fd;

  if ((num_arg == 3) && !(f.type(2) & (T_STRING | T_FUNCTION))) {
    bad_arg(3, F_SOCKET_CREATE);
  }
  if (check_valid_socket("create", -1, current_object, "N/A", -1)) {
    if (num_arg == 2) {
      fd = socket_create(static_cast<enum socket_mode>(f.number(0)), f.argp(1), nullptr);
    } else {
      fd = socket_create(static_cast<enum socket_mode>(f.number(0)), f.argp(1), f.argp(2));
    }
    f.ret_front_number(fd);
  } else {
    f.ret_front_number(EESECURITY);
  }
}
#endif

#ifdef F_SOCKET_BIND
void f_socket_bind(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i, fd, port;
  char addr[ADDR_BUF_SIZE];

  if ((num_arg == 3) && (f.type(2) != T_STRING)) {
    bad_arg(3, F_SOCKET_BIND);
  }

  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("bind")) {
    i = socket_bind(fd, f.number(1), (num_arg == 3 ? f.string(2) : nullptr));
    f.ret_front_number(i);
  } else {
    f.ret_front_number(EESECURITY);
  }
}
#endif

#ifdef F_SOCKET_LISTEN
void f_socket_listen(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i, fd, port;
  char addr[ADDR_BUF_SIZE];

  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("listen")) {
    i = socket_listen(fd, f.argp(1));
    f.ret_front_number(i);
  } else {
    f.ret_front_number(EESECURITY);
  }
}
#endif

#ifdef F_SOCKET_ACCEPT
void f_socket_accept(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int port, fd;
  char addr[ADDR_BUF_SIZE];

  if (!(f.type(2) & (T_STRING | T_FUNCTION))) {
    bad_arg(3, F_SOCKET_ACCEPT);
  }
  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  f.ret_front_number(VALID_SOCKET("accept") ? socket_accept(fd, f.argp(1), f.argp(2)) : EESECURITY);
}
#endif

#ifdef F_SOCKET_CONNECT
void f_socket_connect(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i, fd, port;
  char addr[ADDR_BUF_SIZE];

  if (!(f.type(2) & (T_FUNCTION | T_STRING))) {
    bad_arg(3, F_SOCKET_CONNECT);
  }
  if (!(f.type(3) & (T_FUNCTION | T_STRING))) {
    bad_arg(4, F_SOCKET_CONNECT);
  }
  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

#ifdef IPV6
  if (!strcmp(addr, "::") && port == 0) {
#else
  if (!strcmp(addr, "0.0.0.0") && port == 0) {
#endif
    /*
     * socket descriptor is not bound yet
     */
    const char* s;
    int start = 0;

    addr[0] = '\0';
    if ((s = strchr(f.string(1), ' '))) {
      /*
       * use specified address and port
       */
      i = s - f.string(1);
      if (i > ADDR_BUF_SIZE - 1) {
        start = i - ADDR_BUF_SIZE - 1;
        i = ADDR_BUF_SIZE - 1;
      }
      strncat(addr, f.string(1) + start, i);
      port = atoi(s + 1);
    }
#ifdef DEBUG
  } else {
    debug_message("socket_connect: socket already bound to address/port: %s/%d\n", addr, port);
    debug_message("socket_connect: but requested to connect to: %s\n", f.string(1));
#endif
  }

  f.ret_front_number(VALID_SOCKET("connect")
                         ? socket_connect(fd, f.string(1), f.argp(2), f.argp(3))
                         : EESECURITY);
}
#endif

#ifdef F_SOCKET_WRITE
void f_socket_write(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i, fd, port;
  char addr[ADDR_BUF_SIZE];

  if ((num_arg == 3) && (f.type(2) != T_STRING)) {
    bad_arg(3, F_SOCKET_WRITE);
  }
  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("write")) {
    i = socket_write(fd, f.argp(1), (num_arg == 3) ? f.string(2) : (char*)nullptr);
    f.ret_front_number(i);
  } else {
    f.ret_front_number(EESECURITY);
  }
}
#endif

#ifdef F_SOCKET_CLOSE
void f_socket_close(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int fd, port;
  char addr[ADDR_BUF_SIZE];

  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  f.top().u.number = VALID_SOCKET("close") ? socket_close(fd, 0) : EESECURITY;
  f.done();
}
#endif

#ifdef F_SOCKET_RELEASE
void f_socket_release(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int fd, port;
  char addr[ADDR_BUF_SIZE];

  if (!(f.type(2) & (T_STRING | T_FUNCTION))) {
    bad_arg(3, F_SOCKET_RELEASE);
  }
  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  f.ret_front_number(VALID_SOCKET("release") ? socket_release(fd, f.object(1), f.argp(2))
                                            : EESECURITY);
}
#endif

#ifdef F_SOCKET_ACQUIRE
void f_socket_acquire(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int fd, port;
  char addr[ADDR_BUF_SIZE];

  if (!(f.type(2) & (T_FUNCTION | T_STRING))) {
    bad_arg(3, F_SOCKET_ACQUIRE);
  }
  if (!(f.type(3) & (T_FUNCTION | T_STRING))) {
    bad_arg(4, F_SOCKET_ACQUIRE);
  }
  fd = f.number(0);
  get_socket_address(fd, addr, &port, 0);

  f.ret_front_number(VALID_SOCKET("acquire")
                         ? socket_acquire(fd, f.argp(1), f.argp(2), f.argp(3))
                         : EESECURITY);
}
#endif

#ifdef F_SOCKET_ERROR
void f_socket_error(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  f.ret_str_constant(socket_error(f.number(0)));
}
#endif

#ifdef F_SOCKET_ADDRESS
void f_socket_address(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  char* str;
  int local, port;
  char addr[ADDR_BUF_SIZE];
  char buf[2 * ADDR_BUF_SIZE]; /* a bit of overkill to be safe */

  /*
   * Ok, we will add in a cute little check thing here to see if it is
   * an object or not...
   */
  local = f.number(1);
  f.pop_back();
  if (f.type(0) & T_OBJECT) {
    /* This is so we can get the address of interactives as well. */
    if (!f.object(0)->interactive) {
      f.set_top_undefined();
      f.done();
      return;
    }

    char host[NI_MAXHOST], service[NI_MAXSERV];
    int const ret =
        getnameinfo(reinterpret_cast<struct sockaddr*>(&f.object(0)->interactive->addr),
                    f.object(0)->interactive->addrlen, host, sizeof(host), service, sizeof(service),
                    NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret) {
      strcpy(host, "0.0.0.0");
      strcpy(service, "0");
    }
    sprintf(buf, "%s %s", host, service);

    str = string_copy(buf, "f_socket_address");
    f.ret_str_malloced(str);
    return;
  }
  get_socket_address(f.number(0), addr, &port, local);
  sprintf(buf, "%s %d", addr, port);
  str = string_copy(buf, "f_socket_address");
  f.ret_str_malloced(str);
} /* f_socket_address() */
#endif

#ifdef F_SOCKET_STATUS
void f_socket_status(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  array_t* info;
  int i;

  if (num_arg) {
    info = socket_status(f.number(0));

    if (!info) {
      f.set_top_number(0);
      f.done();
    } else {
      f.set_top_array(info);
      f.done();
    }
  } else {
    info = allocate_empty_array(lpc_socks_num());
    for (i = 0; i < lpc_socks_num(); i++) {
      info->item[i].type = T_ARRAY;
      info->item[i].u.arr = socket_status(i);
    }
    f.ret_refed(info);
  }
}

#if defined(F_SOCKET_SET_OPTION) || defined(F_SOCKET_GET_OPTION)
namespace {
// Every other socket efun refuses to touch a socket the calling object does
// not own; the option efuns did not, which let any object read another
// object's SNI hostname / cert paths and -- worse -- turn off
// SO_TLS_VERIFY_PEER on a connection it does not own.
void check_socket_option_access(const char* efun, int lpc_sock) {
  if (lpc_sock < 0 || lpc_sock >= lpc_socks_num()) {
    error("Bad socket descriptor: %d\n", lpc_sock);
  }
  if (get_socket_owner(lpc_sock) != current_object) {
    error("%s: socket %d is not owned by this object\n", efun, lpc_sock);
  }
}
}  // namespace
#endif

#ifdef F_SOCKET_SET_OPTION
void f_socket_set_option(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto lpc_sock = f.number(0);
  auto option = f.number(1);
  auto* arg = f.argp(2);

  check_socket_option_access("socket_set_option", lpc_sock);

  switch (option) {
    case SO_TLS_VERIFY_PEER:
      if (arg->type != T_NUMBER) {
        bad_arg(3, F_SOCKET_SET_OPTION);
      }
      if (arg->u.number != 0 && arg->u.number != 1) {
        error("Bad socket option value: %d, onl 0 or 1 is accepted.\n", arg->u.number);
      }
      assign_svalue_no_free(&lpc_socks_get(lpc_sock)->options[SO_TLS_VERIFY_PEER], arg);
      break;
    case SO_TLS_SNI_HOSTNAME:
      if (arg->type != T_STRING) {
        bad_arg(3, F_SOCKET_SET_OPTION);
      }
      assign_svalue(&lpc_socks_get(lpc_sock)->options[SO_TLS_SNI_HOSTNAME], arg);
      break;
    case SO_TLS_CERT:
      if (arg->type != T_STRING) {
        bad_arg(3, F_SOCKET_SET_OPTION);
      }
      assign_svalue(&lpc_socks_get(lpc_sock)->options[SO_TLS_CERT], arg);
      break;
    case SO_TLS_KEY:
      if (arg->type != T_STRING) {
        bad_arg(3, F_SOCKET_SET_OPTION);
      }
      assign_svalue(&lpc_socks_get(lpc_sock)->options[SO_TLS_KEY], arg);
      break;
    default:
      error("Unknown socket option: %d\n", option);
  }
  f.ret_void();
}
#endif

#ifdef F_SOCKET_GET_OPTION
void f_socket_get_option(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  // socket_get_option(int, int): socket at arg 0, option at arg 1.
  auto lpc_sock = f.number(0);
  auto option = f.number(1);

  check_socket_option_access("socket_get_option", lpc_sock);

  switch (option) {
    case SO_TLS_VERIFY_PEER:
    case SO_TLS_SNI_HOSTNAME:
    case SO_TLS_CERT:
    case SO_TLS_KEY:
      break;
    default:
      error("Unknown socket option: %d\n", option);
  }

  svalue_t const value = lpc_socks_get(lpc_sock)->options[option];
  if (value.type == T_STRING) {
    f.ret_str_copy(value.u.string);
  } else if (value.type == T_NUMBER) {
    f.ret(value.u.number);
  } else {
    f.ret(0);
  }
}
#endif

#endif
