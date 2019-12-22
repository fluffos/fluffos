#ifndef NET_WS_ASCII_H
#define NET_WS_ASCII_H

#include <libwebsockets.h>

/* one of these is created for each client connecting to us */
const int PROTOCOL_WS_ASCII = 1;

struct ws_ascii_session {
  struct lws *wsi;
  struct interactive_t *user;

  struct evbuffer *buffer;
};

int ws_ascii_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in,
                      size_t len);

void ws_ascii_send(struct lws *wsi, const char *data, size_t len);

#endif /* NET_WS_ASCII_H */
