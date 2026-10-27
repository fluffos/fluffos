/*
 Author: Zoilder (RL-Mud)
 Date: 2013 May 09
 Description: String triming utils.

 Modified by sunyucong@gmail.com.
 */

#include "base/package_api.h"

#include <string_view>

namespace {

/* trim()/ltrim()/rtrim(): `chars` is a set of Unicode scalar values (issue
 * #1401), whitespace when absent or empty. A string with nothing to trim is
 * returned as is; otherwise the kept range is copied once. */
vm::Value trim_impl(vm::EfunArgs& f, bool left, bool right, vm::StringArg str,
                    std::optional<vm::StringArg> chars) {
  std::string_view const set =
      chars && !chars->empty() ? chars->view() : std::string_view("\t\n\v\f\r ");
  auto const [first, last] = u8_trim_range(str.view(), set, left, right);

  if (first == 0 && last == str.size()) {
    return f.take(0);
  }
  if (first == last) {
    return vm::Value::str_make_shared("");
  }
  return vm::Value::str_copy(str.view().substr(first, last - first));
}

}  // namespace

#ifdef F_TRIM
vm::Value f_trim(vm::EfunArgs& f, vm::StringArg str, std::optional<vm::StringArg> chars) {
  return trim_impl(f, true, true, str, chars);
}
#endif

#ifdef F_LTRIM
vm::Value f_ltrim(vm::EfunArgs& f, vm::StringArg str, std::optional<vm::StringArg> chars) {
  return trim_impl(f, true, false, str, chars);
}
#endif

#ifdef F_RTRIM
vm::Value f_rtrim(vm::EfunArgs& f, vm::StringArg str, std::optional<vm::StringArg> chars) {
  return trim_impl(f, false, true, str, chars);
}
#endif
