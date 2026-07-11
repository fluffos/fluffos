/*
 * transport.h -- the per-connection byte transport interface.
 *
 * Everything above this interface (comm.cc's user/command pipeline,
 * net/telnet.cc's protocol handling) is transport-agnostic: it produces
 * and consumes wire bytes and never touches sockets directly. Each
 * interactive_t owns one Transport that moves those bytes:
 *
 *   - SocketTransport    (net/transport_libevent.cc): libevent
 *     bufferevent, optionally TLS.
 *   - WebsocketTransport (net/transport_libevent.cc): libwebsockets.
 *   - WasmConsoleTransport (wasm/comm_wasm.cc): the JS host callbacks --
 *     the webpage is the peer.
 *
 * Inbound bytes don't come through this interface (they are pushed by
 * each transport's event source into comm_telnet_received() /
 * comm_text_received()); this is the outbound + lifecycle half.
 */

#ifndef NET_TRANSPORT_H
#define NET_TRANSPORT_H

#include <cstddef>

class Transport {
 public:
  virtual ~Transport() = default;

  // Queue server->client wire bytes (already IAC-escaped / negotiated by
  // the protocol layer where applicable).
  virtual void write(const char* data, size_t len) = 0;

  // Push queued output toward the peer. Returns nonzero on progress.
  virtual int flush() = 0;

  // A complete user command is buffered on this connection: arrange for
  // process_user_command() to run from the driver's scheduler.
  virtual void schedule_command() = 0;

  // True when the transport compresses its own byte stream (websocket
  // permessage-deflate, for example). The telnet layer refuses MCCP on
  // such transports -- stacking a second deflate would only re-wrap the
  // stream and hide the protocol framing.
  virtual bool compresses_stream() const { return false; }

  // Release transport resources (called once, from remove_interactive()).
  virtual void close() = 0;
};

#endif /* NET_TRANSPORT_H */
