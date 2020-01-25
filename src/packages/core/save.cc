// This file implements save_* and restore_* efuns

#include "base/package_api.h"

#ifdef F_SAVE_OBJECT
void f_save_object(void) {
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int flag;
  if (st_num_arg == 2) {
    flag = (sp--)->u.number;
    if (sp->type != T_STRING) {
      error("first argument must be a string for save_object with 2 args");
    }
  } else {
    flag = 0;
  }

  if (st_num_arg == 1 && sp->type == T_NUMBER) {
    flag = sp->u.number;
  }

  if (st_num_arg && sp->type == T_STRING) {
    flag = save_object(current_object, sp->u.string, flag);

    free_string_svalue(sp);
    put_number(flag);
  } else {
    pop_n_elems(st_num_arg);
    std::string saved {};
    push_string(saved);
    int left = max_string_length;
    flag = save_object_str(current_object, flag, saved, left);
    if (!flag) {
      pop_stack();
      push_undefined();
    } else {
      saved = sp->u.string;
      pop_stack();
      push_string(saved);
    }
  }
}
#endif

#ifdef F_SAVE_VARIABLE
/*
 * return a string representing an svalue in the form that save_object()
 * would write it.
 */
std::string save_variable(svalue_t *var) {
  int theSize;
  std::unique_ptr<char> new_str;
  char *p;

  save_svalue_depth = 0;
  theSize = svalue_save_size(var);
  new_str.reset(new char[theSize - 1]);
  *new_str = '\0';
  p = new_str.get();
  save_svalue(var, &p);
  DEBUG_CHECK(p - new_str.get() != theSize - 1, "Length miscalculated in save_variable");
  return std::string {new_str.get()};
}

void f_save_variable(void) {
    std::string p;

    p = save_variable(sp);
    pop_stack();
    push_string(p);
}
#endif

#ifdef F_RESTORE_OBJECT
void f_restore_object(void) {
  int flag;

  flag = (st_num_arg > 1) ? (sp--)->u.number : 0;

  flag = restore_object(current_object, sp->u.string, flag);

  free_string_svalue(sp);
  put_number(flag);
}
#endif

#ifdef F_RESTORE_VARIABLE
void f_restore_variable(void) {
  svalue_t v;

  v.type = T_NUMBER;

  restore_variable(&v, sp->u.string);
  assign_svalue(sp, &v);
}
#endif
