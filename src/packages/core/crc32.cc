#include "base/package_api.h"

#include "packages/core/crc32.h"

#define NEED_CRC_TABLE
#include "packages/core/crctab.h" /* see this file for more CRC credits and \
* comments */

/* compute_crc32: compute a cyclic redundancy code for a buffer 'buf' of a
   given length 'len'.  This trivial little routine was written by
   John Garnett.  All of the code in crctab.h (the hard stuff) was written
   by others.  See the comments in the file (crctab.h) for the credits.
*/

uint32_t compute_crc32(unsigned char *buf, int len) {
  uint32_t crc = 0xFFFFFFFFL;
  int j;

  j = len;
  while (j--) {
    /* the UPDC32 macro uses 1st arg only once */
    crc = UPDC32((unsigned int)*buf++, crc);
  }
  return crc;
}

#ifdef F_CRC32
void f_crc32(void) {
  int len;
  unsigned char *buf;
  uint32_t crc;

  if (sp->type == T_STRING) {
    len = SVALUE_STRLEN(sp);
    buf = (unsigned char *)sp->u.string;
  } else if (sp->type == T_BUFFER) {
    len = sp->u.buf->size;
    buf = sp->u.buf->item;
  } else {
    bad_argument(sp, T_STRING | T_BUFFER, 1, F_CRC32);
  }
  crc = compute_crc32(buf, len);
  free_svalue(sp, "f_crc32");
  put_number(crc);
}
#endif
