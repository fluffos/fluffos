// This file implements save_* and restore_* efuns

#include "base/package_api.h"

#ifdef F_SAVE_OBJECT
Value f_save_object(EfunArgs&, std::optional<Arg> target, std::optional<LPC_INT> flag_arg) {
  const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  int flag = 0;
  if (flag_arg) {
    flag = static_cast<int>(*flag_arg);
    if (!target->is(T_STRING)) {
      error("first argument must be a string for save_object with 2 args");
    }
  }

  if (target && !flag_arg && target->is(T_NUMBER)) {
    flag = static_cast<int>(target->number());
  }

  if (target && target->is(T_STRING)) {
    flag = save_object(current_object, target->string().c_str(), flag);
    return flag;
  }
  char* saved = new_string(max_string_length, "save_object_str");
  /* Hold the scratch buffer in a Value so an error() in the save frees it. */
  Value scratch = Value::str_malloced(saved);
  flag = save_object_str(current_object, flag, saved, max_string_length);
  if (flag) {
    return Value::str_malloced(string_copy(saved, "save_object_str2"));
  }
  return Value::undefined();
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
Value f_save_variable(EfunArgs&, Arg value) {
  char* p = save_variable(value.sv());
  return Value::str_malloced(p);
}
#endif

#ifdef F_RESTORE_OBJECT
Value f_restore_object(EfunArgs&, StringArg file, std::optional<LPC_INT> flag_arg) {
  int flag = flag_arg ? static_cast<int>(*flag_arg) : 0;
  flag = restore_object(current_object, file.c_str(), flag);
  return flag;
}
#endif

#ifdef F_RESTORE_VARIABLE
Value f_restore_variable(EfunArgs& f, StringArg saved) {
  svalue_t v;

  /* restore_variable() edits the text in place: give the argument's slot a
   * private copy first. The slot keeps owning it, so it is released after
   * the restore, or by an error() in it. */
  unlink_string_svalue(f[0].sv());
  v.type = T_NUMBER;

  // unlinked string
  restore_variable(&v, const_cast<char*>(saved.c_str()));
  return Value::owned(v);
}
#endif
