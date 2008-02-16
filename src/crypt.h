#ifndef CRYPT_H
#define CRYPT_H

/* Number of bytes in salt (raw, fully used, not printable-encoded bytes). 
 * This is configurable, but don't set it too high (up to say 10). But 4
 * is plenty really, 2^32 salts possible (the DES-style crypt used only 4096).
 */
#define MD5_SALTLEN  4

/* Magic string, used in hashing. Configurable. */
#define MD5_MAGIC "xyzz"

/* Maximum length (in bytes) of digested data. 
 * This is /not/ configurable! Don't change it.
 */
#define MD5_MAXLEN  55

/* This salt value is used in generating salts. It must
 * be a valid salt as returned by md5crypt(), or the
 * routine will go into an infinite loop when generating
 * a salt from an unrecognized string. Any value returned
 * from md5crypt("anything", NULL, NULL) is valid here.
 */

#define MD5_VALID_SALT "anhlklck!ggddl`l`lg`bjblodlfcljdcnhffib`c"

typedef unsigned char BytE;

char *custom_crypt(const char *key, const char *salt, unsigned char *rawout);

int MD5Digest ( BytE *, unsigned long buflen, BytE *);
int encode ( unsigned char *, BytE *, int );
void decode ( unsigned char *, BytE *, int );
void getsalt (BytE *, BytE * );
void crunchbuffer (BytE *buf, SIGNED int *, char *, SIGNED int, int );

#endif
