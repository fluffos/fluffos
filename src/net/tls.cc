#include "base/std.h"

#include "net/tls.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string>

void tls_library_init() {
  static int called = 0;

  if (!called) {
    called = 1;

    /* Initialize the OpenSSL library */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
#else
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, nullptr);
#endif
  }
}

SSL_CTX* tls_server_init(std::string_view file_cert, std::string_view file_key) {
  tls_library_init();

  /* We MUST have entropy, or else there's no point to crypto. */
  if (!RAND_poll() && !RAND_status()) return nullptr;

  SSL_CTX* server_ctx;

  server_ctx = SSL_CTX_new(SSLv23_server_method());

  if (!SSL_CTX_use_certificate_chain_file(server_ctx, file_cert.data()) ||
      !SSL_CTX_use_PrivateKey_file(server_ctx, file_key.data(), SSL_FILETYPE_PEM)) {
    debug_message("Couldn't read '%s' or '%s' file. Please verify these are valid PEM files.",
                  file_cert.data(), file_key.data());
    return nullptr;
  }
  SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
  SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv3);
  SSL_CTX_set_options(server_ctx, SSL_OP_NO_TLSv1_1);

  return server_ctx;
}

void tls_server_close(SSL_CTX* ssl_ctx) { SSL_CTX_free(ssl_ctx); }

SSL* tls_get_client_ctx(SSL_CTX* server_ctx) {
  auto ctx = SSL_new(server_ctx);
  return ctx;
}

int tls_verify_callback(int preverify_ok, X509_STORE_CTX* x509_ctx) {
  char  buf[256];
  X509* cert;
  int   err, depth;

  cert = X509_STORE_CTX_get_current_cert(x509_ctx);
  err = X509_STORE_CTX_get_error(x509_ctx);
  depth = X509_STORE_CTX_get_error_depth(x509_ctx);

  X509_NAME_oneline(X509_get_subject_name(cert), buf, 256);

  /* If error is not X509_V_OK, print out the error information */
  if (err != X509_V_OK) {
    debug(sockets, "tls_verify_callback: verify error:num=%d:%s:depth=%d:%s\n", err,
           X509_verify_cert_error_string(err), depth, buf);
  }

  return preverify_ok;
}

SSL_CTX* tls_client_init() {
  tls_library_init();

  /* We MUST have entropy, or else there's no point to crypto. */
  if (!RAND_poll() && !RAND_status()) return nullptr;

  SSL_CTX* ctx;

  ctx = SSL_CTX_new(TLS_client_method());

  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);
  SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_1);

  // Load system default CA certificates
  auto ret = SSL_CTX_set_default_verify_paths(ctx);
  if (ret != 1) {
    debug_message("Warning: unable to load system default CA certificates.\n");
  }

  // setup certificate verification
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, tls_verify_callback);

  return ctx;
}
