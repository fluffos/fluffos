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

namespace {

// trim from start
inline std::string &_ltrim(std::string &s, const std::string &charset) {
  std::string trimset{charset};

  if (trimset.empty()) {
    trimset = "\t\n\v\f\r ";
  }
  s = ltrim(s, trimset);
  return s;
}

// trim from end
inline std::string &_rtrim(std::string &s, const std::string &charset) {
  std::string trimset{charset};

  if (trimset.empty()) {
    trimset = "\t\n\v\f\r ";
  }
  s = rtrim(s, trimset);
  return s;
}

// trim from both ends
inline std::string &_trim(std::string &s, const std::string &charset) {
  std::string trimset{charset};

  if (trimset.empty()) {
    trimset = "\t\n\v\f\r ";
  }
  s = trim(s, trimset);
  return s;
}

typedef std::string &(trim_func)(std::string &, const std::string &);

inline void _trim_impl(trim_func *func) {
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

}  // namespace

#ifdef F_TRIM
void f_trim(void) { _trim_impl(&_trim); }
#endif

#ifdef F_LTRIM
void f_ltrim(void) { _trim_impl(&_ltrim); }
#endif

#ifdef F_RTRIM
void f_rtrim(void) { _trim_impl(&_rtrim); }
#endif
