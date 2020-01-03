/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 *
 *  This is included from core/private.h if LWS_WITH_TLS
 */

#if !defined(__LWS_TLS_PRIVATE_H__)
#define __LWS_TLS_PRIVATE_H__


#if defined(LWS_WITH_TLS)

#if defined(USE_WOLFSSL)
 #if defined(USE_OLD_CYASSL)
  #if defined(_WIN32)
   #include <IDE/WIN/user_settings.h>
   #include <cyassl/ctaocrypt/settings.h>
  #else
   #include <cyassl/options.h>
  #endif
  #include <cyassl/openssl/ssl.h>
  #include <cyassl/error-ssl.h>
 #else
  #if defined(_WIN32)
   #include <IDE/WIN/user_settings.h>
   #include <wolfssl/wolfcrypt/settings.h>
  #else
   #include <wolfssl/options.h>
  #endif
  #include <wolfssl/openssl/ssl.h>
  #include <wolfssl/error-ssl.h>
  #define OPENSSL_NO_TLSEXT
 #endif /* not USE_OLD_CYASSL */
#else /* WOLFSSL */
 #if defined(LWS_WITH_ESP32)
  #define OPENSSL_NO_TLSEXT
  #if !defined(LWS_AMAZON_RTOS)
   /* AMAZON RTOS has its own setting via MTK_MBEDTLS_CONFIG_FILE */
   #undef MBEDTLS_CONFIG_FILE
   #define MBEDTLS_CONFIG_FILE <mbedtls/esp_config.h>
  #endif
  #include <mbedtls/ssl.h>
  #include <mbedtls/aes.h>
  #include <mbedtls/gcm.h>
  #include <mbedtls/x509_crt.h>
  #include "tls/mbedtls/wrapper/include/openssl/ssl.h" /* wrapper !!!! */
 #else /* not esp32 */
  #if defined(LWS_WITH_MBEDTLS)
   #include <mbedtls/ssl.h>
   #include <mbedtls/aes.h>
   #include <mbedtls/gcm.h>
   #include <mbedtls/x509_crt.h>
   #include <mbedtls/x509_csr.h>
   #include <mbedtls/ecp.h>
   #include <mbedtls/ecdsa.h>
   #include "tls/mbedtls/wrapper/include/openssl/ssl.h" /* wrapper !!!! */
  #else
   #include <openssl/ssl.h>
   #include <openssl/evp.h>
   #include <openssl/err.h>
   #include <openssl/md5.h>
   #include <openssl/sha.h>
   #include <openssl/rsa.h>
   #include <openssl/bn.h>
   #include <openssl/aes.h>
   #ifdef LWS_HAVE_OPENSSL_ECDH_H
    #include <openssl/ecdh.h>
   #endif
   #if !defined(LWS_HAVE_EVP_MD_CTX_free)
    #define EVP_MD_CTX_free EVP_MD_CTX_destroy
   #endif
   #include <openssl/x509v3.h>
  #endif /* not mbedtls */
  #if defined(OPENSSL_VERSION_NUMBER)
   #if (OPENSSL_VERSION_NUMBER < 0x0009080afL)
/*
 * later openssl defines this to negate the presence of tlsext... but it was
 * only introduced at 0.9.8j.  Earlier versions don't know it exists so don't
 * define it... making it look like the feature exists...
 */
    #define OPENSSL_NO_TLSEXT
   #endif
  #endif
 #endif /* not ESP32 */
#endif /* not USE_WOLFSSL */

#endif /* LWS_WITH_TLS */

enum lws_tls_extant {
	LWS_TLS_EXTANT_NO,
	LWS_TLS_EXTANT_YES,
	LWS_TLS_EXTANT_ALTERNATIVE
};


#if defined(LWS_WITH_TLS)

typedef SSL lws_tls_conn;
typedef SSL_CTX lws_tls_ctx;
typedef BIO lws_tls_bio;
typedef X509 lws_tls_x509;

#if defined(LWS_WITH_NETWORK)
#include "tls/private-network.h"
#endif

LWS_EXTERN int
lws_context_init_ssl_library(const struct lws_context_creation_info *info);
LWS_EXTERN void
lws_context_deinit_ssl_library(struct lws_context *context);
#define LWS_SSL_ENABLED(vh) (vh && vh->tls.use_ssl)

extern const struct lws_tls_ops tls_ops_openssl, tls_ops_mbedtls;

struct lws_ec_valid_curves {
	int id;
	const char *jwa_name; /* list terminates with NULL jwa_name */
};

LWS_EXTERN enum lws_tls_extant
lws_tls_use_any_upgrade_check_extant(const char *name);
LWS_EXTERN int openssl_websocket_private_data_index;


LWS_EXTERN void
lws_tls_err_describe_clear(void);

LWS_EXTERN int
lws_tls_openssl_cert_info(X509 *x509, enum lws_tls_cert_info type,
			  union lws_tls_cert_info_results *buf, size_t len);
LWS_EXTERN int
lws_tls_check_all_cert_lifetimes(struct lws_context *context);

LWS_EXTERN int
lws_tls_alloc_pem_to_der_file(struct lws_context *context, const char *filename,
			      const char *inbuf, lws_filepos_t inlen,
			      uint8_t **buf, lws_filepos_t *amount);
LWS_EXTERN char *
lws_ssl_get_error_string(int status, int ret, char *buf, size_t len);

int
lws_gencrypto_bits_to_bytes(int bits);

void
lws_gencrypto_destroy_elements(struct lws_gencrypto_keyelem *el, int m);

/* genec */

struct lws_gencrypto_keyelem;
struct lws_ec_curves;

LWS_EXTERN const struct lws_ec_curves lws_ec_curves[4];
const struct lws_ec_curves *
lws_genec_curve(const struct lws_ec_curves *table, const char *name);
LWS_VISIBLE void
lws_genec_destroy_elements(struct lws_gencrypto_keyelem *el);
int
lws_gencrypto_mbedtls_rngf(void *context, unsigned char *buf, size_t len);

int
lws_genec_confirm_curve_allowed_by_tls_id(const char *allowed, int id,
					  struct lws_jwk *jwk);

#endif
#endif
