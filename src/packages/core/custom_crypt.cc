/* Password hasher for the LPC crypt() function.
 *
 * The hasher is based on the MD5 message digest algorithm.
 * In this module MD5 is implemented from scratch using RFC 1321.
 * RFC 1321 is ``The MD5 Message-Digest Algorithm'', by R. Rivest.
 *
 * Implemented by Fermat@Equilibria <bjgras@cs.vu.nl> for MudOS.
 *
 * 970125: First version.
 * 970131: Added crunchbuffer() to allow the crypting of passwords
 *         of (practically) arbitrary length -- custom_crypt() should now
 *         never return NULL.
 * 970205: Added fix for little-endian systems (well, fix for 486 actually),
 *         plus `reference results'.
 *
 *
 * Note:
 *
 *  This should work equally on little- and big-endian, 16, 32 and 64 bit
 *  systems alike (well, everywhere really), but I can't test it everywhere.
 *  Please mail me if you can't reproduce any of the following results (and
 *  are prepared to work out the fixes with me):
 *
 *    key:      "thing"
 *    salt:     "anhlklck!"
 *    result:   "anhlklck!ggddl`l`lg`bjblodlfcljdcnhffib`c"
 *
 *    key:      "this is a ridiculously long PW that nobody would really use"
 *    salt:     "saltstring"
 *    result:   "nahhdhfc!dhbeclbjk`llmhifc`jedo`adbdboc`k"
 *
 *    key:      ""
 *    salt:     ""
 *    salt:     ""
 *    result:   "ijegehja!j`kacklajkljde`od`ogdmlnbfl`bjfo"
 *
 *  This doesn't apply, of course, if you change any of the settings
 *  or any other code before trying this.
 *
 *      =Fermat
 *
 */

#include "base/package_api.h"

#include "packages/core/custom_crypt.h"

/* Can we cheat and just use network byte order (and htonl/ntohl) here?
 * -Beek
 */

/* MIRROR: inverts the order of bytes in a 32-bit quantity, if
 * necessary.
 * Define this to be empty on big-endian systems:
 *      #define MIRROR(l)
 * And for little-endian systems something like:
 *      #define MIRROR(l)    ( (l) = ( ((l) << 24) | (((l) & 0x0000ff00) << 8)
| (((l) & 0x00ff0000) >> 8) | ((l) >> 24) ) )
 */
#ifdef WORDS_BIGENDIAN
#define MIRROR(l)
#else
#define MIRROR(l) \
  ((l) = (((l) << 24) | (((l)&0x0000ff00) << 8) | (((l)&0x00ff0000) >> 8) | ((l) >> 24)))
#endif

/* Functions F, G, H and I as mentioned in RFC, section 3.4. */
#define F(X, Y, Z) (((X) & (Y)) | ((~(X)) & (Z)))
#define G(X, Y, Z) (((X) & (Z)) | ((~(Z)) & (Y)))
#define H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define I(X, Y, Z) ((Y) ^ ((X) | (~(Z))))

/* Left rotation. */
#define RLEFT(a, n) (a) = (((a) << (n)) | ((a) >> (32 - (n))))

namespace {
/* Table T constructed from a sine function, mentioned in RFC, section 3.4.
 * Table T[i], 1 <= i <= 64,    = trunc (4294967296 * |sin i|).
 */
uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

/* This function returns success, i.e. 0 on error. */
int MD5Digest(BytE *buf,            /* Buffer to be digested.               */
              unsigned long buflen, /* Length of the buffer in bytes.
                                     */
              BytE *Digest          /* Output area: 16 raw bytes.           */
) {
#define OA 0x67452301 /* Per RFC, section 3.3. */
#define OB 0xefcdab89
#define OC 0x98badcfe
#define OD 0x10325476

  uint32_t a = OA, b = OB, c = OC, d = OD;
  static uint32_t Block[16]; /* One block: 512 bits. */

  if (buflen > MD5_MAXLEN) {
    return 0;
  } /* Too large. */

  /* Create the block we're going to digest: padded. */
  memset(Block, 0, sizeof(Block));
  memcpy(Block, buf, buflen);
  {
    int i;
    for (i = 0; i <= buflen / 4; i++) {
      MIRROR(Block[i]);
    }
  }
  Block[buflen >> 2] |= 0x00000080 << (8 * (buflen % 4));
  Block[14] = buflen << 3; /* Number of bits in original data. */

/* MD5 Transformation. */
#define Tr(a, b, c, d, k, s, i, x)         \
  (a) += x(b, c, d) + Block[k] + T[(i)-1]; \
  RLEFT(a, s);                             \
  (a) += (b);

  /* Round 1 */
  Tr(a, b, c, d, 0, 7, 1, F);
  Tr(d, a, b, c, 1, 12, 2, F);
  Tr(c, d, a, b, 2, 17, 3, F);
  Tr(b, c, d, a, 3, 22, 4, F);
  Tr(a, b, c, d, 4, 7, 5, F);
  Tr(d, a, b, c, 5, 12, 6, F);
  Tr(c, d, a, b, 6, 17, 7, F);
  Tr(b, c, d, a, 7, 22, 8, F);
  Tr(a, b, c, d, 8, 7, 9, F);
  Tr(d, a, b, c, 9, 12, 10, F);
  Tr(c, d, a, b, 10, 17, 11, F);
  Tr(b, c, d, a, 11, 22, 12, F);
  Tr(a, b, c, d, 12, 7, 13, F);
  Tr(d, a, b, c, 13, 12, 14, F);
  Tr(c, d, a, b, 14, 17, 15, F);
  Tr(b, c, d, a, 15, 22, 16, F);

  /* Round 2 */
  Tr(a, b, c, d, 1, 5, 17, G);
  Tr(d, a, b, c, 6, 9, 18, G);
  Tr(c, d, a, b, 11, 14, 19, G);
  Tr(b, c, d, a, 0, 20, 20, G);
  Tr(a, b, c, d, 5, 5, 21, G);
  Tr(d, a, b, c, 10, 9, 22, G);
  Tr(c, d, a, b, 15, 14, 23, G);
  Tr(b, c, d, a, 4, 20, 24, G);
  Tr(a, b, c, d, 9, 5, 25, G);
  Tr(d, a, b, c, 14, 9, 26, G);
  Tr(c, d, a, b, 3, 14, 27, G);
  Tr(b, c, d, a, 8, 20, 28, G);
  Tr(a, b, c, d, 13, 5, 29, G);
  Tr(d, a, b, c, 2, 9, 30, G);
  Tr(c, d, a, b, 7, 14, 31, G);
  Tr(b, c, d, a, 12, 20, 32, G);

  /* Round 3 */
  Tr(a, b, c, d, 5, 4, 33, H);
  Tr(d, a, b, c, 8, 11, 34, H);
  Tr(c, d, a, b, 11, 16, 35, H);
  Tr(b, c, d, a, 14, 23, 36, H);
  Tr(a, b, c, d, 1, 4, 37, H);
  Tr(d, a, b, c, 4, 11, 38, H);
  Tr(c, d, a, b, 7, 16, 39, H);
  Tr(b, c, d, a, 10, 23, 40, H);
  Tr(a, b, c, d, 13, 4, 41, H);
  Tr(d, a, b, c, 0, 11, 42, H);
  Tr(c, d, a, b, 3, 16, 43, H);
  Tr(b, c, d, a, 6, 23, 44, H);
  Tr(a, b, c, d, 9, 4, 45, H);
  Tr(d, a, b, c, 12, 11, 46, H);
  Tr(c, d, a, b, 15, 16, 47, H);
  Tr(b, c, d, a, 2, 23, 48, H);

  /* Round 4 */
  Tr(a, b, c, d, 0, 6, 49, I);
  Tr(d, a, b, c, 7, 10, 50, I);
  Tr(c, d, a, b, 14, 15, 51, I);
  Tr(b, c, d, a, 5, 21, 52, I);
  Tr(a, b, c, d, 12, 6, 53, I);
  Tr(d, a, b, c, 3, 10, 54, I);
  Tr(c, d, a, b, 10, 15, 55, I);
  Tr(b, c, d, a, 1, 21, 56, I);
  Tr(a, b, c, d, 8, 6, 57, I);
  Tr(d, a, b, c, 15, 10, 58, I);
  Tr(c, d, a, b, 6, 15, 59, I);
  Tr(b, c, d, a, 13, 21, 60, I);
  Tr(a, b, c, d, 4, 6, 61, I);
  Tr(d, a, b, c, 11, 10, 62, I);
  Tr(c, d, a, b, 2, 15, 63, I);
  Tr(b, c, d, a, 9, 21, 64, I);

  /* Final adjustment of registers. */
  a += OA;
  b += OB;
  c += OC;
  d += OD;

  /* Store output. */
  MIRROR(a);
  memcpy(Digest, &a, sizeof(a));
  MIRROR(b);
  memcpy(&(Digest[4]), &b, sizeof(b));
  MIRROR(c);
  memcpy(&(Digest[8]), &c, sizeof(c));
  MIRROR(d);
  memcpy(&(Digest[12]), &d, sizeof(d));

  /* Burn; earlier registers are more useful in attacks. */
  a = b = c = d = 195952365;

  /* Ok. */
  return 1;
}

/* Encode in a printable manner the raw input data. Return written bytes.
 * The number of bytes required is always double the number of input
 * bytes.
 *
 * It's pretty simple-minded encoding, but we're not scraping
 * for bytes here. Besides, the only other possibilities are 5 or 6
 * bits encoding per byte (instead of 4, as here), which look really
 * messy, when implemented, compared to this.
 *
 * Hell, perhaps sprintf (printing in hex) should be used..
 */
int encode(unsigned char *whEre, const BytE *data, int inputbytes) {
  int i, w = 0;

/* This number has to leave the 4 low-end bits free. */
#define ENCODER_OFFSET 96

  for (i = 0; i < inputbytes; i++) {
    whEre[i * 2] = ENCODER_OFFSET + (data[i] & 0x0f);
    whEre[1 + (i * 2)] = ENCODER_OFFSET + ((data[i] >> 4) & 0x0f);
    w += 2;
  }

  return w;
}

/* Gets raw data from printable string; opposite of encode().  */
void decode(BytE *whEre, const BytE *string, int stringbytes) {
  int i;

  for (i = 0; i < stringbytes; i += 2) {
    whEre[i / 2] = (string[i] & 0x0f) | ((string[i + 1] & 0x0f) << 4);
  }
}

/* If there is a valid salt in the input, copy it. Otherwise,
 * generate a new one.
 */
void getsalt(BytE *to, BytE *from) {
  int i;

/* This character seperates the salt encoding from the password encoding
 * in the string returned by custom_crypt(). Configurable.
 */
#define MAGIC_SALTSEP '!'

  if (from) {
    BytE digest[16];

    if (strlen(reinterpret_cast<char *>(from)) > MD5_SALTLEN * 2) {
      if (from[2 * MD5_SALTLEN] == MAGIC_SALTSEP) {
        /* It is possible, by a big fluke,
         * that this string is not generated by custom_crypt()
         * and encode(), and the MAGIC_SALTSEP
         * is there by coincedence. That doesn't really
         * matter, as long as we get consistently
         * the same salt out of here, always. decode()
         * takes care of this.
         */
        decode(to, from, MD5_SALTLEN * 2);
        return;
      }
    }

    /* We have a salt value, but it's not generated by
     * this function. Well, suit yourself, we'll /get/ a salt
     * out of there.. However, we can't do any straightforward
     * copying, because the `salt' is likely to be (also) the
     * plain text, to be mashed up. So to generate a salt from
     * any string, we digest it first, to avoid people being able
     * to pry information from the salt.
     *
     * It is vital, for fairly obvious reasons, that MD5_VALID_SALT
     * is indeed a valid, immediately accepted salt value (as above),
     * otherwise we'll end up right here again.
     */
    custom_crypt(reinterpret_cast<char *>(from), MD5_VALID_SALT, digest);
    memset(to, strlen(reinterpret_cast<char *>(from)), MD5_SALTLEN);
    for (i = 0; i < sizeof(digest); i++) {
      to[i % MD5_SALTLEN] += digest[i];
    }

    return;
  }

  /* We have to generate a random salt. */
  for (i = 0; i < MD5_SALTLEN; i++) {
    to[i] = random_number(256); /* port.c */
  }
}

void crunchbuffer(BytE *buf,      /* Buffer to be crunched.       */
                  int *len,       /* Length now used in buf.      */
                  char *addition, /* What to add to buf.          */
                  int addlen,     /* Length of addition.          */
                  int maxlen      /* How many bytes in buf.       */
) {
  int used;

  used = *len;

  while (addlen > 0) {
    BytE digest[16];
    int crunched;

    /* Reduce `buf' by digesting it. */
    if (used > sizeof(digest)) {
      MD5Digest(buf, used, digest);
      memcpy(buf, digest, sizeof(digest));
      used = sizeof(digest);
    }

    /* Work out how many bytes we can add to `buf', and do it. */
    crunched = std::min((maxlen - used), addlen);
    memcpy(&(buf[used]), addition, crunched);

    /* Update counters and pointers. */
    used += crunched;
    addition += crunched;
    addlen -= crunched;
  }

  *len = used;
}
}  // namespace

/* Return hash of buffer `key' using salt `salt', which
 * must both be null-terminated strings if given.
 *      -  `key' must be given. This is the basic string to be hashed.
 *      -  `salt' is optional. It can be NULL (in which case a random
 *         salt will be used), it can be output from a previous
 *         custom_crypt() call (in which case the salt used in that call
 *         will be used), or it can be any other string, in which case
 *         a salt derived from that string will be used.
 *      -  `rawout' is optional. If non-NULL, the raw digest value
 *         (without any salt) is written to that buffer (16 bytes).
 *
 * At this point, custom_crypt() should never return NULL.
 *
 */
char *custom_crypt(const char *key, const char *salt, unsigned char *rawout) {
  BytE digest[16];
  static BytE buffer[MD5_MAXLEN], abuffer[MD5_MAXLEN], thesalt[MD5_SALTLEN];
  int used = 0, len, i;
  static BytE /* encode()d salt, encode()d digest, salt seperator
               * and null terminating byte:
               */
      ret[(MD5_SALTLEN * 2) + 1 + (sizeof(digest) * 2) + 1];

  /* Obtain the salt we have to use (either given in salt
   * arg or randomly generated one).
   */
  getsalt(thesalt, (BytE *)salt);

#define ADDBUFFER(b, l)                                            \
  if (used + (l) > sizeof(buffer))                                 \
    crunchbuffer(buffer, &used, (char *)(b), (l), sizeof(buffer)); \
  else {                                                           \
    memcpy(&(buffer[used]), (b), (l));                             \
    used += (l);                                                   \
  }

  memset(buffer, 0, sizeof(buffer));

  /* It's important the 0 byte is copied too. */
  len = strlen(key) + 1;
  ADDBUFFER(key, len);
  len = strlen(MD5_MAGIC) + 1;
  ADDBUFFER(MD5_MAGIC, len);

  ADDBUFFER(thesalt, sizeof(thesalt));

  memcpy(abuffer, buffer, sizeof(abuffer));

  /* The iteration count should be high to thwart
   * brute-force guessers. The choice of 3000 is
   * fairly arbitrary, but you shouldn't set it much
   * lower; but it's configurable.
   *
   * Here a `digest' value is generated to be included
   * in the final to-be-digested buffer, along with the
   * actual data.
   *
   * Make sure to make it length-dependant.
   */
  len = strlen(key);
  for (i = 3000 + (11 * len); i > 0; i--) {
    if (!MD5Digest(abuffer, sizeof(abuffer), digest)) {
      return nullptr;
    }
    memcpy(&(abuffer[(i + len) % (MD5_MAXLEN - sizeof(digest))]), digest, sizeof(digest));
  }

  ADDBUFFER(digest, sizeof(digest));

  /* Use this generated buffer to do the actual digesting. */
  if (!MD5Digest(buffer, sizeof(buffer), digest)) {
    return nullptr;
  }

  /* Pyre! */
  memset(buffer, 0, sizeof(buffer));

  /* Now code the salt and the digest into the return string. */
  len = encode(ret, thesalt, sizeof(thesalt));
  ret[len++] = MAGIC_SALTSEP;
  len += encode(&(ret[len]), digest, sizeof(digest));
  ret[len] = 0;

  /* Give raw output (without salt info) if requested. */
  if (rawout) {
    memcpy(rawout, digest, sizeof(digest));
  }

  return reinterpret_cast<char *>(ret);
}
