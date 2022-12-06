#include "base/package_api.h"

#ifdef F_EXPLODE
void f_explode() {
  array_t *vec;

  int const len = SVALUE_STRLEN(sp - 1);

  vec = explode_string((sp - 1)->u.string, len, sp->u.string, SVALUE_STRLEN(sp),
                       CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0);
  free_string_svalue(sp--);
  free_string_svalue(sp);
  put_array(vec);
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
void f_explode_reversible() {
  array_t *vec;

  int const len = SVALUE_STRLEN(sp - 1);

  vec = explode_string((sp - 1)->u.string, len, sp->u.string, SVALUE_STRLEN(sp), true);
  free_string_svalue(sp--);
  free_string_svalue(sp);
  put_array(vec);
}
#endif

#ifdef F_IMPLODE
void f_implode() {
  array_t *arr;
  int flag;
  svalue_t *args;

  if (st_num_arg == 3) {
    args = (sp - 2);
    if (args[1].type == T_STRING) {
      error(
          "Third argument to implode() is illegal with implode(array, "
          "string)\n");
    }
    flag = 1;
  } else {
    args = (sp - 1);
    flag = 0;
  }
  arr = args->u.arr;
  check_for_destr(arr);

  if (args[1].type == T_STRING) {
    /* st_num_arg == 2 here */
    char *str;

    str = implode_string(arr, sp->u.string, SVALUE_STRLEN(sp));
    free_string_svalue(sp--);
    free_array(arr);
    put_malloced_string(str);
  } else { /* function */
    funptr_t *funp = args[1].u.fp;

    /* this pulls the extra arg off the stack if it exists */
    implode_array(funp, arr, args, flag);
    pop_stack();
  }
}
#endif
