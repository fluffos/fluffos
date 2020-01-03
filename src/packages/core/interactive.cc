#include "base/package_api.h"

#ifdef F_EXEC
int replace_interactive(object_t *ob, object_t *obfrom) {
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

void f_exec(void) {
  int i;

  i = replace_interactive((sp - 1)->u.ob, sp->u.ob);

  /* They might have been destructed */
  if (sp->type == T_OBJECT) {
    free_object(&sp->u.ob, "f_exec:1");
  }
  if ((--sp)->type == T_OBJECT) {
    free_object(&sp->u.ob, "f_exec:2");
  }
  put_number(i);
}
#endif

#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type() {
  auto ip = command_giver->interactive;
  if (ip->telnet) {
    telnet_request_ttype(ip->telnet);
    flush_message(ip);
  }
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type() {
  auto ip = command_giver->interactive;
  if (ip->telnet) {
    telnet_start_request_ttype(ip->telnet);
    flush_message(ip);
  }
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size() {
  auto ip = command_giver->interactive;

  if (ip->telnet) {
    if ((st_num_arg == 1) && (sp->u.number == 0)) {
      telnet_dont_naws(ip->telnet);
    } else {
      telnet_do_naws(ip->telnet);
    }
    flush_message(ip);
  }

  if (st_num_arg == 1) {
    sp--;
  }
}
#endif
