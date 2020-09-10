// Various mud extension protocol using telnet.

#include "base/package_api.h"

#include "thirdparty/libtelnet/libtelnet.h"  // FIXME?

/* TELNET */
#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type() {
  if (command_giver) {
    auto ip = command_giver->interactive;
    if (ip && ip->telnet) {
      telnet_request_ttype(ip->telnet);
      flush_message(ip);
    }
  }
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type() {
  if (command_giver) {
    auto ip = command_giver->interactive;
    if (ip && ip->telnet) {
      telnet_start_request_ttype(ip->telnet);
      flush_message(ip);
    }
  }
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size() {
  if (command_giver) {
    auto ip = command_giver->interactive;

    if (ip && ip->telnet) {
      if ((st_num_arg == 1) && (sp->u.number == 0)) {
        telnet_dont_naws(ip->telnet);
      } else {
        telnet_do_naws(ip->telnet);
      }
      flush_message(ip);
    }
  }
  if (st_num_arg == 1) {
    sp--;
  }
}
#endif

#ifdef F_TELNET_NOP
void f_telnet_nop() {
  if (command_giver) {
    auto ip = command_giver->interactive;

    if (ip && ip->telnet) {
      telnet_send_nop(ip->telnet);
      flush_message(ip);
    }
  }
}
#endif

/* MXP */
#ifdef F_HAS_MXP
void f_has_mxp(void) {
  int i = 0;

  if (sp->u.ob->interactive) {
    i = sp->u.ob->interactive->iflags & USING_MXP;
    i = !!i;  // force 1 or 0
  }
  free_object(&sp->u.ob, "f_has_mxp");
  put_number(i);
}
#endif

#ifdef F_ACT_MXP
void f_act_mxp() {
  if (command_giver) {
    auto ip = command_giver->interactive;

    if (ip && ip->telnet) {
      // start MXP
      telnet_begin_sb(ip->telnet, TELNET_TELOPT_MXP);
    }
  }
}
#endif

/* GMCP */

#ifdef F_HAS_GMCP
void f_has_gmcp() {
  int i = 0;

  if (sp->u.ob->interactive) {
    i = sp->u.ob->interactive->iflags & USING_GMCP;
    i = !!i;  // force 1 or 0
  }
  free_object(&sp->u.ob, "f_has_gmcp");
  put_number(i);
}
#endif

#ifdef F_SEND_GMCP
void f_send_gmcp() {
  if (command_giver) {
    auto ip = command_giver->interactive;

    if (ip && ip->telnet) {
      telnet_subnegotiation(ip->telnet, TELNET_TELOPT_GMCP, sp->u.string, SVALUE_STRLEN(sp));
      flush_message(ip);
    }
  }
  pop_stack();
}
#endif

/* ZMP */

#ifdef F_HAS_ZMP
void f_has_zmp(void) {
  int i = 0;

  if (sp->u.ob->interactive) {
    i = sp->u.ob->interactive->iflags & USING_ZMP;
    i = !!i;  // force 1 or 0
  }
  free_object(&sp->u.ob, "f_has_zmp");
  put_number(i);
}
#endif

#ifdef F_SEND_ZMP
void f_send_zmp() {
  if (command_giver) {
    auto ip = command_giver->interactive;
    if (ip && ip->telnet) {
      telnet_begin_zmp(ip->telnet, (sp - 1)->u.string);

      for (int i = 0; i < sp->u.arr->size; i++) {
        if (sp->u.arr->item[i].type == T_STRING) {
          telnet_zmp_arg(ip->telnet, sp->u.arr->item[i].u.string);
        }
      }

      telnet_finish_zmp(ip->telnet);

      flush_message(ip);
    }
  }
  pop_2_elems();
}
#endif
