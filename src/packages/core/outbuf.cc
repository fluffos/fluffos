#include "base/package_api.h"

#include "packages/core/outbuf.h"

void outbuf_push(outbuffer_t *outbuf) {
  STACK_INC;
  sp->type = T_STRING;
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = static_cast<char*>(realloc(outbuf->buffer, outbuf->real_size));

    sp->subtype = 0;
    sp->u.string = std::string{outbuf->buffer};
  } else {
    sp->subtype = 0;
    sp->u.string = std::string {""};
  }
}
