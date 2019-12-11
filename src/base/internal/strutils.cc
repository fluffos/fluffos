#include <cctype>
#include <string.h>

#include "thirdparty/utf8_decoder_dfa/decoder.h"
#include "thirdparty/widecharwidth/widechar_width.h"

#include "strutils.h"

// Addition by Yucong Sun

// check string s is valid utf8
bool u8_validate(const uint8_t *s) {
  uint32_t codepoint, state = 0;

  while (*s) decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}

// saves number of codepoints in string s, to count.
// Returns false if string is not valid utf8
bool u8_codepoints(const uint8_t *s, size_t *count) {
  uint32_t codepoint, state = 0;

  for (*count = 0; *s; ++s)
    if (!decode(&state, &codepoint, *s)) *count += 1;

  return state == UTF8_ACCEPT;
}

// Return the character at given index of str
UChar32 u8_codepoint_at(const uint8_t *str, int32_t index) {
  int32_t offset = 0;
  U8_FWD_N(str, offset, -1, index);

  UChar32 c = 0;
  U8_NEXT(str, offset, -1, c);
  return c;
}

// Copy string src to dest, replacing character at index to c. Assuming dst is already allocated.
void u8_copy_and_replace_codepoint_at(const uint8_t *src, uint8_t *dst, int32_t index, UChar32 c) {
  int32_t src_offset = 0;
  int32_t dst_offset = 0;

  U8_FWD_N(src, src_offset, -1, index);
  memcpy(dst, src, src_offset);
  dst_offset = src_offset;
  U8_APPEND_UNSAFE(dst, dst_offset, c);

  U8_FWD_1_UNSAFE(src, src_offset);
  strcpy((char *)dst + dst_offset, (const char *)src + src_offset);
}

// Get the byte offset to the codepoint index, doesn't check validity or bounds.
size_t u8_codepoint_index_to_offset(const uint8_t *src, int32_t index) {
  size_t offset = 0;
  U8_FWD_N_UNSAFE(src, offset, index);
  return offset;
}

// same as strncpy, copy up to maxlen bytes but will not copy broken characters.
int32_t u8_strncpy(uint8_t *dest, const uint8_t *src, size_t maxlen) {
  int32_t src_offset = 0;
  int32_t written = 0;
  for (;;) {
    UChar32 c = 0;
    U8_NEXT(src, src_offset, -1, c);
    if (c <= 0) break;
    UBool isError = FALSE;
    U8_APPEND(dest, written, maxlen, c, isError);
    if (isError == TRUE) break;
  }
  return written;
}

// Total width of characters(grapheme cluster). Also adjust for east asain full width characters
size_t u8_width(const char *src) {
  size_t total = 0;
  int32_t src_offset = 0;

  UChar32 c = 0;
  UChar32 prev = 0;
  for (;;) {
    prev = c;
    U8_NEXT_OR_FFFD(src, src_offset, -1, c);
    if (c <= 0) break;
    if (c == 0x200d || prev == 0x200d) {  // zwj, skip the next character
      continue;
    }
    size_t width = widechar_wcwidth(c);
    if (width > 0) {
      total += width;
    } else if (width == widechar_ambiguous) {
      total += 2;
    }
  }
  return total;
}
