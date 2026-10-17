#include "base/package_api.h"

#ifdef F_EXPLODE
void f_explode(LPCFrame& f) {
  array_t* vec = explode_string(f.string(0), f.string_len(0), f.string(1), f.string_len(1),
                                CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0);
  f.ret(vec);
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
void f_explode_reversible(LPCFrame& f) {
  array_t* vec =
      explode_string(f.string(0), f.string_len(0), f.string(1), f.string_len(1), true);
  f.ret(vec);
}
#endif

#ifdef F_IMPLODE
void f_implode(LPCFrame& f) {
  int num_arg = f.argc();
  array_t* arr = f.array(0);
  check_for_destr(arr);

  if (f.type(1) == T_STRING) {
    /* num_arg == 2 here */
    char* str = implode_string(arr, f.string(1), f.string_len(1));
    f.ret_str_malloced(str);
  } else { /* function */
    funptr_t* funp = f.funptr(1);
    int const flag = (num_arg == 3) ? 1 : 0;

    /* this pulls the extra arg off the stack if it exists */
    implode_array(funp, arr, f.argp(0), flag);
    /* implode_array may have consumed the optional 3rd arg; leave the
     * result in slot 0 and drop the function pointer still on top. */
    f.done();
    LPCFrame drop_fun = LPCFrame::args(1);
    drop_fun.ret_void();
  }
}
#endif
