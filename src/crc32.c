#include "std.h"
#include "crc32.h"
#define NEED_CRC_TABLE
#include "crctab.h"		/* see this file for more CRC credits and
				 * comments */

/* compute_crc32: compute a cyclic redundancy code for a buffer 'buf' of a
   given length 'len'.  This trivial little routine was written by
   John Garnett.  All of the code in crctab.h (the hard stuff) was written
   by others.  See the comments in the file (crctab.h) for the credits.
*/

UINT32
compute_crc32 (unsigned char * buf, int len)
{
    register UINT32 crc = 0xFFFFFFFFL;
    register int j;

    j = len;
    while (j--) {
	/* the UPDC32 macro uses 1st arg only once */
	crc = UPDC32((unsigned int) *buf++, crc);
    }
    return crc;
}
