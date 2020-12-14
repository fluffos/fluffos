#include "base/internal/strutils.h"

#include <cctype>
#include <string.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>

#include "thirdparty/utf8_decoder_dfa/decoder.h"
#include "thirdparty/widecharwidth/widechar_width.h"
#include "thirdparty/utfcpp/source/utf8.h"

#include "base/internal/log.h"
#include "base/internal/rc.h"
#include "base/internal/tracing.h"

// Addition by Yucong Sun

bool u8_validate(const char *s) {
  auto p = (const uint8_t *)s;
  uint32_t codepoint, state = 0;

  while (*p) decode(&state, &codepoint, *p++);

  return state == UTF8_ACCEPT;
}

bool u8_validate(const uint8_t *s, size_t len) {
  auto end = s + len;
  uint32_t codepoint, state = 0;

  while (s < end && *s) decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}

std::string u8_sanitize(std::string_view src) { return utf8::replace_invalid(src); }

namespace {
typedef std::function<void(std::unique_ptr<icu::BreakIterator> &)> u8_char_iter_callback;
bool u8_char_iter(const char *src, const u8_char_iter_callback &cb) {
  static std::unique_ptr<icu::BreakIterator> brk = nullptr;

  /* create an iterator for graphemes */
  if (!brk) {
    UErrorCode status = U_ZERO_ERROR;
    brk.reset(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
    if (!U_SUCCESS(status)) {
      return false;
    }
  }

  UErrorCode status = U_ZERO_ERROR;
  UText text = UTEXT_INITIALIZER;

  utext_openUTF8(&text, src, -1, &status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return false;
  }

  status = U_ZERO_ERROR;
  brk->setText(&text, status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    return false;
  }
  brk->first();

  cb(brk);

  utext_close(&text);
  return true;
}
}  // namespace

bool u8_egc_count(const char *src, size_t *count) {
  *count = 0;

  return u8_char_iter(src, [&](std::unique_ptr<icu::BreakIterator> &brk) {
    int total = 0;
    brk->first();
    while (brk->next() != icu::BreakIterator::DONE) ++total;
    *count = total;
  });
}

// Search "needle' in 'haystack', making sure it matches EGC boundary.
int u8_egc_find(const char *haystack, size_t haystack_len, const char *needle, size_t needle_len,
                bool reverse) {
  int index = -1;

  u8_char_iter(haystack, [=, &index](std::unique_ptr<icu::BreakIterator> &brk) {
    bool found = false;
    std::unique_ptr<icu::BreakIterator> brk_tmp(brk->clone());
    if (!reverse) {
      brk->first();
      while (true) {
        auto start = brk->current();
        if ((haystack_len - start) >= needle_len && brk_tmp->isBoundary(start + needle_len)) {
          if (memcmp(&haystack[start], needle, needle_len) == 0) {
            found = true;
            break;
          }
        }
        if (brk->next() == icu::BreakIterator::DONE) {
          break;
        }
      }
    } else {
      brk->last();
      while (true) {
        auto start = brk->current();
        if ((haystack_len - start) >= needle_len && brk_tmp->isBoundary(start + needle_len)) {
          if (memcmp(&haystack[start], needle, needle_len) == 0) {
            found = true;
            break;
          }
        }
        if (brk->previous() == icu::BreakIterator::DONE) {
          break;
        }
      }
    }
    if (!found) {
      index = -1;
    } else {
      // reverse counting
      index = 0;
      while (brk->previous() != icu::BreakIterator::DONE) index++;
    }
  });

  return index;
}

// Return the egc at given index of src, if it is an single code point
UChar32 u8_egc_index_as_single_codepoint(const char *src, int32_t index) {
  UChar32 res = U_SENTINEL;

  u8_char_iter(src, [&](std::unique_ptr<icu::BreakIterator> &brk) {
    int32_t pos = -1;
    {
      pos = brk->first();
      while (index-- > 0 && pos >= 0) {
        pos = brk->next();
      }
    }

    // out-of-bounds
    if (pos < 0) {
      res = -2;
      return;
    }

    // index is end-of-string
    if (src[pos] == 0) {
      res = 0;
      return;
    }

    auto next_pos = brk->next();
    if (next_pos >= 0) {
      if (next_pos - pos <= U8_MAX_LENGTH) {
        U8_NEXT((const uint8_t *)src, pos, -1, res);
      }
    }
  });

  return res;
}

// Copy string src to dest, replacing character at index to c. Assuming dst is already allocated.
void u8_copy_and_replace_codepoint_at(const char *src, char *dst, int32_t index, UChar32 c) {
  int32_t src_offset = u8_egc_index_to_offset(src, index);
  int32_t dst_offset = 0;

  memcpy(dst, src, src_offset);
  dst_offset = src_offset;
  U8_APPEND_UNSAFE(dst, dst_offset, c);

  U8_FWD_1_UNSAFE(src, src_offset);
  strcpy(dst + dst_offset, src + src_offset);
}

// Get the byte offset to the egc index, doesn't check validity or bounds.
int32_t u8_egc_index_to_offset(const char *src, int32_t index) {
  int pos = -1;

  u8_char_iter(src, [&](std::unique_ptr<icu::BreakIterator> &brk) {
    pos = brk->first();
    while (index-- > 0 && pos >= 0) {
      pos = brk->next();
    }
  });

  return pos;
}

// same as strncpy, copy up to maxlen bytes but will not copy broken characters.
int32_t u8_strncpy(uint8_t *dest, const uint8_t *src, const int32_t maxlen) {
  auto len = u8_truncate(src, strnlen(reinterpret_cast<const char *>(src), maxlen));
  if (len != maxlen) {
    memset(dest + len, '\0', maxlen - len);
  }
  memcpy(dest, src, len);
  return len;
}

// From ICU 61
/**
 * Internal bit vector for 3-byte UTF-8 validity check, for use in U8_IS_VALID_LEAD3_AND_T1.
 * Each bit indicates whether one lead byte + first trail byte pair starts a valid sequence.
 * Lead byte E0..EF bits 3..0 are used as byte index,
 * first trail byte bits 7..5 are used as bit index into that byte.
 * @see U8_IS_VALID_LEAD3_AND_T1
 * @internal
 */
#ifndef U8_LEAD3_T1_BITS
#define U8_LEAD3_T1_BITS "\x20\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x10\x30\x30"
#endif

/**
 * Internal 3-byte UTF-8 validity check.
 * Non-zero if lead byte E0..EF and first trail byte 00..FF start a valid sequence.
 * @internal
 */
#ifndef U8_IS_VALID_LEAD3_AND_T1
#define U8_IS_VALID_LEAD3_AND_T1(lead, t1) \
  (U8_LEAD3_T1_BITS[(lead)&0xf] & (1 << ((uint8_t)(t1) >> 5)))
#endif

/**
 * Internal bit vector for 4-byte UTF-8 validity check, for use in U8_IS_VALID_LEAD4_AND_T1.
 * Each bit indicates whether one lead byte + first trail byte pair starts a valid sequence.
 * First trail byte bits 7..4 are used as byte index,
 * lead byte F0..F4 bits 2..0 are used as bit index into that byte.
 * @see U8_IS_VALID_LEAD4_AND_T1
 * @internal
 */
#ifndef U8_LEAD4_T1_BITS
#define U8_LEAD4_T1_BITS "\x00\x00\x00\x00\x00\x00\x00\x00\x1E\x0F\x0F\x0F\x00\x00\x00\x00"
#endif

/**
 * Internal 4-byte UTF-8 validity check.
 * Non-zero if lead byte F0..F4 and first trail byte 00..FF start a valid sequence.
 * @internal
 */
#ifndef U8_IS_VALID_LEAD4_AND_T1
#define U8_IS_VALID_LEAD4_AND_T1(lead, t1) \
  (U8_LEAD4_T1_BITS[(uint8_t)(t1) >> 4] & (1 << ((lead)&7)))
#endif
/**
 * If the string ends with a UTF-8 byte sequence that is valid so far
 * but incomplete, then reduce the length of the string to end before
 * the lead byte of that incomplete sequence.
 * For example, if the string ends with E1 80, the length is reduced by 2.
 *
 * In all other cases (the string ends with a complete sequence, or it is not
 * possible for any further trail byte to extend the trailing sequence)
 * the length remains unchanged.
 *
 * Useful for processing text split across multiple buffers
 * (save the incomplete sequence for later)
 * and for optimizing iteration
 * (check for string length only once per character).
 *
 * "Safe" macro, checks for illegal sequences and for string boundaries.
 * Unlike U8_SET_CP_START(), this macro never reads s[length].
 *
 * (In UTF-16, simply check for U16_IS_LEAD(last code unit).)
 *
 * @param s const uint8_t * string
 * @param start int32_t starting string offset (usually 0)
 * @param length int32_t string length (usually start<=length)
 * @see U8_SET_CP_START
 * @stable ICU 61
 */
#ifndef U8_TRUNCATE_IF_INCOMPLETE
#define U8_TRUNCATE_IF_INCOMPLETE(s, start, length)                                   \
  do {                                                                                \
    if ((length) > (start)) {                                                         \
      uint8_t __b1 = s[(length)-1];                                                   \
      if (U8_IS_SINGLE(__b1)) {                                                       \
        /* common ASCII character */                                                  \
      } else if (U8_IS_LEAD(__b1)) {                                                  \
        --(length);                                                                   \
      } else if (U8_IS_TRAIL(__b1) && ((length)-2) >= (start)) {                      \
        uint8_t __b2 = s[(length)-2];                                                 \
        if (0xe0 <= __b2 && __b2 <= 0xf4) {                                           \
          if (__b2 < 0xf0 ? U8_IS_VALID_LEAD3_AND_T1(__b2, __b1)                      \
                          : U8_IS_VALID_LEAD4_AND_T1(__b2, __b1)) {                   \
            (length) -= 2;                                                            \
          }                                                                           \
        } else if (U8_IS_TRAIL(__b2) && ((length)-3) >= (start)) {                    \
          uint8_t __b3 = s[(length)-3];                                               \
          if (0xf0 <= __b3 && __b3 <= 0xf4 && U8_IS_VALID_LEAD4_AND_T1(__b3, __b2)) { \
            (length) -= 3;                                                            \
          }                                                                           \
        }                                                                             \
      }                                                                               \
    }                                                                                 \
  } while (false)
#endif

// truncate strlen() to last valid codepoint
size_t u8_truncate(const uint8_t *src, size_t len) {
  int32_t res = len;
  U8_TRUNCATE_IF_INCOMPLETE(src, 0, res);
  return res;
}

// Truncate string to last valid codepoint that doesn't exceed maximum width, returns real width and
// new len. If brake_at_space, then attempts to truncate before the last ' ' character. If
// always_break_at_newline, then always truncate to first '\n' character. Invalid codepoints are
// replaced to 0xfffd;
void u8_truncate_below_width(const char *src, size_t len, size_t max_width, bool break_for_line,
                             bool always_break_before_newline, size_t *out_len, size_t *out_width) {
  if (len == 0) {
    *out_len = 0;
    *out_width = 0;
    return;
  }

  static std::unique_ptr<icu::BreakIterator> brk = nullptr, linebrk = nullptr;
  if (!brk) {
    UErrorCode status = U_ZERO_ERROR;
    brk.reset(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
    if (!U_SUCCESS(status)) {
      debug_message("u8_truncate_below_width: Unable to create break iterator! error %d: %s\n",
                    status, u_errorName(status));
      return;
    }
  }
  if (!linebrk) {
    UErrorCode status = U_ZERO_ERROR;
    linebrk.reset(icu::BreakIterator::createLineInstance(icu::Locale::getDefault(), status));
    if (!U_SUCCESS(status)) {
      debug_message("u8_truncate_below_width: Unable to create break iterator! error %d: %s\n",
                    status, u_errorName(status));
      return;
    }
  }

  bool hardwrap = false;

  // length to the breakpoint
  size_t break_length = 0;
  // total width at breakponint, expect to be updated.
  size_t break_width = 0;

  UErrorCode status = U_ZERO_ERROR;

  UText text = UTEXT_INITIALIZER;
  utext_openUTF8(&text, src, len, &status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    debug_message("u8_truncate_below_width: utext_openUTF8 error %d: %s\n", status,
                  u_errorName(status));
    return;
  }

  status = U_ZERO_ERROR;
  brk->setText(&text, status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    debug_message("u8_truncate_below_width: setText error %d: %s\n", status, u_errorName(status));
    return;
  }

  status = U_ZERO_ERROR;
  linebrk->setText(&text, status);
  if (!U_SUCCESS(status)) {
    utext_close(&text);
    debug_message("u8_truncate_below_width: setText error %d: %s\n", status, u_errorName(status));
    return;
  }

  break_length = brk->first();
  int pos = break_length;

  while (pos != icu::BreakIterator::DONE) {
    char c = src[pos];

    // Skip over possible ansi color code
    if (CONFIG_INT(__RC_SPRINTF_ADD_JUSTFIED_IGNORE_ANSI_COLORS__)) {
      while (c == '\x1B') {
        auto start_pos = pos;
        pos++;
        if (pos == len) {
          pos = start_pos;
          break;
        }
        c = src[pos];
        if (c == '[') {
          pos++;
          if (pos == len) {
            pos = start_pos;
            break;
          }
          c = src[pos];
          while (c == ';' || isdigit(c)) {
            pos++;
            if (pos == len) {
              pos = start_pos;
              break;
            }
            c = src[pos];
          }
          if (c == 'm') {
            pos++;
            if (pos > len) {
              pos = start_pos;
              break;
            }
            c = src[pos];
          }
        }
      }
    }

    c = src[pos];

    // If we found '\n' break right away
    if (always_break_before_newline) {
      if (c == '\n') {
        hardwrap = true;

        break_length = pos;
        break_width = u8_width(src, break_length);
        break;
      }
    }

    auto new_width = break_width + u8_width(src + break_length, pos - break_length);

    // break right on spot
    if (new_width > 0 && new_width == max_width) {
      break_length = pos;
      break_width = new_width;
      break;
    }

    if (new_width > max_width) {
      // If we will ends up with nothing, force break at current character
      // This is special case for wide character, sprintf("%-=1s", "一二三四五")
      if (break_width == 0) {
        hardwrap = true;

        break_length = pos;
        break_width = new_width;
        break;
      }
      // Otherwise just break on last point
      break;
    }

    // not enough width yet, move on to next character
    break_length = pos;
    break_width = new_width;
    pos = brk->next();
  }

  if (break_for_line) {
    // Try to find an better line break point
    if (!hardwrap && src[break_length] != ' ' && !linebrk->isBoundary(break_length)) {
      auto potential_breakpoint = linebrk->preceding(break_length);
      // Suitable breakpoints
      if (potential_breakpoint > 0) {
        break_length = potential_breakpoint;
        break_width = u8_width(src, break_length);
      }
    }
    // Eat the last space
    if (break_length > 1 && src[break_length - 1] == ' ') {
      break_length--;
      break_width--;
    }
  }

  DEBUG_CHECK(break_length == 0 && src[break_length] != ' ' && src[break_length] != '\n',
              "BUG: truncated to empty string!");

  utext_close(&text);

  *out_len = break_length;
  *out_width = break_width;
}

// Total width of characters(grapheme cluster). Also adjust for east asain full width characters
size_t u8_width(const char *src, int len) {
  size_t total = 0;
  int32_t src_offset = 0;

  UChar32 c = 0;
  UChar32 prev = 0;

  if (len == 0) return 0;
  for (;;) {
    prev = c;
    U8_NEXT(src, src_offset, len, c);

    // Treat invalid codepoints as replacement chars
    if (c < 0) c = 0xfffd;
    if (c == 0) break;
    if (c == 0x200d || prev == 0x200d) {  // zwj, skip the next character
      continue;
    }

    // ignoring ANSI codes when calculating display width
    // https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
    // format is "\x1b[X;Ym"
    if (CONFIG_INT(__RC_SPRINTF_ADD_JUSTFIED_IGNORE_ANSI_COLORS__)) {
      if (c == 0x1B) {
        auto p = src + src_offset;
        auto end = (len > 0) ? src + len : nullptr;
        if (p != end && *p == '[') {
          p++;
          // we don't check validity here, just assume valid code
          while (p != end && (isdigit(*p) || *p == ';')) p++;
          if (p != end && *p == 'm') {
            p++;
            src_offset = p - src;
            continue;
          }
        }
      }
    }

    auto width = widechar_wcwidth(c);
    if (width > 0) {
      total += width;
    } else if (width == widechar_ambiguous) {
      total += 2;
    }
    if (len > 0 && src_offset >= len) break;
  }
  return total;
}
