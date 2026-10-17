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

void f_exec(int num_arg) {
  int i;

  i = replace_interactive((vm_sp() - 1)->u.ob, vm_sp()->u.ob);

  /* They might have been destructed */
  if (vm_sp()->type == T_OBJECT) {
    free_object(&vm_sp()->u.ob, "f_exec:1");
  }
  if ((--vm_sp())->type == T_OBJECT) {
    free_object(&vm_sp()->u.ob, "f_exec:2");
  }
  put_number(i);
}
#endif
