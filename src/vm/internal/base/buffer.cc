/* buffer.c for MudOS 0.9.x by John Garnett, 1993/11/07 */

#include "base/std.h"

#include "vm/internal/base/machine.h"

buffer_t null_buf = {
    1, /* Ref count, which will ensure that it will
        * never be deallocated */
    0  /* size */
};

buffer_t *null_buffer() {
  null_buf.ref++;
  return &null_buf;
} /* null_buffer() */

void free_buffer(buffer_t *b) {
  b->ref--;
  /* don't try to free the null_buffer (ref count might overflow) */
  if ((b->ref > 0) || (b == &null_buf)) {
    return;
  }
  FREE((char *)b);
} /* free_buffer() */

buffer_t *allocate_buffer(int size) {
  auto max_buffer_size = CONFIG_INT(__MAX_BUFFER_SIZE__);

  buffer_t *buf;

  if ((size < 0) || (size > max_buffer_size)) {
    error("Illegal buffer size.\n");
  }
  if (size == 0) {
    return null_buffer();
  }
  /* using calloc() so that memory will be zero'd out when allocated */
  buf = reinterpret_cast<buffer_t *>(
      DCALLOC(sizeof(buffer_t) + size - 1, 1, TAG_BUFFER, "allocate_buffer"));
  buf->size = size;
  buf->ref = 1;
  return buf;
}

int write_buffer(buffer_t *buf, int start, const char *str, int theLength) {
  int size;

  size = buf->size;
  if (start < 0) {
    start = size + start;
    if (start < 0) {
      return 0;
    }
  }
  /*
   * can't write past the end of the buffer since we can't reallocate the
   * buffer here (no easy way to propagate back the changes to the caller
   */
  if ((start + theLength) > size) {
    return 0;
  }
  memcpy(buf->item + start, str, theLength);
  return 1;
} /* write_buffer() */

char *read_buffer(buffer_t *b, int start, int len, int *rlen) {
  char *str;
  unsigned int size;

  if (len < 0) {
    return nullptr;
  }

  size = b->size;
  if (start < 0) {
    start = size + start;
    if (start < 0) {
      return nullptr;
    }
  }
  if (len == 0) {
    len = size;
  }
  if (start >= size) {
    return nullptr;
  }
  if ((start + len) > size) {
    len = (size - start);
  }
  for (str = reinterpret_cast<char *>(b->item) + start, size = 0; *str && size < len;
       str++, size++) {
    ;
  }
  str = new_string(size, "read_buffer: str");
  memcpy(str, b->item + start, size);
  str[ *rlen = size] = '\0';

  return str;
} /* read_buffer() */
