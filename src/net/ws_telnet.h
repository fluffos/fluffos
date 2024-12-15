#ifndef NET_WS_TELNET_H
#define NET_WS_TELNET_H

#include <libwebsockets.h>

/* one of these is created for each client connecting to us */
const int PROTOCOL_WS_TELNET = 2;

struct ws_telnet_session {
  struct lws *wsi;
  struct interactive_t *user;

  struct evbuffer *buffer;
};

int ws_telnet_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in,
                      size_t len);

void ws_telnet_send(struct lws *wsi, const char *data, size_t len);

#endif /* NET_WS_TELNET_H */
