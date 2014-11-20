/*
 Author: Zoilder (RL-Mud)
 Date: 2013 May 09
 Description: String triming utils.

 Modified by sunyucong@gmail.com.
 */

#include "base/package_api.h"

#include <algorithm>  // std::find_if
#include <cctype>
#include <functional>
#include <locale>
#include <string>

// trim from start
static inline std::string &ltrim(std::string &s, const std::string &charset) {
  if (charset.empty()) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  } else {
    int pos = s.find_first_not_of(charset);
    if (pos == -1)
      s.clear();
    else
      s.erase(s.begin(), s.begin() + pos);
  }
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s, const std::string &charset) {
  if (charset.empty()) {
    s.erase(
        std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
        s.end());
  } else {
    s.erase(s.find_last_not_of(charset) + 1);
  }
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s, const std::string &charset) {
  return ltrim(rtrim(s, charset), charset);
}

typedef std::string &(trim_func)(std::string &, const std::string &);

static inline void _trim_impl(trim_func *func) {
  std::string charset;

  // If use 2 arguments, we get characters to remove and positioning in previous
  // parameter.
  if (st_num_arg == 2) {
    charset = sp->u.string;
    pop_stack();
  }

  std::string target(sp->u.string);
  pop_stack();

  target = (*func)(target, charset);
  if (target.empty()) {
    share_and_push_string("");
  } else {
    copy_and_push_string(target.c_str());
  }
}

#ifdef F_TRIM
void f_trim(void) { _trim_impl(&trim); }
#endif

#ifdef F_LTRIM
void f_ltrim(void) { _trim_impl(&ltrim); }
#endif

#ifdef F_RTRIM
void f_rtrim(void) { _trim_impl(&rtrim); }
#endif
