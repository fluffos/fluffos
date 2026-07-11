#ifndef NET_TRANSPORT_NATIVE_H
#define NET_TRANSPORT_NATIVE_H

/*
 * Native-transport internals (net/transport_libevent.cc) shared with the
 * websocket accept paths. Deliberately NOT part of comm.h: these symbols
 * only exist on targets with socket transports, and shared code must not
 * grow calls to them.
 */

#include "net/net_compat.h"

struct interactive_t;
struct port_def_t;

// Create an interactive for an accepted native connection: fills the
// address/port fields and attaches the matching Transport implementation.
interactive_t* new_user(port_def_t*, evutil_socket_t, sockaddr*, ev_socklen_t);

#endif /* NET_TRANSPORT_NATIVE_H */
