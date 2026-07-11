/*
 * net_compat.h -- portable network type declarations.
 *
 * Headers that only carry pointers/typedefs of networking types
 * (interactive.h, comm.h, base/internal/external_port.h) include this
 * instead of <event2/util.h> / <openssl/ssl.h> directly, so they stay
 * compilable on targets without libevent/OpenSSL (the Emscripten/WASM
 * build). On native builds this is a pure pass-through to the real
 * headers.
 */

#ifndef NET_COMPAT_H
#define NET_COMPAT_H

#ifdef __EMSCRIPTEN__

#include <sys/socket.h>  // for sockaddr_storage, socklen_t

// libevent equivalents (pointer/handle declarations only; no libevent
// code is linked into the WASM build).
typedef int evutil_socket_t;
typedef socklen_t ev_socklen_t;

// OpenSSL equivalents. These match OpenSSL's own forward declarations,
// so native and WASM builds agree on the pointer types.
struct ssl_st;
typedef struct ssl_st SSL;
struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

#else

#include <event2/util.h>
#include <openssl/ssl.h>

#endif /* __EMSCRIPTEN__ */

#endif /* NET_COMPAT_H */
