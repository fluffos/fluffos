#ifndef STRUTILS_H
#define STRUTILS_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>
#include <unicode/utf8.h>
#include <unicode/uchar.h>

namespace {

// --------------------------------------------------------------------------
/// @brief removes given characters from beginning of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string &ltrim(std::string &&str, const std::string &chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

inline std::string ltrim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {
  std::string ret{str};

  ret.erase(0, str.find_first_not_of(chars));
  return ret;
}

// --------------------------------------------------------------------------
/// @brief removes given characters from end of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string &rtrim(std::string &&str, const std::string &chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

inline std::string rtrim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {
  std::string ret{str};

  ret.erase(str.find_last_not_of(chars) + 1);
  return ret;
}

// --------------------------------------------------------------------------
/// @brief removes given chars from beginning and end of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string &trim(std::string &&str, const std::string &chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}

inline std::string trim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}

// --------------------------------------------------------------------------
/// @brief checks beginning of string1 for presens of string2
///
/// @param str   string to be examined
/// @param start string to be searched
///
/// @return returns true if str begins with startm false otherwise
// --------------------------------------------------------------------------
inline bool starts_with(const std::string &str, const std::string &start) {
  return str.compare(0, start.length(), start) == 0;
}

// --------------------------------------------------------------------------
/// @brief checks end of string 1 for presens of string 2
///
/// @param str   string to be examined
/// @param end   string to be searched
///
/// @return returns true if str begins with startm false otherwise
// --------------------------------------------------------------------------
inline bool ends_with(const std::string &str, const std::string &end) {
  if (str.length() < end.length()) return false;
  return str.compare(str.length() - end.length(), end.length(), end) == 0;
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
bool u8_validate(const char *);
bool u8_validate(const uint8_t *, size_t);
// Saves number of EGC/Characters in string src, to count, return false if string is not valid utf8.
bool u8_egc_count(const char *src, size_t *count);
UChar32 u8_egc_index_as_single_codepoint(const char *str, int32_t index);
void u8_copy_and_replace_codepoint_at(const char *src, char *dst, int32_t index, UChar32 c);
int32_t u8_egc_index_to_offset(const char *src, int32_t index);
int32_t u8_strncpy(uint8_t *, const uint8_t *, const int32_t);
size_t u8_truncate(const uint8_t *, size_t);
// Return display width for string piece, len could be -1 for NULL terminated string.
size_t u8_width(const char *src, int len);
void u8_truncate_below_width(const char *src, size_t len, size_t max_width, bool break_for_line,
                             bool always_break_before_newline, size_t *out_len, size_t *out_width);
std::string u8_sanitize(std::string_view src);
int u8_egc_find(const char *haystack, size_t haystack_len, const char *needle, size_t needle_len,
                bool reverse);

#endif  // STRUTILS_H
