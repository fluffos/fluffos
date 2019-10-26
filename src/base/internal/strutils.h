#ifndef STRUTILS_H
#define STRUTILS_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

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

}  // namespace
#endif  // STRUTILS_H