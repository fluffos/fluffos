#include "base/package_api.h"

#include "packages/core/outbuf.h"

void outbuf_push(outbuffer_t *outbuf) {
  STACK_INC;
  sp->type = T_STRING;
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);

    sp->subtype = STRING_MALLOC;
    sp->u.string = outbuf->buffer;
  } else {
    sp->subtype = STRING_CONSTANT;
    sp->u.string = "";
  }
}
