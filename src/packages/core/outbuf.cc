#include "base/package_api.h"

#include "packages/core/outbuf.h"

vm::Value outbuf_value(outbuffer_t* outbuf) {
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
    return vm::Value::str_malloced(outbuf->buffer);
  }
  return vm::Value::str_constant("");
}
