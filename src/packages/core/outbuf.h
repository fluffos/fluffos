#ifndef PACKAGES_CORE_OUTBUF_H
#define PACKAGES_CORE_OUTBUF_H

class LPCFrame;

/* Hand the buffer's contents over as an LPC string: outbuf_ret() as the
 * efun's result, outbuf_push() onto the stack top for a helper that
 * produces a value for its caller. Either way the buffer is consumed. */
void outbuf_ret(LPCFrame& f, struct outbuffer_t* outbuf);
void outbuf_push(struct outbuffer_t* outbuf);

#endif
