// Various mud extension protocol using telnet.

#include "base/package_api.h"

#include "packages/core/telnet_ext.h"

#include "thirdparty/libtelnet/libtelnet.h"  // FIXME?

/* MXP */

#ifdef F_ACT_MXP
void f_act_mxp() {
  // start MXP
  auto ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_begin_sb(ip->telnet, TELNET_TELOPT_MXP);
  }
}
#endif

/* GMCP */

void on_telnet_do_gmcp(interactive_t* ip) {
  ip->iflags |= USING_GMCP;
  apply(APPLY_GMCP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
}

#ifdef F_SEND_GMCP
void f_send_gmcp() {
  auto ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_subnegotiation(ip->telnet, TELNET_TELOPT_GMCP, sp->u.string, SVALUE_STRLEN(sp));
    flush_message(ip);
  }
  pop_stack();
}
#endif

/* ZMP */

void on_telnet_do_zmp(const char** argv, unsigned long argc, interactive_t* ip) {
  ip->iflags |= USING_ZMP;

  // Push the command
  copy_and_push_string(argv[0]);

  // Push the array
  array_t* arr = allocate_array(argc - 1);
  for (int i = 1; i < argc; i++) {
    arr->item[i].u.string = string_copy(argv[i], "ZMP");
    arr->item[i].type = T_STRING;
    arr->item[i].subtype = STRING_MALLOC;
  }
  push_refed_array(arr);

  safe_apply(APPLY_ZMP, ip->ob, 2, ORIGIN_DRIVER);
}

#ifdef F_SEND_ZMP
void f_send_zmp() {
  auto ip = current_object->interactive;

  if (ip && ip->telnet) {
    telnet_begin_zmp(ip->telnet, (sp - 1)->u.string);

    for (int i = 0; i < sp->u.arr->size; i++) {
      if (sp->u.arr->item[i].type == T_STRING) {
        telnet_zmp_arg(ip->telnet, sp->u.arr->item[i].u.string);
      }
    }

    telnet_finish_zmp(ip->telnet);

    flush_message(ip);
    pop_2_elems();
  }
}
#endif
