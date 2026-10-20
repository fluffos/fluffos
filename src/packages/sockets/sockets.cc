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
vm::Value f_socket_create(vm::EfunArgs&, LPC_INT mode, vm::Arg read_callback,
                          std::optional<vm::Arg> close_callback) {
  if (check_valid_socket("create", -1, current_object, "N/A", -1)) {
    return socket_create(static_cast<enum socket_mode>(mode), read_callback.sv(),
                         close_callback ? close_callback->sv() : nullptr);
  }
  return EESECURITY;
}
#endif

#ifdef F_SOCKET_BIND
vm::Value f_socket_bind(vm::EfunArgs&, LPC_INT fd, LPC_INT bind_port,
                        std::optional<vm::StringArg> bind_addr) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("bind")) {
    return socket_bind(fd, bind_port, bind_addr ? bind_addr->c_str() : nullptr);
  }
  return EESECURITY;
}
#endif

#ifdef F_SOCKET_LISTEN
vm::Value f_socket_listen(vm::EfunArgs&, LPC_INT fd, vm::Arg listen_callback) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("listen")) {
    return socket_listen(fd, listen_callback.sv());
  }
  return EESECURITY;
}
#endif

#ifdef F_SOCKET_ACCEPT
vm::Value f_socket_accept(vm::EfunArgs&, LPC_INT fd, vm::Arg read_callback,
                          vm::Arg write_callback) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  return VALID_SOCKET("accept") ? socket_accept(fd, read_callback.sv(), write_callback.sv())
                                : EESECURITY;
}
#endif

#ifdef F_SOCKET_CONNECT
vm::Value f_socket_connect(vm::EfunArgs&, LPC_INT fd, vm::StringArg address, vm::Arg read_callback,
                           vm::Arg write_callback) {
  int i, port;
  char addr[ADDR_BUF_SIZE];

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
    if ((s = strchr(address.c_str(), ' '))) {
      /*
       * use specified address and port
       */
      i = s - address.c_str();
      if (i > ADDR_BUF_SIZE - 1) {
        start = i - ADDR_BUF_SIZE - 1;
        i = ADDR_BUF_SIZE - 1;
      }
      memcpy(addr, address.c_str() + start, static_cast<size_t>(i));
      addr[i] = '\0';
      port = atoi(s + 1);
    }
#ifdef DEBUG
  } else {
    debug_message("socket_connect: socket already bound to address/port: %s/%d\n", addr, port);
    debug_message("socket_connect: but requested to connect to: %s\n", address.c_str());
#endif
  }

  return VALID_SOCKET("connect")
             ? socket_connect(fd, address.c_str(), read_callback.sv(), write_callback.sv())
             : EESECURITY;
}
#endif

#ifdef F_SOCKET_WRITE
vm::Value f_socket_write(vm::EfunArgs&, LPC_INT fd, vm::Arg message,
                         std::optional<vm::StringArg> address) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  if (VALID_SOCKET("write")) {
    return socket_write(fd, message.sv(), address ? address->c_str() : nullptr);
  }
  return EESECURITY;
}
#endif

#ifdef F_SOCKET_CLOSE
vm::Value f_socket_close(vm::EfunArgs&, LPC_INT fd) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  return VALID_SOCKET("close") ? socket_close(fd, 0) : EESECURITY;
}
#endif

#ifdef F_SOCKET_RELEASE
vm::Value f_socket_release(vm::EfunArgs&, LPC_INT fd, object_t* ob, vm::Arg release_callback) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  /* the release callback runs LPC, which may destruct `ob`: its argument
   * slot may be 0 by the time the window is released */
  return VALID_SOCKET("release") ? socket_release(fd, ob, release_callback.sv()) : EESECURITY;
}
#endif

#ifdef F_SOCKET_ACQUIRE
vm::Value f_socket_acquire(vm::EfunArgs&, LPC_INT fd, vm::Arg read_callback, vm::Arg write_callback,
                           vm::Arg close_callback) {
  int port;
  char addr[ADDR_BUF_SIZE];

  get_socket_address(fd, addr, &port, 0);

  return VALID_SOCKET("acquire") ? socket_acquire(fd, read_callback.sv(), write_callback.sv(),
                                                  close_callback.sv())
                                 : EESECURITY;
}
#endif

#ifdef F_SOCKET_ERROR
vm::Value f_socket_error(vm::EfunArgs&, LPC_INT error_code) {
  return vm::Value::str_constant(socket_error(error_code));
}
#endif

#ifdef F_SOCKET_ADDRESS
vm::Value f_socket_address(vm::EfunArgs&, vm::Arg target, LPC_INT local) {
  char* str;
  int port;
  char addr[ADDR_BUF_SIZE];
  char buf[2 * ADDR_BUF_SIZE]; /* a bit of overkill to be safe */

  /*
   * Ok, we will add in a cute little check thing here to see if it is
   * an object or not...
   */
  if (target.is(T_OBJECT)) {
    object_t* ob = target.object();
    /* This is so we can get the address of interactives as well. */
    if (!ob->interactive) {
      return vm::Value::undefined();
    }

    char host[NI_MAXHOST], service[NI_MAXSERV];
    int const ret = getnameinfo(reinterpret_cast<struct sockaddr*>(&ob->interactive->addr),
                                ob->interactive->addrlen, host, sizeof(host), service,
                                sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret) {
      strcpy(host, "0.0.0.0");
      strcpy(service, "0");
    }
    sprintf(buf, "%s %s", host, service);

    str = string_copy(buf, "f_socket_address");
    return vm::Value::str_malloced(str);
  }
  get_socket_address(target.number(), addr, &port, local);
  sprintf(buf, "%s %d", addr, port);
  str = string_copy(buf, "f_socket_address");
  return vm::Value::str_malloced(str);
} /* f_socket_address() */
#endif

#ifdef F_SOCKET_STATUS
vm::Value f_socket_status(vm::EfunArgs&, std::optional<LPC_INT> fd) {
  array_t* info;
  int i;

  if (fd) {
    info = socket_status(*fd);

    if (!info) {
      return 0;
    }
    return vm::Value::refed(info);
  }
  info = allocate_empty_array(lpc_socks_num());
  for (i = 0; i < lpc_socks_num(); i++) {
    info->item[i].type = T_ARRAY;
    info->item[i].u.arr = socket_status(i);
  }
  return vm::Value::refed(info);
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
void f_socket_set_option(vm::EfunArgs&, LPC_INT lpc_sock, LPC_INT option, vm::Arg value) {
  auto* arg = value.sv();

  check_socket_option_access("socket_set_option", lpc_sock);

  switch (option) {
    case SO_TLS_VERIFY_PEER:
      if (arg->type != T_NUMBER) {
        bad_arg(3, F_SOCKET_SET_OPTION);
      }
      if (arg->u.number != 0 && arg->u.number != 1) {
        error("Bad socket option value: %" LPC_INT_FMTSTR_P ", onl 0 or 1 is accepted.\n",
              arg->u.number);
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
      error("Unknown socket option: %" LPC_INT_FMTSTR_P "\n", option);
  }
}
#endif

#ifdef F_SOCKET_GET_OPTION
vm::Value f_socket_get_option(vm::EfunArgs&, LPC_INT lpc_sock, LPC_INT option) {
  // socket_get_option(int, int): socket at arg 0, option at arg 1.

  check_socket_option_access("socket_get_option", lpc_sock);

  switch (option) {
    case SO_TLS_VERIFY_PEER:
    case SO_TLS_SNI_HOSTNAME:
    case SO_TLS_CERT:
    case SO_TLS_KEY:
      break;
    default:
      error("Unknown socket option: %" LPC_INT_FMTSTR_P "\n", option);
  }

  svalue_t const value = lpc_socks_get(lpc_sock)->options[option];
  if (value.type == T_STRING) {
    return vm::Value::str_copy(value.u.string);
  }
  if (value.type == T_NUMBER) {
    return value.u.number;
  }
  return 0;
}
#endif

#endif
