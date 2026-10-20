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

uint32_t compute_crc32(unsigned char* buf, int len) {
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
vm::Value f_crc32(vm::EfunArgs&, vm::Arg data) {
  int len;
  unsigned char* buf;
  uint32_t crc;

  /* string or buffer: the spec has checked it */
  if (data.is(T_STRING)) {
    vm::StringArg str = data.string();
    len = str.size();
    buf = reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str()));
  } else {
    buffer_t* b = data.buffer();
    len = b->size;
    buf = b->item;
  }
  crc = compute_crc32(buf, len);
  return static_cast<LPC_INT>(crc);
}
#endif
