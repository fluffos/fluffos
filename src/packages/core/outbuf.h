#ifndef PACKAGES_CORE_OUTBUF_H
#define PACKAGES_CORE_OUTBUF_H

class Value;

/* The buffer's contents as an LPC string ("" when nothing was written);
 * the buffer is consumed. */
Value outbuf_value(struct outbuffer_t* outbuf);

#endif
