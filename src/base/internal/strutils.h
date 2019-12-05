#ifndef STRUTILS_H
#define STRUTILS_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <unicode/utf8.h>
#include <unicode/utypes.h>
#include <unicode/ustring.h>
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

}  // namespace
#endif  // STRUTILS_H
