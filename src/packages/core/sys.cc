#include "base/package_api.h"

#include "net/tls.h"

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
