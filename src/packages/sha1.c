/**
 * sha1.c: this file provides the sha1 hashing algorithm
 * based in part on code in the public domain by Niyaz PK.
 * -- coded by Ajandurah@Demonslair 21/02/08
 */

#include "../std.h"
#include "../lpc_incl.h"

#define rotateleft(x,n)    ((x)<<(n) | ((x)>>(32-(n))))

#ifdef F_SHA1
static inline void sha1_hash_block(
		const unsigned char *block,
		uint32_t *h0,
		uint32_t *h1,
		uint32_t *h2,
		uint32_t *h3,
		uint32_t *h4)
{
	uint32_t a = *h0, b = *h1, c = *h2, d = *h3, e = *h4, tmp;
	uint32_t word[80];
	int i, f, k;

	for (i = 0; i < 16; i++) {
		word[i] =
			block[i * 4 + 0] << 24 |
			block[i * 4 + 1] << 16 |
			block[i * 4 + 2] <<  8 |
			block[i * 4 + 3] <<  0;
	}

	for (i = 16; i < 80; i++) {
		word[i] = rotateleft((word[i - 3] ^ word[i - 8] ^ word[i - 14] ^ word[i - 16]), 1);
	}

	for (i = 0; i < 80; i++) {
		if (i < 20) {
			f = (b & c) | ((~b) & d);
			k = 0x5A827999;
		} else if (i < 40) {
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
		} else if (i < 60) {
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		} else {
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		}

		tmp = (rotateleft(a, 5) + f + e + k + word[i]) & 0xFFFFFFFF;
		e = d;
		d = c;
		c = rotateleft(b, 30);
		b = a;
		a = tmp;
	}

	*h0 += a;
	*h1 += b;
	*h2 += c;
	*h3 += d;
	*h4 += e;
}

void f_sha1(void)
{
	uint32_t h0, h1, h2, h3, h4;
	const unsigned char *str;
	unsigned char final[128] = { 0 };
	int input_length, final_length, i;

#ifndef NO_BUFFER_TYPE
	if (sp->type == T_BUFFER) {
		str = (const unsigned char *)sp->u.buf->item;
		input_length = sp->u.buf->size;
	} else {
#endif
		str = (const unsigned char *)sp->u.string;
		input_length = SVALUE_STRLEN(sp);
#ifndef NO_BUFFER_TYPE
	}
#endif

	i = input_length % 64;

	/* Copy partial block into 'final' */
	memcpy(final, str + input_length - i, i);

	/* Terminate with a '1' bit, then decide whether an additional
	 * block is needed.
	 */
	final[i] = 0x80;
	final_length = (i > 55) ? 128 : 64;

	/* Write the 64-bit bit count in big-endian order */
	final[final_length - 5] = (input_length >> 29) & 0x7;
	final[final_length - 4] = (input_length >> 21) & 0xff;
	final[final_length - 3] = (input_length >> 13) & 0xff;
	final[final_length - 2] = (input_length >>  5) & 0xff;
	final[final_length - 1] = (input_length <<  3) & 0xff;

	/* Trim the extra bytes off of the input length so that
	 * input rounds off to the nearest complete block
	 */
	input_length -= i;

	/* Initialize the hash */
	h0 = 0x67452301;
	h1 = 0xEFCDAB89;
	h2 = 0x98BADCFE;
	h3 = 0x10325476;
	h4 = 0xC3D2E1F0;

	/* Hash all complete input blocks */
	for (i = 0; i < input_length; i += 64) {
		sha1_hash_block(str + i, &h0, &h1, &h2, &h3, &h4);
	}

	/* Hash the 1 or 2 final padded blocks */
	sha1_hash_block(final, &h0, &h1, &h2, &h3, &h4);
	if (final_length == 128) {
		sha1_hash_block(final + 64, &h0, &h1, &h2, &h3, &h4);
	}

	sprintf((char *)final, "%08x%08x%08x%08x%08x", h0, h1, h2, h3, h4);
	pop_stack();
	push_malloced_string(string_copy((char *)final, "f_sha1"));
}
#endif
