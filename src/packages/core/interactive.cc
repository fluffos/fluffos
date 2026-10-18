#include "base/package_api.h"

#ifdef F_EXEC
int replace_interactive(object_t* ob, object_t* obfrom) {
  if (ob->interactive) {
    error("Bad argument 1 to exec()\n");
  }
  if (!obfrom->interactive) {
    error("Bad argument 2 to exec()\n");
  }
  ob->interactive = obfrom->interactive;
  /*
   * assume the existance of write_prompt and process_input in user.c until
   * proven wrong (after trying to call them).
   */
  ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);
  obfrom->interactive = nullptr;
  ob->interactive->ob = ob;
  ob->flags |= O_ONCE_INTERACTIVE;
  obfrom->flags &= ~O_ONCE_INTERACTIVE;
  add_ref(ob, "exec");
  if (obfrom == command_giver) {
    set_command_giver(ob);
  }

  free_object(&obfrom, "exec");
  return (1);
} /* replace_interactive() */

Value f_exec(EfunArgs&, object_t* to, object_t* from) {
  /* The objects may have been destructed meanwhile: their slots are released
   * with free_svalue, which copes with whatever remains. */
  return replace_interactive(to, from);
}
#endif
