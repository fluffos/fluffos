#ifndef STRUTILS_H
#define STRUTILS_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <unicode/utf8.h>
#include <unicode/uchar.h>

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
}  // namespace

// Check string s is valid utf8
bool u8_validate(const uint8_t *s);
// Saves number of EGC/Characters in string src, to count, return false if string is not valid utf8.
bool u8_egc_count(const char *src, size_t *count);
UChar32 u8_egc_index_as_single_codepoint(const char *str, int32_t index);
void u8_copy_and_replace_codepoint_at(const char *src, char *dst, int32_t index, UChar32 c);
int32_t u8_egc_index_to_offset(const char *src, int32_t index);
int32_t u8_strncpy(uint8_t *dest, const uint8_t *src, size_t maxlen);
size_t u8_width(const char *src);

#endif  // STRUTILS_H
