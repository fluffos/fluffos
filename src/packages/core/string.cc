#include "base/package_api.h"

#ifdef F_EXPLODE
void f_explode(LPCFrame& f) {
  array_t* vec = explode_string(f.string(0), f.string_len(0), f.string(1), f.string_len(1),
                                CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0);
  f.ret_refed(vec);
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
void f_explode_reversible(LPCFrame& f) {
  array_t* vec =
      explode_string(f.string(0), f.string_len(0), f.string(1), f.string_len(1), true);
  f.ret_refed(vec);
}
#endif

#ifdef F_IMPLODE
void f_implode(LPCFrame& f) {
  int const num_arg = f.argc();
  if (num_arg == 3 && f.type(1) == T_STRING) {
    error("Third argument to implode() is illegal with implode(array, string)\n");
  }
  array_t* arr = f.array(0);
  check_for_destr(arr);

  if (f.type(1) == T_STRING) {
    char* str = implode_string(arr, f.string(1), f.string_len(1));
    f.ret_str_malloced(str);
  } else { /* function */
    /* implode_array() writes the result into slot 0, taking over the
     * array's reference, and consumes the optional third argument (the
     * starting value) itself. What is left is [result, function]. */
    implode_array(f.funptr(1), arr, f.argp(0), num_arg == 3);
    f.rebase(2);
    f.pop_back();
  }
}
#endif
