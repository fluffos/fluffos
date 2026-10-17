// This file implements save_* and restore_* efuns

#include "base/package_api.h"

#ifdef F_SAVE_OBJECT
void f_save_object(int num_arg) {
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int flag;
  if (num_arg == 2) {
    flag = (vm_sp()--)->u.number;
    if (vm_sp()->type != T_STRING) {
      error("first argument must be a string for save_object with 2 args");
    }
  } else {
    flag = 0;
  }

  if (num_arg == 1 && vm_sp()->type == T_NUMBER) {
    flag = vm_sp()->u.number;
  }

  if (num_arg && vm_sp()->type == T_STRING) {
    flag = save_object(current_object, vm_sp()->u.string, flag);

    free_string_svalue(vm_sp());
    put_number(flag);
  } else {
    pop_n_elems(num_arg);
    char* saved = new_string(max_string_length, "save_object_str");
    push_malloced_string(saved);
    int const left = max_string_length;
    flag = save_object_str(current_object, flag, saved, left);
    if (!flag) {
      pop_stack();
      push_undefined();
    } else {
      saved = new_string(strlen(vm_sp()->u.string), "save_object_str2");
      strcpy(saved, vm_sp()->u.string);
      pop_stack();
      push_malloced_string(saved);
    }
  }
}
#endif

/*
 * return a string representing an svalue in the form that save_object()
 * would write it.
 */
char* save_variable(svalue_t* var) {
  int the_size;
  char *new_str, *p;

  save_svalue_depth = 0;
  the_size = svalue_save_size(var);
  new_str = new_string(the_size - 1, "save_variable");
  *new_str = '\0';
  p = new_str;
  save_svalue(var, &p);
  DEBUG_CHECK(p - new_str != the_size - 1, "Length miscalculated in save_variable");
  return new_str;
}

#ifdef F_SAVE_VARIABLE
void f_save_variable(int num_arg) {
  char* p;

  p = save_variable(vm_sp());
  pop_stack();
  push_malloced_string(p);
}
#endif

#ifdef F_RESTORE_OBJECT
void f_restore_object(int num_arg) {
  int flag;

  flag = (num_arg > 1) ? (vm_sp()--)->u.number : 0;

  flag = restore_object(current_object, vm_sp()->u.string, flag);

  free_string_svalue(vm_sp());
  put_number(flag);
}
#endif

#ifdef F_RESTORE_VARIABLE
void f_restore_variable(int num_arg) {
  svalue_t v;

  unlink_string_svalue(vm_sp());
  v.type = T_NUMBER;

  // unlinked string
  restore_variable(&v, const_cast<char*>(vm_sp()->u.string));
  FREE_MSTR(vm_sp()->u.string);
  *vm_sp() = v;
}
#endif
