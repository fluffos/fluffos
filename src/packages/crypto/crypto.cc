/**
 * crypto.c
 *
 * Utilises the OpenSSL crypto library to provide various message digest hashes
 * via a hash() efun.  It works in almost the same manner as the hash function
 * from php, and provides md2, md4, md5, mdc2, sha1 and ripemd160 hashes.  You
 * must link against the ssl link library (add -lssl to system_libs).
 *
 * -- coded by Ajandurah@Demonslair (Mark Lyndoe) 10/03/09
 */

#include "base/package_api.h"

#include <openssl/opensslconf.h>

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

void f_hash(void) {
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
  })

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

  if (!result) {
    error("hash() unknown hash type: %s", algo);
  }

  /* Pop the arguments off the stack and push the result */
  pop_n_elems(st_num_arg);
  push_malloced_string(result);
}
#endif
