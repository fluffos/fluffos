#include "base/package_api.h"

#include "net/tls.h"

#ifdef F_SYS_NETWORK_PORTS
void f_sys_network_ports() {
  array_t *info;
  int i = 0, p = 0;

  for (i = 0; i < 5; i++) {
    if (external_port[i].port) {
      p ++;
    }
  }

  info = allocate_empty_array(p);
  p = 0;

  for (i = 0; i < 5; i++) {
    if (!external_port[i].port) {
      continue;
    }
    array_t *p = allocate_empty_array(3);

    p->item[0].type = T_STRING;
    p->item[0].subtype = STRING_CONSTANT;
    p->item[0].u.string = port_kind_name(external_port[i].kind);

    p->item[1].type = T_NUMBER;
    p->item[1].subtype = 0;
    p->item[1].u.number = external_port[i].port;

    p->item[2].type = T_NUMBER;
    p->item[2].subtype = 0;
    p->item[2].u.number = !external_port[i].tls_cert.empty() && !external_port[i].tls_key.empty();

    info->item[p].type = T_ARRAY;
    info->item[p].u.arr = pInfo;
    p ++;
  }

  push_refed_array(info);
}
#endif

#ifdef F_SYS_RELOAD_TLS
void f_sys_reload_tls() {
  auto port_index_display = sp->u.number;
  auto port_index = port_index_display - 1;

  DEFER{ pop_stack(); };
  if (port_index < 0 || port_index > sizeof(external_port)) {
    error("Invalid port index: %d\n", port_index_display);
  }
  auto *port = &external_port[port_index];
  if (port->kind == PORT_TYPE_UNDEFINED) {
    error("Invalid port index: %d\n", port_index_display);
  }
  if (port->kind == PORT_TYPE_WEBSOCKET) {
    error("Reloading websocket TLS config is not supported for port %d.\n", port->port);
  } else {
    if (port->ssl == nullptr) {
      error("Port %d is not TLS enabled\n", port_index_display);
    }
    auto *ctx = tls_server_init(port->tls_cert, port->tls_key);
    if (ctx == nullptr) {
      error("Failed to reload TLS context for port %d\n", port->port);
    }
    // no race condition here since connection listener operates on main thread(), as all EFUNs do
    auto *old_ctx = port->ssl;
    tls_server_close(old_ctx);
    port->ssl = ctx;
    debug_message("Reloading TLS config for port %d.\n", port->port);
  }
}
#endif
