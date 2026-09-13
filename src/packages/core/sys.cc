#include "base/package_api.h"

#include "net/tls.h"
#ifdef F_SYS_RELOAD_TLS
#include "net/websocket.h"
#endif

#ifdef F_SYS_NETWORK_PORTS
void f_sys_network_ports() {
  array_t* info;
  int i = 0, p = 0;

  for (i = 0; i < 5; i++) {
    if (external_port[i].port) {
      p++;
    }
  }

  info = allocate_empty_array(p);
  p = 0;

  for (i = 0; i < 5; i++) {
    if (!external_port[i].port) {
      continue;
    }
    array_t* pInfo = allocate_empty_array(4);

    pInfo->item[0].type = T_NUMBER;
    pInfo->item[0].subtype = 0;
    pInfo->item[0].u.number = i + 1;

    pInfo->item[1].type = T_STRING;
    pInfo->item[1].subtype = STRING_CONSTANT;
    pInfo->item[1].u.string = port_kind_name(external_port[i].kind);

    pInfo->item[2].type = T_NUMBER;
    pInfo->item[2].subtype = 0;
    pInfo->item[2].u.number = external_port[i].port;

    pInfo->item[3].type = T_NUMBER;
    pInfo->item[3].subtype = 0;
    pInfo->item[3].u.number =
        !external_port[i].tls_cert.empty() && !external_port[i].tls_key.empty();

    info->item[p].type = T_ARRAY;
    info->item[p].u.arr = pInfo;
    p++;
  }

  push_refed_array(info);
}
#endif

#ifdef F_SYS_RELOAD_TLS
void f_sys_reload_tls() {
  auto port_index_display = sp->u.number;
  auto port_index = port_index_display - 1;

  DEFER { pop_stack(); };
  // external_port is a fixed array; bound by ELEMENT COUNT, not sizeof (which
  // is the size in bytes and let indices up to ~sizeof(port_def_t)*N through,
  // for an out-of-bounds read and a wild tls_server_close()/port->ssl write).
  if (port_index < 0 ||
      port_index >= (LPC_INT)(sizeof(external_port) / sizeof(external_port[0]))) {
    error("Invalid port index: %" LPC_INT_FMTSTR_P "\n", port_index_display);
  }
  auto* port = &external_port[port_index];
  if (port->kind == PORT_TYPE_UNDEFINED) {
    error("Invalid port index: %" LPC_INT_FMTSTR_P "\n", port_index_display);
  }
  if (port->kind == PORT_TYPE_WEBSOCKET) {
    // Same semantics as the telnet path: new connections pick up the
    // cert/key currently on disk; existing sessions keep the handshake
    // they already completed. lws_tls_cert_updated() is not used: it
    // only reloads vhosts whose stored filepath strcmp-equals the
    // caller, and CONTEXT_PORT_NO_LISTEN_SERVER does not keep that
    // match reliably.
    int rc = reload_websocket_tls(port);
    if (rc == 1) {
      error("Port %" LPC_INT_FMTSTR_P " is not TLS enabled\n", port_index_display);
    }
    if (rc == 2) {
      error("Failed to reload TLS context for port %d\n", port->port);
    }
    debug_message("Reloading TLS config for port %d.\n", port->port);
  } else {
    if (port->ssl == nullptr) {
      error("Port %" LPC_INT_FMTSTR_P " is not TLS enabled\n", port_index_display);
    }
    auto* ctx = tls_server_init(port->tls_cert, port->tls_key);
    if (ctx == nullptr) {
      error("Failed to reload TLS context for port %d\n", port->port);
    }
    // no race condition here since connection listener operates on main thread(), as all EFUNs do
    auto* old_ctx = port->ssl;
    tls_server_close(old_ctx);
    port->ssl = ctx;
    debug_message("Reloading TLS config for port %d.\n", port->port);
  }
}
#endif
