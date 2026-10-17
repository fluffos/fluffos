#include "base/package_api.h"

#include "packages/core/outbuf.h"

/* The buffer's contents as an LPC string; "" when nothing was written. */
static svalue_t outbuf_string(outbuffer_t* outbuf) {
  svalue_t v;
  v.type = T_STRING;
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
    v.subtype = STRING_MALLOC;
    v.u.string = outbuf->buffer;
  } else {
    v.subtype = STRING_CONSTANT;
    v.u.string = "";
  }
  return v;
}

void outbuf_push(outbuffer_t* outbuf) {
  svalue_t v = outbuf_string(outbuf);
  transfer_push_some_svalues(&v, 1);
}

void outbuf_ret(LPCFrame& f, outbuffer_t* outbuf) { f.ret_owned(outbuf_string(outbuf)); }
