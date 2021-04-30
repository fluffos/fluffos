#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <string>
SSL_CTX* tls_server_init(std::string_view file_cert, std::string_view file_key);
SSL* tls_get_client_ctx(SSL_CTX* server_ctx);
