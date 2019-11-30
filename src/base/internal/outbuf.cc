#include "base/internal/outbuf.h"

#include <cstdarg>  // for va_*
#include <cstring>  // for strlen
#include <cstdio>   // for vsnprintf

#include "base/internal/rc.h"        // for CONFIG_*
#include "base/internal/log.h"       // for DEBUG_*
#include "base/internal/stralloc.h"  // for MSTR_*

void outbuf_zero(outbuffer_t *outbuf) {
  outbuf->real_size = 0;
  outbuf->buffer = nullptr;
}

int outbuf_extend(outbuffer_t *outbuf, int l) {
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int limit;

  DEBUG_CHECK(l < 0, "Negative length passed to outbuf_extend.\n");

  l = (l > max_string_length ? max_string_length : l);

  if (outbuf->buffer) {
    limit = MSTR_SIZE(outbuf->buffer);
    if (outbuf->real_size + l > limit) {
      if (outbuf->real_size == max_string_length) {
        return 0;
      } /* TRUNCATED */

      /* assume it's going to grow some more */
      limit = (outbuf->real_size + l) * 2;
      if (limit > max_string_length) {
        limit = max_string_length;
        outbuf->buffer = extend_string(outbuf->buffer, limit);
        return limit - outbuf->real_size;
      }
      outbuf->buffer = extend_string(outbuf->buffer, limit);
    }
  } else {
    outbuf->buffer = new_string(l, "outbuf_extend");
    outbuf->real_size = 0;
  }
  return l;
}

void outbuf_add(outbuffer_t *outbuf, const char *str) {
  int l, limit;

  if (!outbuf) {
    return;
  }
  l = strlen(str);
  if ((limit = outbuf_extend(outbuf, l)) > 0) {
    strncpy(outbuf->buffer + outbuf->real_size, str, limit);
    outbuf->real_size += (l > limit ? limit : l);
    *(outbuf->buffer + outbuf->real_size) = 0;
  }
}

void outbuf_addchar(outbuffer_t *outbuf, char c) {
  if (outbuf && (outbuf_extend(outbuf, 1) > 0)) {
    *(outbuf->buffer + outbuf->real_size++) = c;
    *(outbuf->buffer + outbuf->real_size) = 0;
  }
}

void outbuf_addv(outbuffer_t *outbuf, const char *format, ...) {
  char buf[LARGEST_PRINTABLE_STRING + 1];
  va_list args;

  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (!outbuf) {
    return;
  }

  outbuf_add(outbuf, buf);
}

void outbuf_fix(outbuffer_t *outbuf) {
  if (outbuf && outbuf->buffer) {
    outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
  }
}
