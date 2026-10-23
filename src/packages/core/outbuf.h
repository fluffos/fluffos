#ifndef PACKAGES_CORE_OUTBUF_H
#define PACKAGES_CORE_OUTBUF_H

namespace vm {
class Value;
}  // namespace vm

/* The buffer's contents as an LPC string ("" when nothing was written);
 * the buffer is consumed. */
vm::Value outbuf_value(struct outbuffer_t* outbuf);

#endif
