/*
 * tls_stub.cc -- the "no TLS" implementation of net/tls.h.
 *
 * Used by targets without OpenSSL (the WASM build -- the browser
 * terminates TLS long before bytes reach the driver). Everything fails
 * cleanly: callers already handle a null context as "TLS unavailable".
 * The native driver compiles net/tls.cc instead of this file.
 */

#include "base/std.h"

#include "net/tls.h"

SSL_CTX* tls_server_init(std::string_view /*file_cert*/, std::string_view /*file_key*/) {
  debug_message("TLS is not supported on this platform.\n");
  return nullptr;
}

void tls_server_close(SSL_CTX* /*ssl_ctx*/) {}

SSL* tls_get_client_ctx(SSL_CTX* /*server_ctx*/) { return nullptr; }

SSL_CTX* tls_client_init() { return nullptr; }

int tls_verify_callback(int /*preverify_ok*/, X509_STORE_CTX* /*x509_ctx*/) { return 0; }
