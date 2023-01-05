#include "base/internal/strutils.h"

#include <cctype>
#include <vector>
#include <string>
#include <memory>
#include <cstring>

#include "thirdparty/utf8_decoder_dfa/decoder.h"
#include "thirdparty/widecharwidth/widechar_width.h"
#include "thirdparty/utfcpp/source/utf8.h"

#include "base/internal/log.h"
#include "base/internal/rc.h"
#include "base/internal/EGCIterator.h"

bool u8_validate(char **s) {
  const auto *p = (const uint8_t *)(*s);
  uint32_t codepoint, state = 0;

  while (*p && state != UTF8_REJECT) decode(&state, &codepoint, *p++);
  *s = (char *)p;
  return state == UTF8_ACCEPT;
}

bool u8_validate(const char *s) {
  const auto *p = (const uint8_t *)s;
  uint32_t codepoint, state = 0;

  while (*p && state != UTF8_REJECT) decode(&state, &codepoint, *p++);

  return state == UTF8_ACCEPT;
}

bool u8_validate(const uint8_t *s, size_t len) {
  const auto *end = s + len;
  uint32_t codepoint, state = 0;

  while (s < end && *s && state != UTF8_REJECT) decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}

std::string u8_sanitize(std::string_view src) { return utf8::replace_invalid(src); }

// Search "needle' in 'haystack', making sure it matches EGC boundary, returning byte offset.
int32_t u8_egc_find_as_offset(EGCIterator &iter, const char *needle, size_t needle_len,
                              bool reverse) {
  const char *haystack = iter.data();
  size_t const haystack_len = iter.len() == -1 ? strlen(haystack) : iter.len();

  // no way
  if (needle_len > haystack_len) {
    return -1;
  }
  if (!iter.ok()) return -1;

  // fast track ascii string search upto 4 characters.
  if (!reverse) {
    bool is_all_ascii = false;
    for (int i = 0; i < 4 && i < needle_len; i++) {
      char const c = needle[i];
      is_all_ascii = c >= 0;
      if (!is_all_ascii) break;
      if (c == '\0') break;
      if (i == 3) is_all_ascii = false;
    }
    if (is_all_ascii) {
      // strstr doesn't follow haystack_len, so we may overrun, wasting some cycles.
      const auto *res = strstr(haystack, needle);
      auto ret = res == nullptr ? -1 : (decltype(haystack))res - haystack;
      if (ret >= haystack_len) ret = -1;
      return ret;
    }
  }

  int res = -1;

  std::string_view const sv_haystack(haystack, haystack_len);
  std::string_view const sv_needle(needle, needle_len);
  auto pos = std::string_view::npos;
  if (!reverse) {
    pos = 0;
    while ((pos = sv_haystack.find(sv_needle, pos)) != std::string_view::npos) {
      if (iter->isBoundary(pos) && iter->isBoundary(pos + sv_needle.length())) break;
      pos++;
    }
  } else {
    pos = std::string_view::npos;
    while ((pos = sv_haystack.rfind(sv_needle, pos)) != std::string_view::npos) {
      if (iter->isBoundary(pos) && iter->isBoundary(pos + sv_needle.length())) break;
      pos--;
    }
  }
  if (pos != std::string_view::npos) {
    res = pos;
  }
  return res;
}

// Return the egc at given index of src, if it is an single code point.
// Return -2 if requested index is out of bounds
// Return -1 if requested EGC is multi codepoint
UChar32 u8_egc_index_as_single_codepoint(const char *src, int32_t src_len, int32_t index) {
  UChar32 c = U_SENTINEL;

  EGCSmartIterator iter(src, src_len);
  if (!iter.ok()) return c;

  auto pos = iter.index_to_offset(index);
  // out-of-bounds
  if (pos < 0) return -2;
  auto post_pos = iter.post_index_to_offset(index);
  // end-of-string
  if (post_pos < 0) return 0;

  if (post_pos - pos > U8_MAX_LENGTH) return c;
  U8_NEXT((const uint8_t *)src, pos, -1, c);
  return c;
}

// Copy string src to dest, replacing character at index to c. Assuming dst is already allocated.
void u8_copy_and_replace_codepoint_at(EGCSmartIterator &iter, char *dst, int32_t index, UChar32 c) {
  if (!iter.ok()) return;

  const char *src = iter.data();
  int32_t const slen = iter.len();

  int32_t src_offset = iter.index_to_offset(index);
  int32_t dst_offset = 0;

  if (src_offset < 0) return;
  memcpy(dst, src, src_offset);
  dst_offset = src_offset;
  U8_APPEND_UNSAFE(dst, dst_offset, c);

  U8_FWD_1_UNSAFE(src, src_offset);
  memcpy(dst + dst_offset, src + src_offset, slen - src_offset + 1);
}

// Get the byte offset to the egc index, return -1 for non boundary.
int32_t u8_offset_to_egc_index(EGCIterator &iter, int32_t offset) {
  if (offset <= 0) return offset;
  if (!iter.ok()) return -1;

  int idx = -1;
  int pos = 0;

  pos = iter->first();
  idx = 0;
  do {
    pos = iter->next();
    idx++;
  } while (pos != icu::BreakIterator::DONE && pos < offset);

  if (pos == icu::BreakIterator::DONE) idx = -1;
  if (pos != offset) idx = -1;

  return idx;
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
      auto prev_linebreak = linebrk->preceding(break_length);
      // Suitable breakpoints
      if (prev_linebreak > 0) {
        break_length = prev_linebreak;
        break_width = u8_width(src, break_length);
      } else {
        auto next_linebreak = linebrk->following(break_length);
        if (next_linebreak > 0) {
          if (src[next_linebreak - 1] == '\n') {
            next_linebreak--;
          }
          break_length = next_linebreak;
          break_width = u8_width(src, break_length);
        }
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
        const auto *p = src + src_offset;
        const auto *end = (len > 0) ? src + len : nullptr;
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
    } else if (width == widechar_widened_in_9) {
      total += 2;
    } else if (width == widechar_private_use) {
      total += 1;
    } else if (width == widechar_ambiguous) {
      total += 1;
    }
    if (len > 0 && src_offset >= len) break;
  }
  return total;
}

std::vector<std::string_view> u8_egc_split(const char *src, int32_t slen) {
  std::vector<std::string_view> result;
  result.reserve(16);

  EGCSmartIterator iter(src, slen);
  if (!iter.ok()) return result;

  iter->first();
  auto start = iter->current();
  while (iter->next() != icu::BreakIterator::DONE) {
    auto size = iter->current() - start;
    result.emplace_back(src + start, size);
    start = iter->current();
  }

  return result;
}

// Return empty string if error or invalid translator.
std::string u8_convert_encoding(UConverter *trans, const char *data, int len) {
  std::string result;

  if (trans) {
    UErrorCode error_code = U_ZERO_ERROR;

    auto required = ucnv_fromAlgorithmic(trans, UCNV_UTF8, nullptr, 0, data, len, &error_code);
    if (error_code == U_BUFFER_OVERFLOW_ERROR) {
      result.resize(required);

      error_code = U_ZERO_ERROR;
      ucnv_fromAlgorithmic(trans, UCNV_UTF8, result.data(), result.size(), data, len, &error_code);
      if (U_FAILURE(error_code)) {
        debug_message("add_message: Translation failed!");
        result = "";
      }
    }
  }
  return result;
}
