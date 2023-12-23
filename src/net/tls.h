#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <string>
SSL_CTX* tls_server_init(std::string_view file_cert, std::string_view file_key);
void tls_server_close(SSL_CTX* ssl_ctx);
SSL* tls_get_client_ctx(SSL_CTX* server_ctx);
SSL_CTX* tls_client_init();
int tls_verify_callback(int preverify_ok, X509_STORE_CTX* x509_ctx);
