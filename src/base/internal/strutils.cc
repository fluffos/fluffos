#include <cctype>
#include <string.h>
#include <unicode/brkiter.h>

#include "thirdparty/utf8_decoder_dfa/decoder.h"
#include "thirdparty/widecharwidth/widechar_width.h"

#include "strutils.h"

// Addition by Yucong Sun

bool u8_validate(const uint8_t *s) {
  uint32_t codepoint, state = 0;

  while (*s) decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}

bool u8_egc_count(const char *src, size_t *count) {
  UErrorCode status = U_ZERO_ERROR;
  size_t total = 0;
  UText text = UTEXT_INITIALIZER;

  utext_openUTF8(&text, src, -1, &status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return false;
  }

  /* create an iterator for graphemes */
  std::unique_ptr<icu::BreakIterator> brk(
      icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return false;
  }
  brk->setText(&text, status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return false;
  }
  brk->first();
  while (brk->next() != icu::BreakIterator::DONE) ++total;

  *count = total;

  utext_close(&text);
  return true;
}

// Return the egc at given index of src, if it is an single code point
UChar32 u8_egc_index_as_single_codepoint(const char *src, int32_t index) {
  UChar32 c = U_SENTINEL;
  auto start = u8_egc_index_to_offset(src, index);
  auto end = u8_egc_index_to_offset(src, index + 1);

  if (start >= 0 && end >= start && end - start < U8_MAX_LENGTH) U8_NEXT_UNSAFE(src, start, c);

  return c;
}

// Copy string src to dest, replacing character at index to c. Assuming dst is already allocated.
void u8_copy_and_replace_codepoint_at(const char *src, char *dst, int32_t index, UChar32 c) {
  int32_t src_offset = u8_egc_index_to_offset(src, index);
  int32_t dst_offset = 0;

  memcpy(dst, src, src_offset);
  dst_offset = src_offset;
  U8_APPEND_UNSAFE(dst, dst_offset, c);

  U8_FWD_1_UNSAFE(src, src_offset);
  strcpy((char *)dst + dst_offset, (const char *)src + src_offset);
}

// Get the byte offset to the egc index, doesn't check validity or bounds.
int32_t u8_egc_index_to_offset(const char *src, int32_t index) {
  UErrorCode status = U_ZERO_ERROR;
  int32_t pos = -1;
  UText text = UTEXT_INITIALIZER;

  utext_openUTF8(&text, src, -1, &status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return -1;
  }

  std::unique_ptr<icu::BreakIterator> brk(
      icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return -1;
  }

  brk->setText(&text, status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return -1;
  }

  pos = brk->first();
  while (index-- > 0 && pos >= 0) {
    pos = brk->next();
  }

  utext_close(&text);
  return pos;
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
    auto width = widechar_wcwidth(c);
    if (width > 0) {
      total += width;
    } else if (width == widechar_ambiguous) {
      total += 2;
    }
  }
  return total;
}
