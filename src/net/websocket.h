#ifndef NET_WEBSOCKET_H
#define NET_WEBSOCKET_H

#include <event2/util.h>

// Initialize websocket context
struct lws_context* init_websocket_context(event_base* base, struct port_def_t* port);

void close_websocket_context(struct lws_context* context);

struct lws* init_user_websocket(struct lws_context*, evutil_socket_t);
void close_user_websocket(struct lws* wsi);

void websocket_send_text(struct lws*, const char*, size_t);

#endif /* NET_WEBSOCKET_H */
