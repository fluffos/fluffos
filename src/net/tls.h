#ifndef NET_TLS_H
#define NET_TLS_H

#include "net/net_compat.h"  // for SSL / SSL_CTX declarations

#include <string>

/*
 * TLS support. Implemented over OpenSSL in net/tls.cc for the native
 * driver; targets without TLS (the WASM build) compile net/tls_stub.cc,
 * whose tls_server_init() always fails cleanly.
 */

// Matches OpenSSL's own forward declaration.
struct x509_store_ctx_st;
typedef struct x509_store_ctx_st X509_STORE_CTX;

SSL_CTX* tls_server_init(std::string_view file_cert, std::string_view file_key);
void tls_server_close(SSL_CTX* ssl_ctx);
SSL* tls_get_client_ctx(SSL_CTX* server_ctx);
SSL_CTX* tls_client_init();
int tls_verify_callback(int preverify_ok, X509_STORE_CTX* x509_ctx);

#endif /* NET_TLS_H */
