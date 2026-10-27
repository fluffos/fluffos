// Various mud extension protocol using telnet.

#include "base/package_api.h"

#include "thirdparty/libtelnet/libtelnet.h"  // FIXME?

/* TELNET */
#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type(vm::EfunArgs&) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_request_ttype(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. request_term_type() should only be called by a user object.\n");
  }
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type(vm::EfunArgs&) {
  auto* ip = command_giver ? command_giver->interactive : nullptr;
  if (ip && ip->telnet) {
    telnet_start_request_ttype(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. start_request_term_type() should only be called by a user "
        "object.\n");
  }
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size(vm::EfunArgs&, std::optional<LPC_INT> enable) {
  auto* ip = current_object->interactive;

  if (ip && ip->telnet) {
    if (enable && *enable == 0) {
      telnet_dont_naws(ip->telnet);
    } else {
      telnet_do_naws(ip->telnet);
    }
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. request_term_size() should only be called by a user object.\n");
  }
}
#endif

#ifdef F_TELNET_NOP
void f_telnet_nop(vm::EfunArgs&) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_nop(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. telnet_nop() should only be called by a user object.\n");
  }
}
#endif

#ifdef F_TELNET_GA
void f_telnet_ga(vm::EfunArgs&) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_ga(ip->telnet);
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. telnet_ga() should only be called by a user object.\n");
  }
}
#endif

/* MXP */
#ifdef F_HAS_MXP
vm::Value f_has_mxp(vm::EfunArgs&, object_t* ob) {
  int i = 0;

  if (ob->interactive) {
    i = ob->interactive->iflags & USING_MXP;
    i = !!i;  // force 1 or 0
  }
  return i;
}
#endif

#ifdef F_ACT_MXP
void f_act_mxp(vm::EfunArgs&) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    // start MXP
    telnet_begin_sb(ip->telnet, TELNET_TELOPT_MXP);
  } else if (!ip) {
    debug_message("Warning: wrong usage. act_mxp() should only be called by a user object.\n");
  }
}
#endif

/* GMCP */

#ifdef F_HAS_GMCP
vm::Value f_has_gmcp(vm::EfunArgs&, object_t* ob) {
  int i = 0;

  if (ob->interactive) {
    i = ob->interactive->iflags & USING_GMCP;
    i = !!i;  // force 1 or 0
  }
  return i;
}
#endif

#ifdef F_SEND_GMCP
void f_send_gmcp(vm::EfunArgs&, vm::StringArg message) {
  auto* ip = current_object->interactive;
  const auto* data = message.c_str();
  auto len = message.size();
  if (ip && ip->telnet) {
    std::string const transdata = u8_convert_encoding(ip->trans, data, len);
    std::string_view const result = transdata.empty() ? std::string_view(data, len) : transdata;
    telnet_subnegotiation(ip->telnet, TELNET_TELOPT_GMCP, result.data(), result.size());
    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. send_gmcp() should only be called by a user object.\n");
  }
}
#endif

/* MSP */

#ifdef F_HAS_MSP
vm::Value f_has_msp(vm::EfunArgs&, object_t* ob) {
  int i = 0;

  if (ob->interactive) {
    i = ob->interactive->iflags & USING_MSP;
    i = !!i;  // force 1 or 0
  }
  return i;
}
#endif

/* MSDP */

#ifdef F_HAS_MSDP
vm::Value f_has_msdp(vm::EfunArgs&, object_t* ob) {
  int i = 0;

  if (ob->interactive) {
    i = ob->interactive->iflags & USING_MSDP;
    i = !!i;  // force 1 or 0
  }
  return i;
}
#endif

#ifdef F_SEND_MSDP_VARIABLE
/** TODO update to support sending other value type and use proper MSDP mapping/array formats as
 * needed based on data types */
void f_send_msdp_variable(vm::EfunArgs&, vm::StringArg name, vm::Arg value) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    switch (value.type()) {
      case T_STRING: {
        vm::StringArg str = value.string();
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02", name.c_str());
        telnet_send(ip->telnet, str.c_str(), str.size());
        telnet_finish_sb((ip->telnet));
        break;
      }
      case T_NUMBER:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02%" LPC_INT_FMTSTR_P, name.c_str(), value.number());
        telnet_finish_sb((ip->telnet));
        break;
      case T_REAL:
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02%f", name.c_str(), value.real());
        telnet_finish_sb((ip->telnet));
        break;
      case T_BUFFER: {
        buffer_t* buf = value.buffer();
        telnet_begin_sb(ip->telnet, TELNET_TELOPT_MSDP);
        telnet_printf(ip->telnet, "\x01%s\x02", name.c_str());
        telnet_send(ip->telnet, reinterpret_cast<char*>(buf->item), buf->size);
        telnet_finish_sb((ip->telnet));
        break;
      }
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
}
#endif

/* ZMP */

#ifdef F_HAS_ZMP
vm::Value f_has_zmp(vm::EfunArgs&, object_t* ob) {
  int i = 0;

  if (ob->interactive) {
    i = ob->interactive->iflags & USING_ZMP;
    i = !!i;  // force 1 or 0
  }
  return i;
}
#endif

#ifdef F_SEND_ZMP
void f_send_zmp(vm::EfunArgs&, vm::StringArg command, array_t* args) {
  auto* ip = command_giver ? command_giver->interactive : nullptr;
  if (ip && ip->telnet) {
    telnet_begin_zmp(ip->telnet, command.c_str());

    for (int i = 0; i < args->size; i++) {
      if (args->item[i].type == T_STRING) {
        telnet_zmp_arg(ip->telnet, args->item[i].u.string);
      }
    }

    telnet_finish_zmp(ip->telnet);

    flush_message(ip);
  } else if (!ip) {
    debug_message("Warning: wrong usage. send_zmp() should only be called by a user object.\n");
  }
}
#endif

#ifdef F_TELNET_MSP_OOB
void f_telnet_msp_oob(vm::EfunArgs&, vm::StringArg message) {
  auto* ip = current_object->interactive;
  if (ip && ip->telnet) {
    telnet_send_msp_oob(ip, message.c_str(), message.size());
    flush_message(ip);
  } else if (!ip) {
    debug_message(
        "Warning: wrong usage. telnet_msp_oob() should only be called by a user object.\n");
  }
}
#endif
