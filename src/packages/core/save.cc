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
    char *saved = new_string(max_string_length, "save_object_str");
    push_malloced_string(saved);
    int left = max_string_length;
    flag = save_object_str(current_object, flag, saved, left);
    if (!flag) {
      pop_stack();
      push_undefined();
    } else {
      saved = new_string(strlen(sp->u.string), "save_object_str2");
      strcpy(saved, sp->u.string);
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
char *save_variable(svalue_t *var) {
  int theSize;
  char *new_str, *p;

  save_svalue_depth = 0;
  theSize = svalue_save_size(var);
  new_str = new_string(theSize - 1, "save_variable");
  *new_str = '\0';
  p = new_str;
  save_svalue(var, &p);
  DEBUG_CHECK(p - new_str != theSize - 1, "Length miscalculated in save_variable");
  return new_str;
}

#ifdef F_SAVE_VARIABLE
void f_save_variable(void) {
  char *p;

  p = save_variable(sp);
  pop_stack();
  push_malloced_string(p);
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

  unlink_string_svalue(sp);
  v.type = T_NUMBER;

  // unlinked string
  restore_variable(&v, const_cast<char *>(sp->u.string));
  FREE_MSTR(sp->u.string);
  *sp = v;
}
#endif
