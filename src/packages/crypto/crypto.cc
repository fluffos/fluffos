/**
 * crypto.c
 *
 * Utilises the OpenSSL crypto library to provide various message digest hashes
 * via a hash() efun.  It works in almost the same manner as the hash function
 * from php, and provides legacy hashes (md4, md5, sha1, ripemd160),
 * SHA-2 family (sha224, sha256, sha384, sha512), and modern hashes 
 * (sha3-224, sha3-256, sha3-384, sha3-512, blake2s256, blake2b512, sm3).
 * You must link against the ssl crypto library (add -lssl -lcrypto to system_libs).
 *
 * Version compatibility:
 * - Legacy algorithms (MD4, MD5, SHA-1, SHA-2, RIPEMD160): OpenSSL 1.0.0+
 * - MD2, MDC2: OpenSSL 1.0.0+ to 2.x (deprecated/removed in OpenSSL 3.0+)
 * - BLAKE2 family: OpenSSL 1.1.0+
 * - SHA-3 family and SM3: OpenSSL 1.1.1+
 *
 * -- coded by Ajandurah@Demonslair (Mark Lyndoe) 10/03/09
 * -- updated to support modern hash algorithms (Matthew Lange) 26/07/25
 */

#include "base/package_api.h"

#include <openssl/opensslconf.h>
#include <openssl/evp.h>

#ifndef OPENSSL_NO_SHA
#include <openssl/sha.h>
#endif
#ifndef OPENSSL_NO_MD5
#include <openssl/md5.h>
#endif
#ifndef OPENSSL_NO_MD2
#include <openssl/md2.h>
#endif
#ifndef OPENSSL_NO_MD4
#include <openssl/md4.h>
#endif
#ifndef OPENSSL_NO_MDC2
#include <openssl/mdc2.h>
#endif
#ifndef OPENSSL_NO_RIPEMD160
#include <openssl/ripemd.h>
#endif

#ifdef F_HASH
static char *hexdump(const unsigned char *data, int len) {
  const char hexchars[] = "0123456789abcdef";
  char *result, *p;

  p = result = new_string(len * 2, "f_hash");

  while (len--) {
    *p++ = hexchars[(*data) >> 4];
    *p++ = hexchars[(*data++) & 0xf];
  }

  *p = '\0';

  return result;
}

/* Modern EVP-based hash function for newer algorithms (Copilot assisted)
 * 
 * Uses OpenSSL's EVP interface to support modern hash algorithms like SHA-3,
 * BLAKE2, and other algorithms not available through direct function calls.
 * The EVP interface is the recommended approach for new algorithms in OpenSSL.
 * 
 * This function provides graceful fallback behavior for older OpenSSL versions:
 * - Returns nullptr if the algorithm is not supported in the current OpenSSL build
 * - The calling code will then show an appropriate error message to the user
 */
static char *evp_hash(const char *algo, const unsigned char *data, int data_len) {
  const EVP_MD *md;
  EVP_MD_CTX *ctx;
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len;
  char *result = nullptr;

  // Get the message digest algorithm by name (e.g., "sha3-256")
  md = EVP_get_digestbyname(algo);
  if (!md) {
    // Algorithm not found or not supported in this OpenSSL build
    return nullptr;
  }

  // Create a new digest context for this operation
  ctx = EVP_MD_CTX_new();
  if (!ctx) {
    // Memory allocation failed
    return nullptr;
  }

  // Initialize the digest context with the chosen algorithm
  if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
    EVP_MD_CTX_free(ctx);
    return nullptr;
  }

  // Process the input data through the hash function
  if (EVP_DigestUpdate(ctx, data, data_len) != 1) {
    EVP_MD_CTX_free(ctx);
    return nullptr;
  }

  // Finalize the hash computation and get the result
  if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
    EVP_MD_CTX_free(ctx);
    return nullptr;
  }

  // Clean up the digest context
  EVP_MD_CTX_free(ctx);
  
  // Convert binary hash to hexadecimal string representation
  result = hexdump(hash, hash_len);
  return result;
}

void f_hash() {
  const char *algo, *data;
  char *result = nullptr;
  int data_len;

  algo = (sp - 1)->u.string;
  data = sp->u.string;
  data_len = SVALUE_STRLEN(sp);

#define DO_HASH_IF(id, func, hash_size)        \
  SAFE(if (strcasecmp(algo, id) == 0) {        \
    unsigned char md[hash_size];               \
    func((unsigned char *)data, data_len, md); \
    result = hexdump(md, hash_size);           \
    goto result;                               \
  })

#define DO_EVP_HASH_IF(id)                     \
  SAFE(if (strcasecmp(algo, id) == 0) {        \
    result = evp_hash(id, (unsigned char *)data, data_len); \
    goto result;                               \
  })

  /* Legacy hash algorithms using direct functions */
#ifndef OPENSSL_NO_SHA1
  DO_HASH_IF("sha1", SHA1, SHA_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_MD5
  DO_HASH_IF("md5", MD5, MD5_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_SHA256
  DO_HASH_IF("sha256", SHA256, SHA256_DIGEST_LENGTH);
  DO_HASH_IF("sha224", SHA224, SHA224_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_SHA512
  DO_HASH_IF("sha512", SHA512, SHA512_DIGEST_LENGTH);
  DO_HASH_IF("sha384", SHA384, SHA384_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_MD2
  DO_HASH_IF("md2", MD2, MD2_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_MD4
  DO_HASH_IF("md4", MD4, MD4_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_MDC2
  DO_HASH_IF("mdc2", MDC2, MDC2_DIGEST_LENGTH);
#endif

#ifndef OPENSSL_NO_RIPEMD160
  DO_HASH_IF("ripemd160", RIPEMD160, RIPEMD160_DIGEST_LENGTH);
#endif

  /* Modern hash algorithms using EVP interface */
  /* These algorithms require OpenSSL 1.1.1+ and will gracefully fail on older versions */
  
  /* SHA-3 family (FIPS 202 standard) - requires OpenSSL 1.1.1+ */
  DO_EVP_HASH_IF("sha3-224");
  DO_EVP_HASH_IF("sha3-256");
  DO_EVP_HASH_IF("sha3-384");
  DO_EVP_HASH_IF("sha3-512");

  /* BLAKE2 family (RFC 7693) - requires OpenSSL 1.1.0+ */
  DO_EVP_HASH_IF("blake2b512");
  DO_EVP_HASH_IF("blake2s256");

  /* Other modern algorithms - SM3 requires OpenSSL 1.1.1+ */
  DO_EVP_HASH_IF("sm3");

result:
  if (!result) {
    error("hash() unknown hash type: %s\n", algo);
  }

  /* Pop the arguments off the stack and push the result */
  pop_n_elems(st_num_arg);
  push_malloced_string(result);
}
#endif
