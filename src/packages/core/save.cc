// This file implements save_* and restore_* efuns

#include "base/package_api.h"

#ifdef F_SAVE_OBJECT
void f_save_object(LPCFrame& f) {
  int num_arg = f.argc();
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int flag = 0;
  if (num_arg == 2) {
    flag = static_cast<int>(f.number(1));
    if (f.type(0) != T_STRING) {
      error("first argument must be a string for save_object with 2 args");
    }
  }

  if (num_arg == 1 && f.type(0) == T_NUMBER) {
    flag = static_cast<int>(f.number(0));
  }

  if (num_arg && f.type(0) == T_STRING) {
    flag = save_object(current_object, f.string(0), flag);
    f.ret(static_cast<LPC_INT>(flag));
  } else {
    char* saved = new_string(max_string_length, "save_object_str");
    /* Keep the scratch buffer on the stack so an error() in the save frees it. */
    LPCCall scratch;
    scratch.push_str_malloced(saved);
    flag = save_object_str(current_object, flag, saved, max_string_length);
    char* result = flag ? string_copy(saved, "save_object_str2") : nullptr;
    scratch.discard();
    if (result) {
      f.ret_str_malloced(result);
    } else {
      f.ret_undefined();
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
void f_save_variable(LPCFrame& f) {
  char* p = save_variable(&f.top());
  f.ret_str_malloced(p);
}
#endif

#ifdef F_RESTORE_OBJECT
void f_restore_object(LPCFrame& f) {
  int num_arg = f.argc();
  int flag = (num_arg > 1) ? static_cast<int>(f.number(1)) : 0;
  if (num_arg > 1) {
    f.pop_back(1);
  }
  flag = restore_object(current_object, f.string(0), flag);
  f.ret(static_cast<LPC_INT>(flag));
}
#endif

#ifdef F_RESTORE_VARIABLE
void f_restore_variable(LPCFrame& f) {
  svalue_t v;

  unlink_string_svalue(&f.top());
  v.type = T_NUMBER;

  // unlinked string
  restore_variable(&v, const_cast<char*>(f.top().u.string));
  FREE_MSTR(f.top().u.string);
  f.top() = v;
}
#endif
