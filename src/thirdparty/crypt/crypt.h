#ifndef _CRYPT_H
#define _CRYPT_H

#include "include/crypt.h"

#define weak __attribute__((__weak__))
#define hidden __attribute__((__visibility__("hidden")))
#define weak_alias(old, new) \
  extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))

#ifdef __cplusplus
extern "C" {
#endif

char *__crypt(const char *, const char *);
char *__crypt_r(const char *, const char *, struct crypt_data *);

#ifdef __cplusplus
}
#endif


hidden char *__crypt_des(const char *, const char *, char *);
hidden char *__crypt_md5(const char *, const char *, char *);
hidden char *__crypt_blowfish(const char *, const char *, char *);
hidden char *__crypt_sha256(const char *, const char *, char *);
hidden char *__crypt_sha512(const char *, const char *, char *);

#endif // _CRYPT_H
