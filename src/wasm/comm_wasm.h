#ifndef WASM_COMM_WASM_H
#define WASM_COMM_WASM_H

/*
 * The WASM connection lifecycle (see wasm/comm_wasm.cc): the JS host
 * plays the role of the TCP peer. Wire bytes leave through the
 * WasmConsoleTransport (net/transport.h) to Module.fluffos.onOutput;
 * these entry points are how the host creates connections and feeds
 * bytes back in. Exported to JS via wasm/main_wasm.cc.
 */

#include <cstddef>

// Create a new "connection" (virtual console). Runs the full driver logon
// sequence: master::connect(port) followed by logon() on the returned
// object, with telnet negotiation bytes flowing to the JS host. Returns a
// connection id (>= 0), or -1 on failure.
int wasm_console_connect();

// Feed raw client->server bytes (telnet wire data) for a connection.
void wasm_console_receive(int id, const char* data, size_t len);

// Disconnect a connection (as if the socket closed).
void wasm_console_disconnect(int id);

#endif /* WASM_COMM_WASM_H */
