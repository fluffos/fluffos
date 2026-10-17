#include "base/package_api.h"

#ifdef F_EXPLODE
void f_explode(int num_arg) {
  array_t* vec;

  int const len = SVALUE_STRLEN(vm_sp() - 1);

  vec = explode_string((vm_sp() - 1)->u.string, len, vm_sp()->u.string, SVALUE_STRLEN(vm_sp()),
                       CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__) != 0);
  free_string_svalue(vm_sp()--);
  free_string_svalue(vm_sp());
  put_array(vec);
}
#endif

#ifdef F_EXPLODE_REVERSIBLE
void f_explode_reversible(int num_arg) {
  array_t* vec;

  int const len = SVALUE_STRLEN(vm_sp() - 1);

  vec = explode_string((vm_sp() - 1)->u.string, len, vm_sp()->u.string, SVALUE_STRLEN(vm_sp()), true);
  free_string_svalue(vm_sp()--);
  free_string_svalue(vm_sp());
  put_array(vec);
}
#endif

#ifdef F_IMPLODE
void f_implode(int num_arg) {
  array_t* arr;
  int flag;
  svalue_t* args;

  if (num_arg == 3) {
    args = (vm_sp() - 2);
    if (args[1].type == T_STRING) {
      error(
          "Third argument to implode() is illegal with implode(array, "
          "string)\n");
    }
    flag = 1;
  } else {
    args = (vm_sp() - 1);
    flag = 0;
  }
  arr = args->u.arr;
  check_for_destr(arr);

  if (args[1].type == T_STRING) {
    /* num_arg == 2 here */
    char* str;

    str = implode_string(arr, vm_sp()->u.string, SVALUE_STRLEN(vm_sp()));
    free_string_svalue(vm_sp()--);
    free_array(arr);
    put_malloced_string(str);
  } else { /* function */
    funptr_t* funp = args[1].u.fp;

    /* this pulls the extra arg off the stack if it exists */
    implode_array(funp, arr, args, flag);
    pop_stack();
  }
}
#endif
