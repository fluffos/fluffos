#ifndef NET_WS_COMMON_H
#define NET_WS_COMMON_H

#include <cstddef>

#include <libwebsockets.h>

struct interactive_t;

/* one protocol id per client-facing websocket sub-protocol */
const int PROTOCOL_WS_ASCII = 1;
const int PROTOCOL_WS_TELNET = 2;

// Per-connection session state. The ascii and telnet protocol handlers share
// an identical layout, so they share one session struct.
struct ws_session {
  struct lws* wsi;
  struct interactive_t* user;

  struct evbuffer* buffer;

  // Set by close_user_websocket() for a driver-initiated close: the session
  // is done reading (RECEIVE discards input instead of hard-closing, which
  // would truncate the flush this exists to protect) and ws_handle_writeable()
  // closes with status 1000 once `buffer` is fully drained -- see
  // src/www/AGENTS.md's "Driver-initiated close is close-AFTER-FLUSH" note.
  bool close_after_flush;
};

// Decides how many leading bytes of an outgoing window may be sent without
// splitting a protocol unit across websocket messages (a UTF-8 codepoint for
// ascii, a telnet IAC sequence for telnet). `data`/`len` is the copied-out
// (not yet drained) window and `window_full` is true when it filled the whole
// scratch buffer. Returning 0 means "hold everything back until more data
// arrives". A null callback sends the window verbatim.
using ws_truncate_fn = size_t (*)(ws_session* pss, unsigned char* data, size_t len,
                                  bool window_full);

// LWS_CALLBACK_PROTOCOL_INIT: allocate/zero the per-vhost data lws hangs off
// the vhost.
void ws_handle_protocol_init(struct lws* wsi);

// LWS_CALLBACK_ESTABLISHED: resolve the peer address (honouring X-REAL-IP),
// create the interactive user, wire up the session, and schedule the logon.
// `transport_setup`, if non-null, runs on the fresh user before logon (used by
// the telnet handler to bring up its telnet layer). Returns the value the lws
// callback should return (0 to accept, -1 to drop the connection).
int ws_handle_established(struct lws* wsi, ws_session* pss,
                          void (*transport_setup)(struct interactive_t*));

// LWS_CALLBACK_CLOSED: tear down the interactive user and free session
// resources.
void ws_handle_closed(ws_session* pss);

// LWS_CALLBACK_SERVER_WRITEABLE: drain queued bytes to the socket, holding back
// partial protocol units via `truncate`. Returns the value the lws callback
// should return (0 on success, -1 on a write error).
int ws_handle_writeable(struct lws* wsi, ws_session* pss, ws_truncate_fn truncate);

// Queue bytes and request a writeable callback; the actual send happens in
// LWS_CALLBACK_SERVER_WRITEABLE.
void ws_queue_send(struct lws* wsi, int expected_protocol, const char* data, size_t len);

#endif /* NET_WS_COMMON_H */
