// Various mud extension protocol using telnet.

#include "base/package_api.h"

#include "thirdparty/libtelnet/libtelnet.h"  // FIXME?

/* TELNET */
#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_request_ttype(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. request_term_type() should only be called by a user object.\n");
  }
  f.done();
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = command_giver ? command_giver->interactive : nullptr;
  if (ip && ip->telnet) {
    telnet_start_request_ttype(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. start_request_term_type() should only be called by a user "
        "object.\n");
  }
  f.done();
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;

  if (ip && ip->telnet) {
    if ((num_arg == 1) && (f.number(0) == 0)) {
      telnet_dont_naws(ip->telnet);
    } else {
      telnet_do_naws(ip->telnet);
    }
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. request_term_size() should only be called by a user object.\n");
  }
  f.ret_void();
}
#endif

#ifdef F_TELNET_NOP
void f_telnet_nop(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_nop(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. telnet_nop() should only be called by a user object.\n");
  }
  f.done();
}
#endif

#ifdef F_TELNET_GA
void f_telnet_ga(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_ga(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. telnet_ga() should only be called by a user object.\n");
  }
  f.done();
}
#endif

/* MXP */
#ifdef F_HAS_MXP
void f_has_mxp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.object(0)->interactive) {
    i = f.object(0)->interactive->iflags & USING_MXP;
    i = !!i;  // force 1 or 0
  }
  f.ret(i);
}
#endif

#ifdef F_ACT_MXP
void f_act_mxp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    // start MXP
    telnet_begin_sb(ip->telnet, TELNET_TELOPT_MXP);
  } else if (!ip) {
    debug_message("Warning: wrong usage. act_mxp() should only be called by a user object.\n");
  }
  f.done();
}
#endif

/* GMCP */

#ifdef F_HAS_GMCP
void f_has_gmcp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.object(0)->interactive) {
    i = f.object(0)->interactive->iflags & USING_GMCP;
    i = !!i;  // force 1 or 0
  }
  f.ret(i);
}
#endif

#ifdef F_SEND_GMCP
void f_send_gmcp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  const auto* data = f.string(0);
  auto len = f.string_len(0);
  if (ip && ip->telnet) {
    std::string const transdata = u8_convert_encoding(ip->trans, data, len);
    std::string_view const result = transdata.empty() ? std::string_view(data, len) : transdata;
    telnet_subnegotiation(ip->telnet, TELNET_TELOPT_GMCP, result.data(), result.size());
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. send_gmcp() should only be called by a user object.\n");
  }
  f.ret_void();
}
#endif

/* MSP */

#ifdef F_HAS_MSP
void f_has_msp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.object(0)->interactive) {
    i = f.object(0)->interactive->iflags & USING_MSP;
    i = !!i;  // force 1 or 0
  }
  f.ret(i);
}
#endif

/* MSDP */

#ifdef F_HAS_MSDP
void f_has_msdp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.object(0)->interactive) {
    i = f.object(0)->interactive->iflags & USING_MSDP;
    i = !!i;  // force 1 or 0
  }
  f.ret(i);
}
#endif

#ifdef F_SEND_MSDP_VARIABLE
/** TODO update to support sending other value type and use proper MSDP mapping/array formats as
 * needed based on data types */
void f_send_msdp_variable(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    switch (f.type(1)) {
      case T_STRING:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02", f.string(0));
        telnet_send(ip->telnet, f.string(1), f.string_len(1));
        telnet_finish_sb((ip->telnet));
        break;
      case T_NUMBER:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02%lu", f.string(0), f.number(1));
        telnet_finish_sb((ip->telnet));
        break;
      case T_REAL:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02%f", f.string(0), f.real(1));
        telnet_finish_sb((ip->telnet));
        break;
      case T_BUFFER:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02", f.string(0));
        telnet_send(ip->telnet, reinterpret_cast<char*>(f.buffer(1)->item), f.buffer(1)->size);
        telnet_finish_sb((ip->telnet));
        break;
      case T_ARRAY:
      case T_MAPPING:
      default:
        error("Bad argument 2 to send_msdp_variable()\n");
        break;
    }
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. send_msdp_variable() should only be called by a user object.\n");
  }
  f.ret_void();
}
#endif

/* ZMP */

#ifdef F_HAS_ZMP
void f_has_zmp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  int i = 0;

  if (f.object(0)->interactive) {
    i = f.object(0)->interactive->iflags & USING_ZMP;
    i = !!i;  // force 1 or 0
  }
  f.ret(i);
}
#endif

#ifdef F_SEND_ZMP
void f_send_zmp(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = command_giver ? command_giver->interactive : nullptr;
  if (ip && ip->telnet) {
    telnet_begin_zmp(ip->telnet, f.string(0));

    for (int i = 0; i < f.array(1)->size; i++) {
      if (f.array(1)->item[i].type == T_STRING) {
        telnet_zmp_arg(ip->telnet, f.array(1)->item[i].u.string);
      }
    }

    telnet_finish_zmp(ip->telnet);

    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. send_zmp() should only be called by a user object.\n");
  }
  f.ret_void();
}
#endif

#ifdef F_TELNET_MSP_OOB
void f_telnet_msp_oob(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_msp_oob(ip, f.string(0), f.string_len(0));
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. telnet_msp_oob() should only be called by a user object.\n");
  }
  f.ret_void();
}
#endif
