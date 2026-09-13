#ifndef NET_WEBSOCKET_H
#define NET_WEBSOCKET_H

#include <event2/util.h>

// Initialize websocket context
struct lws_context* init_websocket_context(event_base* base, struct port_def_t* port);

void close_websocket_context(struct lws_context* context);

// Reload the vhost SSL_CTX from port->tls_cert / port->tls_key on disk.
// Returns 0 on success. 1 = port is not TLS websocket. 2 = cert/key would
// not load (same probe as the telnet path). lws_tls_cert_updated() itself
// always returns 0 and swallows load failures, so the probe is required.
int reload_websocket_tls(struct port_def_t* port);

struct lws* init_user_websocket(struct lws_context*, evutil_socket_t);
void close_user_websocket(struct lws* wsi);

void websocket_send_text(struct lws*, const char*, size_t);

#endif /* NET_WEBSOCKET_H */
