#ifndef NET_WEBSOCKET_H
#define NET_WEBSOCKET_H

#include <event2/util.h>

// Initialize websocket context
struct lws_context* init_websocket_context(event_base* base, struct port_def_t* port);

void close_websocket_context(struct lws_context* context);

// Reload websocket TLS from port->tls_cert / port->tls_key on disk.
// Creates a new lws vhost so new connections present the on-disk chain;
// existing sessions stay on the vhost they already handshook. Returns 0
// on success. 1 = port is not TLS websocket. 2 = cert/key would not load
// (same probe as the telnet path) or the new vhost could not be created.
int reload_websocket_tls(struct port_def_t* port);

struct lws* init_user_websocket(struct lws_context*, evutil_socket_t);
void close_user_websocket(struct lws* wsi);

void websocket_send_text(struct lws*, const char*, size_t);

#endif /* NET_WEBSOCKET_H */
