#ifndef NET_WS_DEBUG_H
#define NET_WS_DEBUG_H

#include "base/std.h"
#include <libwebsockets.h>
#include <event2/buffer.h>

const int PROTOCOL_WS_DEBUG = 100;

struct ws_debug_session {
  struct evbuffer* buffer;
};

int ws_debug_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                      size_t len);

#endif /* NET_WS_DEBUG_H */
