#include "base/package_api.h"

#include "packages/core/outbuf.h"

void outbuf_push(outbuffer_t* outbuf) {
  LPCFrame f = LPCFrame::builder();
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
    f.push_str_malloced(outbuf->buffer);
  } else {
    f.push_str_constant("");
  }
  f.done();
}
