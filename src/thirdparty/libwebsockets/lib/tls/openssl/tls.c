/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2018 Andy Green <andy@warmcat.com>
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
 */

#include "core/private.h"
#include "tls/openssl/private.h"

extern int openssl_websocket_private_data_index,
openssl_SSL_CTX_private_data_index;

char* lws_ssl_get_error_string(int status, int ret, char *buf, size_t len) {
	switch (status) {
	case SSL_ERROR_NONE:
		return lws_strncpy(buf, "SSL_ERROR_NONE", len);
	case SSL_ERROR_ZERO_RETURN:
		return lws_strncpy(buf, "SSL_ERROR_ZERO_RETURN", len);
	case SSL_ERROR_WANT_READ:
		return lws_strncpy(buf, "SSL_ERROR_WANT_READ", len);
	case SSL_ERROR_WANT_WRITE:
		return lws_strncpy(buf, "SSL_ERROR_WANT_WRITE", len);
	case SSL_ERROR_WANT_CONNECT:
		return lws_strncpy(buf, "SSL_ERROR_WANT_CONNECT", len);
	case SSL_ERROR_WANT_ACCEPT:
		return lws_strncpy(buf, "SSL_ERROR_WANT_ACCEPT", len);
	case SSL_ERROR_WANT_X509_LOOKUP:
		return lws_strncpy(buf, "SSL_ERROR_WANT_X509_LOOKUP", len);
	case SSL_ERROR_SYSCALL:
		switch (ret) {
                case 0:
                        lws_snprintf(buf, len, "SSL_ERROR_SYSCALL: EOF");
                        return buf;
                case -1:
#ifndef LWS_PLAT_OPTEE
			lws_snprintf(buf, len, "SSL_ERROR_SYSCALL: %s",
				     strerror(errno));
#else
			lws_snprintf(buf, len, "SSL_ERROR_SYSCALL: %d", errno);
#endif
			return buf;
                default:
                        return strncpy(buf, "SSL_ERROR_SYSCALL", len);
	}
	case SSL_ERROR_SSL:
		return "SSL_ERROR_SSL";
	default:
		return "SSL_ERROR_UNKNOWN";
	}
}

void
lws_tls_err_describe_clear(void)
{
	char buf[160];
	unsigned long l;

	do {
		l = ERR_get_error();
		if (!l)
			break;

		ERR_error_string_n(l, buf, sizeof(buf));
		lwsl_info("   openssl error: %s\n", buf);
	} while (l);
	lwsl_info("\n");
}

#if LWS_MAX_SMP != 1

static pthread_mutex_t *openssl_mutexes;

static void
lws_openssl_lock_callback(int mode, int type, const char *file, int line)
{
	(void)file;
	(void)line;

	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&openssl_mutexes[type]);
	else
		pthread_mutex_unlock(&openssl_mutexes[type]);
}

static unsigned long
lws_openssl_thread_id(void)
{
	return (unsigned long)pthread_self();
}
#endif


int
lws_context_init_ssl_library(const struct lws_context_creation_info *info)
{
#ifdef USE_WOLFSSL
#ifdef USE_OLD_CYASSL
	lwsl_info(" Compiled with CyaSSL support\n");
#else
	lwsl_info(" Compiled with wolfSSL support\n");
#endif
#else
#if defined(LWS_WITH_BORINGSSL)
	lwsl_info(" Compiled with BoringSSL support\n");
#else
	lwsl_info(" Compiled with OpenSSL support\n");
#endif
#endif
	if (!lws_check_opt(info->options, LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT)) {
		lwsl_info(" SSL disabled: no "
			  "LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT\n");
		return 0;
	}

	/* basic openssl init */

	lwsl_info("Doing SSL library init\n");

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
#else
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
#endif
#if defined(LWS_WITH_NETWORK)
	openssl_websocket_private_data_index =
		SSL_get_ex_new_index(0, "lws", NULL, NULL, NULL);

	openssl_SSL_CTX_private_data_index = SSL_CTX_get_ex_new_index(0,
			NULL, NULL, NULL, NULL);
#endif

#if LWS_MAX_SMP != 1
	{
		int n;

		openssl_mutexes = (pthread_mutex_t *)
				OPENSSL_malloc(CRYPTO_num_locks() *
					       sizeof(openssl_mutexes[0]));

		for (n = 0; n < CRYPTO_num_locks(); n++)
			pthread_mutex_init(&openssl_mutexes[n], NULL);

		/*
		 * These "functions" disappeared in later OpenSSL which is
		 * already threadsafe.
		 */

		(void)lws_openssl_thread_id;
		(void)lws_openssl_lock_callback;

		CRYPTO_set_id_callback(lws_openssl_thread_id);
		CRYPTO_set_locking_callback(lws_openssl_lock_callback);
	}
#endif

	return 0;
}

void
lws_context_deinit_ssl_library(struct lws_context *context)
{
#if LWS_MAX_SMP != 1
	int n;

	if (!lws_check_opt(context->options,
			   LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT))
		return;

	CRYPTO_set_locking_callback(NULL);

	for (n = 0; n < CRYPTO_num_locks(); n++)
		pthread_mutex_destroy(&openssl_mutexes[n]);

	OPENSSL_free(openssl_mutexes);
#endif
}
