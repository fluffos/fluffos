#include "base/package_api.h"

#ifdef F_EXPLODE
Value f_explode(EfunArgs&, StringArg str, StringArg delimiter) {
  return Value::refed(explode_string(str.c_str(), str.size(), delimiter.c_str(), delimiter.size(),
                                     CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0));
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
Value f_explode_reversible(EfunArgs&, StringArg str, StringArg delimiter) {
  return Value::refed(
      explode_string(str.c_str(), str.size(), delimiter.c_str(), delimiter.size(), true));
}
#endif

#ifdef F_IMPLODE
Value f_implode(EfunArgs& f, array_t* arr, Arg glue, std::optional<Arg> start) {
  if (start && glue.is(T_STRING)) {
    error("Third argument to implode() is illegal with implode(array, string)\n");
  }
  check_for_destr(arr);

  if (glue.is(T_STRING)) {
    StringArg str = glue.string();
    return Value::str_malloced(implode_string(arr, str.c_str(), str.size()));
  }
  /* implode_array() writes the result into slot 0, taking over the array's
   * reference, and consumes the starting value itself: what is left is
   * [result, function]. */
  implode_array(glue.funptr(), arr, f[0].sv(), start.has_value());
  f.resync();
  return f.take(0);
}
#endif
