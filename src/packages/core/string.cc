#include "base/package_api.h"

#ifdef F_EXPLODE
vm::Value f_explode(vm::EfunArgs&, vm::StringArg str, vm::StringArg delimiter) {
  return vm::Value::refed(explode_string(str.c_str(), str.size(), delimiter.c_str(),
                                         delimiter.size(),
                                         CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0));
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
vm::Value f_explode_reversible(vm::EfunArgs&, vm::StringArg str, vm::StringArg delimiter) {
  return vm::Value::refed(
      explode_string(str.c_str(), str.size(), delimiter.c_str(), delimiter.size(), true));
}
#endif

#ifdef F_IMPLODE
vm::Value f_implode(vm::EfunArgs& f, array_t* arr, vm::Arg glue, std::optional<vm::Arg> start) {
  if (start && glue.is(T_STRING)) {
    error("Third argument to implode() is illegal with implode(array, string)\n");
  }
  check_for_destr(arr);

  if (glue.is(T_STRING)) {
    vm::StringArg str = glue.string();
    return vm::Value::str_malloced(implode_string(arr, str.c_str(), str.size()));
  }
  /* implode_array() writes the result into slot 0, taking over the array's
   * reference, and consumes the starting value itself: what is left is
   * [result, function]. */
  implode_array(glue.funptr(), arr, f[0].sv(), start.has_value());
  f.resync();
  return f.take(0);
}
#endif
