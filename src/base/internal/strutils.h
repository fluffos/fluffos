#ifndef STRUTILS_H
#define STRUTILS_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <unicode/utf8.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/brkiter.h>

#include "thirdparty/utf8_decoder_dfa/decoder.h"

namespace {

const auto fn_not_isspace = [](int c) { return !std::isspace(c); };

// trim from start
inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), fn_not_isspace));
  return s;
}

// trim from end
inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), fn_not_isspace).base(), s.end());
  return s;
}

// trim from both ends
inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

inline bool starts_with(const std::string &big_str, const std::string &small_str) {
  return big_str.compare(0, small_str.length(), small_str) == 0;
}

inline bool ends_with(const std::string &str, const std::string &ending) {
  if (str.length() < ending.length()) return false;
  return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}

inline void ReplaceStringInPlace(std::string &subject, const std::string &search,
                                 const std::string &replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

// Addition by Yucong Sun

// check string s is valid utf8
inline bool u8_validate(const uint8_t *s) {
  uint32_t codepoint, state = 0;

  while (*s) decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}

// saves number of codepoints in string s, to count.
// Returns false if string is not valid utf8
inline bool u8_codepoints(const uint8_t *s, size_t *count) {
  uint32_t codepoint, state = 0;

  for (*count = 0; *s; ++s)
    if (!decode(&state, &codepoint, *s)) *count += 1;

  return state == UTF8_ACCEPT;
}

// Return the character at given index of str
inline UChar32 u8_codepoint_at(const uint8_t *str, int32_t index) {
  int32_t offset = 0;
  U8_FWD_N(str, offset, -1, index);

  UChar32 c = 0;
  U8_NEXT(str, offset, -1, c);
  return c;
}

// Copy string src to dest, replacing character at index to c. Assuming dst is already allocated.
inline void u8_copy_and_replace_codepoint_at(const uint8_t *src, uint8_t *dst, int32_t index,
                                             UChar32 c) {
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
inline size_t u8_codepoint_index_to_offset(const uint8_t *src, int32_t index) {
  size_t offset = 0;
  U8_FWD_N_UNSAFE(src, offset, index);
  return offset;
}

// same as strncpy, copy up to maxlen bytes but will not copy broken characters.
inline int32_t u8_strncpy(uint8_t *dest, const uint8_t *src, size_t maxlen) {
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

// Return
inline size_t u8_charwidth(UChar32 codepoint, bool ambiguous_as_full_width = true) {
  if (!u_isdefined(codepoint) ||
      u_iscntrl(codepoint) ||
      u_getCombiningClass(codepoint) > 0 ||
      u_hasBinaryProperty(codepoint, UCHAR_EMOJI_MODIFIER)) {
    return 0;
  }
  // UCHAR_EAST_ASIAN_WIDTH is the Unicode property that identifies a
  // codepoint as being full width, wide, ambiguous, neutral, narrow,
  // or halfwidth.
  const int eaw = u_getIntPropertyValue(codepoint, UCHAR_EAST_ASIAN_WIDTH);
  switch (eaw) {
    case U_EA_FULLWIDTH:
    case U_EA_WIDE:
      return 2;
    case U_EA_AMBIGUOUS:
      // See: http://www.unicode.org/reports/tr11/#Ambiguous for details
      if (ambiguous_as_full_width) {
        return 2;
      }
      // Fall through if ambiguous_as_full_width if false.
    case U_EA_NEUTRAL:
      if (u_hasBinaryProperty(codepoint, UCHAR_EMOJI_PRESENTATION)) {
        return 2;
      }
      // Fall through
    case U_EA_HALFWIDTH:
    case U_EA_NARROW:
    default:
      return 1;
  }
}
// Total width of characters(grapheme cluster). Also adjust for east asain full width characters
inline size_t u8_width(const char* src, bool expand_emoji_sequence = false) {
  UErrorCode status = U_ZERO_ERROR;
  size_t total = 0;

  auto text = utext_openUTF8(nullptr, src, -1, &status);
  /* create an iterator for graphemes */
  auto *brk = icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status);
  if(!U_SUCCESS(status)) {
    return 0;
  }
  brk->setText(text, status);
  if(!U_SUCCESS(status)) {
    return 0;
  }
  int32_t pos = brk->first();
  UChar32 c = U_SENTINEL, prev;
  while(pos != icu::BreakIterator::DONE) {
    prev = c;
    // Treat asain chars as 2 width
    c = utext_char32At(text, pos);
    if (c != U_SENTINEL) {
      if (!expand_emoji_sequence &&
          prev == 0x200d &&  // 0x200d == ZWJ (zero width joiner)
          (u_hasBinaryProperty(c, UCHAR_EMOJI_PRESENTATION) ||
              u_hasBinaryProperty(c, UCHAR_EMOJI_MODIFIER))) {
        // skip
      } else {
        total += u8_charwidth(c);
      }
    }
    pos = brk->next();
  }

  delete brk;
  utext_close(text);

  return total;
}

}  // namespace
#endif  // STRUTILS_H
