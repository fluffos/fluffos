#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(
      std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
      s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

inline bool starts_with(const std::string &big_str, const std::string &small_str) {
  return big_str.compare(0, small_str.length(), small_str) == 0;
}

inline bool ends_with(const std::string &str, const std::string &ending) {
  if (str.length() < ending.length()) return false;
  return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}
