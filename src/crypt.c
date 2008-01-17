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

#include "std.h"
#include "port.h"
#include "crypt.h"

#ifdef CUSTOM_CRYPT

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

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
#ifdef BIGENDIAN
#define MIRROR(l)
#else
#define MIRROR(l)       ( (l) = ( ((l) << 24) | (((l) & 0x0000ff00) << 8) | (((l) & 0x00ff0000) >> 8) | ((l) >> 24) ) )
#endif


/* Functions F, G, H and I as mentioned in RFC, section 3.4. */
#define F(X, Y, Z)      (((X)&(Y))|((~(X))&(Z)))
#define G(X, Y, Z)      (((X)&(Z))|((~(Z))&(Y)))
#define H(X, Y, Z)      ((X)^(Y)^(Z))
#define I(X, Y, Z)      ((Y)^((X)|(~(Z))))

/* Left rotation. */
#define RLEFT(a, n) (a) = (((a) << (n)) | ((a) >> (32-(n))))

/* Table T constructed from a sine function, mentioned in RFC, section 3.4.
 * Table T[i], 1 <= i <= 64,    = trunc (4294967296 * |sin i|).
 */
UINT32 T[64] = {
        0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,
        0xa8304613,0xfd469501,0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
        0x6b901122,0xfd987193,0xa679438e,0x49b40821,0xf61e2562,0xc040b340,
        0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
        0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,
        0x676f02d9,0x8d2a4c8a,0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
        0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,0x289b7ec6,0xeaa127fa,
        0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
        0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,
        0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
        0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

/* This function returns success, i.e. 0 on error. */
int MD5Digest(  BytE *buf, /* Buffer to be digested.               */
                unsigned long  buflen,    /* Length of the buffer in bytes.
  */
                BytE *Digest     /* Output area: 16 raw bytes.           */
             )
{

#define OA 0x67452301   /* Per RFC, section 3.3. */
#define OB 0xefcdab89
#define OC 0x98badcfe
#define OD 0x10325476

        UINT32 A = OA, B = OB, C = OC, D = OD;
        static UINT32 Block[16];  /* One block: 512 bits. */

        if(buflen > MD5_MAXLEN) return 0;       /* Too large. */

        /* Create the block we're going to digest: padded. */
        memset(Block, 0, sizeof(Block));
        memcpy(Block, buf, buflen);
        { int i; for(i = 0; i <= buflen/4; i++) { MIRROR(Block[i]); } }
        Block[buflen>>2] |= 0x00000080 << (8 * (buflen % 4));
        Block[14] = buflen << 3;        /* Number of bits in original data. */

        /* MD5 Transformation. */
#define Tr(a, b, c, d, k, s, i, x)   (a) += x(b,c,d) + Block[k] + T[(i)-1]; RLEFT(a, s); (a) += (b);

        /* Round 1 */
Tr(A,B,C,D, 0, 7, 1,F); Tr(D,A,B,C, 1,12, 2,F); Tr(C,D,A,B, 2,17, 3,F); Tr(B,C,D,A, 3,22, 4,F);
Tr(A,B,C,D, 4, 7, 5,F); Tr(D,A,B,C, 5,12, 6,F); Tr(C,D,A,B, 6,17, 7,F); Tr(B,C,D,A, 7,22, 8,F);
Tr(A,B,C,D, 8, 7, 9,F); Tr(D,A,B,C, 9,12,10,F); Tr(C,D,A,B,10,17,11,F); Tr(B,C,D,A,11,22,12,F);
Tr(A,B,C,D,12, 7,13,F); Tr(D,A,B,C,13,12,14,F); Tr(C,D,A,B,14,17,15,F); Tr(B,C,D,A,15,22,16,F);

        /* Round 2 */
Tr(A,B,C,D, 1, 5,17,G); Tr(D,A,B,C, 6, 9,18,G); Tr(C,D,A,B,11,14,19,G); Tr(B,C,D,A, 0,20,20,G);
Tr(A,B,C,D, 5, 5,21,G); Tr(D,A,B,C,10, 9,22,G); Tr(C,D,A,B,15,14,23,G); Tr(B,C,D,A, 4,20,24,G);
Tr(A,B,C,D, 9, 5,25,G); Tr(D,A,B,C,14, 9,26,G); Tr(C,D,A,B, 3,14,27,G); Tr(B,C,D,A, 8,20,28,G);
Tr(A,B,C,D,13, 5,29,G); Tr(D,A,B,C, 2, 9,30,G); Tr(C,D,A,B, 7,14,31,G); Tr(B,C,D,A,12,20,32,G);

        /* Round 3 */
Tr(A,B,C,D, 5, 4,33,H); Tr(D,A,B,C, 8,11,34,H); Tr(C,D,A,B,11,16,35,H); Tr(B,C,D,A,14,23,36,H);
Tr(A,B,C,D, 1, 4,37,H); Tr(D,A,B,C, 4,11,38,H); Tr(C,D,A,B, 7,16,39,H); Tr(B,C,D,A,10,23,40,H);
Tr(A,B,C,D,13, 4,41,H); Tr(D,A,B,C, 0,11,42,H); Tr(C,D,A,B, 3,16,43,H); Tr(B,C,D,A, 6,23,44,H);
Tr(A,B,C,D, 9, 4,45,H); Tr(D,A,B,C,12,11,46,H); Tr(C,D,A,B,15,16,47,H); Tr(B,C,D,A, 2,23,48,H);

        /* Round 4 */
Tr(A,B,C,D, 0, 6,49,I); Tr(D,A,B,C, 7,10,50,I); Tr(C,D,A,B,14,15,51,I); Tr(B,C,D,A, 5,21,52,I);
Tr(A,B,C,D,12, 6,53,I); Tr(D,A,B,C, 3,10,54,I); Tr(C,D,A,B,10,15,55,I); Tr(B,C,D,A, 1,21,56,I);
Tr(A,B,C,D, 8, 6,57,I); Tr(D,A,B,C,15,10,58,I); Tr(C,D,A,B, 6,15,59,I); Tr(B,C,D,A,13,21,60,I);
Tr(A,B,C,D, 4, 6,61,I); Tr(D,A,B,C,11,10,62,I); Tr(C,D,A,B, 2,15,63,I); Tr(B,C,D,A, 9,21,64,I);

        /* Final adjustment of registers. */
        A += OA;    B += OB;    C += OC;    D += OD;

        /* Store output. */
        MIRROR(A);      memcpy(  Digest,      &A, sizeof(A));
        MIRROR(B);      memcpy(&(Digest[ 4]), &B, sizeof(B));
        MIRROR(C);      memcpy(&(Digest[ 8]), &C, sizeof(C));
        MIRROR(D);      memcpy(&(Digest[12]), &D, sizeof(D));

        /* Burn; earlier registers are more useful in attacks. */
        A = B = C = D = 195952365;

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
int encode(unsigned char *whEre, BytE *data, int inputbytes)
{
        int i, w = 0;

/* This number has to leave the 4 low-end bits free. */
#define ENCODER_OFFSET  96

        for(i = 0; i < inputbytes; i++) {
                whEre[i*2]      = ENCODER_OFFSET + ( data[i]       & 0x0f);
                whEre[1+(i*2)]  = ENCODER_OFFSET + ((data[i] >> 4) & 0x0f);
                w += 2;
        }

        return w;
}

/* Gets raw data from printable string; opposite of encode().  */
void decode(BytE *whEre, BytE *string, int stringbytes)
{
        int i;

        for(i = 0; i < stringbytes; i+=2)
            whEre[i/2] = (string[i] & 0x0f) | ((string[i+1] & 0x0f) << 4);
}

/* If there is a valid salt in the input, copy it. Otherwise,
 * generate a new one.
 */
void getsalt(BytE *to, BytE *from)
{
        int i;

/* This character seperates the salt encoding from the password encoding
 * in the string returned by custom_crypt(). Configurable.
 */
#define MAGIC_SALTSEP   '!'

        if(from) {
            BytE Digest[16];

            if(strlen((char *)from) > MD5_SALTLEN * 2) {
                if(from[2 * MD5_SALTLEN] == MAGIC_SALTSEP) {
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
            custom_crypt((char *) from, MD5_VALID_SALT, Digest);
            memset(to, strlen((char *)from), MD5_SALTLEN);
            for(i = 0; i < sizeof(Digest); i++)
                to[i % MD5_SALTLEN] += Digest[i];

            return;
        }

        /* We have to generate a random salt. */
        for(i = 0; i < MD5_SALTLEN; i++)
                to[i] = random_number(256); /* port.c */

        return;
}

void crunchbuffer(BytE *buf,            /* Buffer to be crunched.       */
                  SIGNED int *len,      /* Length now used in buf.      */
                  char *addition,       /* What to add to buf.          */
                  SIGNED int addlen,    /* Length of addition.          */
                  int maxlen            /* How many bytes in buf.       */
                )
{
        int used;
        
        used = *len;
        
        while(addlen > 0) {
                BytE Digest[16];
                int crunched;

                /* Reduce `buf' by digesting it. */
                if(used > sizeof(Digest)) {
                        MD5Digest(buf, used, Digest);
                        memcpy(buf, Digest, sizeof(Digest));
                        used = sizeof(Digest);
                }

                /* Work out how many bytes we can add to `buf', and do it. */
                crunched = min((maxlen - used), addlen);
                memcpy(&(buf[used]), addition, crunched);

                /* Update counters and pointers. */
                used += crunched;     
                addition += crunched; 
                addlen -= crunched;   
        }

        *len = used;

        return;
}

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
char *custom_crypt(char *key, char *salt, unsigned char *rawout)
{
        BytE Digest[16];
        static BytE buffer[MD5_MAXLEN],
                    abuffer[MD5_MAXLEN],
                    thesalt[MD5_SALTLEN];
        SIGNED int used = 0, len, i;
        static BytE /* encode()d salt, encode()d digest, salt seperator
                     * and null terminating byte:
                     */
                    ret[(MD5_SALTLEN*2) + 1 + (sizeof(Digest)*2) + 1];

        /* Obtain the salt we have to use (either given in salt
         * arg or randomly generated one).
         */
        getsalt(thesalt, (BytE *)salt);

#define ADDBUFFER(b, l) if(used + (l) > sizeof(buffer)) \
	                     crunchbuffer(buffer, &used, (char *)(b), (l), sizeof(buffer)); \
                        else { memcpy(&(buffer[used]), (b), (l)); \
			used += (l); }

        memset(buffer, 0, sizeof(buffer));

        /* It's important the 0 byte is copied too. */
        len = strlen(key) + 1;          ADDBUFFER(key, len);
        len = strlen(MD5_MAGIC) + 1;    ADDBUFFER(MD5_MAGIC, len);

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
        for(i = 3000+(11*len); i > 0; i--) {
                if(!MD5Digest(abuffer, sizeof(abuffer), Digest))  return NULL;
                memcpy(&(abuffer[(i + len) % (MD5_MAXLEN - sizeof(Digest))]), Digest, sizeof(Digest));
        }

        ADDBUFFER(Digest, sizeof(Digest));

        /* Use this generated buffer to do the actual digesting. */
        if(!MD5Digest(buffer, sizeof(buffer), Digest))  return NULL;

        /* Pyre! */
        memset(buffer, 0, sizeof(buffer));

        /* Now code the salt and the digest into the return string. */
        len = encode(ret, thesalt, sizeof(thesalt));
        ret[len++] = MAGIC_SALTSEP;
        len += encode(&(ret[len]), Digest, sizeof(Digest));
        ret[len] = 0;

        /* Give raw output (without salt info) if requested. */
        if(rawout)
                memcpy(rawout, Digest, sizeof(Digest));

        return (char *)ret;
}
#endif
