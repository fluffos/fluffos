#ifndef NET_WS_TELNET_H
#define NET_WS_TELNET_H

#include <libwebsockets.h>

#include "net/ws_common.h"

int ws_telnet_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                       size_t len);

void ws_telnet_send(struct lws* wsi, const char* data, size_t len);

#endif /* NET_WS_TELNET_H */
