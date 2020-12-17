#include "base/std.h"

#include "net/telnet.h"
#include "net/sys_telnet.h"  // our own version of telnet header.

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <string>
#include <unicode/ucnv.h>

#include "comm.h"
#include "interactive.h"
#include "packages/core/mssp.h"
#include "thirdparty/libtelnet/libtelnet.h"  // for telnet_t, telnet_event_t*
#include "vm/vm.h"

static const telnet_telopt_t my_telopts[] = {{TELNET_TELOPT_TM, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_SGA, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_NAWS, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_LINEMODE, TELNET_WONT, TELNET_DO},
                                             {TELNET_TELOPT_ECHO, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_TTYPE, TELNET_WONT, TELNET_DO},
                                             {TELNET_TELOPT_NEW_ENVIRON, TELNET_WONT, TELNET_DO},
                                             {TELNET_TELOPT_COMPRESS2, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_ZMP, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_MSSP, TELNET_WILL, TELNET_DO},
                                             {TELNET_TELOPT_GMCP, TELNET_WILL, TELNET_DO},
                                             {-1, 0, 0}};

// Telnet event handler
static void telnet_event_handler(telnet_t * /*telnet*/, telnet_event_t * /*ev*/,
                                 void * /*user_data*/);

struct telnet_t *net_telnet_init(interactive_t *user) {
  return telnet_init(my_telopts, telnet_event_handler, 0, user);
}

static inline void on_telnet_data(const char *buffer, unsigned long size, interactive_t *ip) {
  char *transdata = const_cast<char *>(buffer);
  auto translen = size;

  // Handle charset transcoding
  if (ip->trans) {
    UErrorCode error_code = U_ZERO_ERROR;
    translen = ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, ip->trans, nullptr, 0, buffer, size,
                                  &error_code);
    if (error_code == U_BUFFER_OVERFLOW_ERROR) {
      error_code = U_ZERO_ERROR;
      transdata = (char *)DMALLOC(translen, TAG_TEMPORARY, "on_telnet_data: transcoding");
      auto written = ucnv_toAlgorithmic(UConverterType::UCNV_UTF8, ip->trans, transdata, translen,
                                        buffer, size, &error_code);
      DEBUG_CHECK(written != translen, "Bug: translation buffer size calculation error");
      if (U_FAILURE(error_code)) {
        debug_message("add_message: Translation failed!");
        transdata = const_cast<char *>(buffer);
        translen = size;
      };
    }
  }

  auto sanitized = u8_sanitize({transdata, translen});
  on_user_input(ip, sanitized.c_str(), sanitized.length());

  if (transdata != buffer) {
    FREE(transdata);
  }
}

static inline void on_telnet_send(const char *buffer, unsigned long size, interactive_t *ip) {
  bufferevent_write(ip->ev_buffer, buffer, size);
}

static inline void on_telnet_iac(unsigned char cmd, interactive_t *ip) {
  switch (cmd) {
    case TELNET_BREAK: {
      const char response = 28;
      telnet_send(ip->telnet, &response, sizeof(response));
      telnet_negotiate(ip->telnet, TELNET_WILL, TELNET_TELOPT_TM);
      break;
    }
    case TELNET_IP: { /* interrupt process */
      const char response = 127;
      telnet_send(ip->telnet, &response, sizeof(response));
      telnet_negotiate(ip->telnet, TELNET_WILL, TELNET_TELOPT_TM);
      break;
    }
    case TELNET_AYT: { /* are you there?  you bet */
      const char response[] = {'\n', '[', '-', 'Y', 'e', 's', '-', ']', ' ', '\n'};
      telnet_send(ip->telnet, response, sizeof(response));
      break;
    }
    case TELNET_AO: { /* abort output */
      flush_message(ip);
      // Driver use to send response as OOB, but bufferevent doesn't support it.
      // We just send it non-OOB-ly. It's not like it matters to anyone anymore anyway.
      telnet_iac(ip->telnet, TELNET_DM);
      break;
    }
    default:
      debug(telnet, "on_telnet_iac: unsupported cmd %d.\n", cmd);
      break;
  }
  flush_message(ip);
}

static inline void on_telnet_will(unsigned char cmd, interactive_t *ip) {
  switch (cmd) {
    case TELNET_TELOPT_LINEMODE: {
      ip->iflags |= USING_LINEMODE;
      break;
    }
    case TELNET_TELOPT_ECHO:
    case TELNET_TELOPT_NAWS:
    case TELNET_TELOPT_SGA:
      /* do nothing */
      break;
    case TELNET_TELOPT_TTYPE: {
      telnet_ttype_send(ip->telnet);
      break;
    }
    case TELNET_TELOPT_NEW_ENVIRON: {
      char buf[] = {TELNET_ENVIRON_SEND};
      telnet_subnegotiation(ip->telnet, TELNET_TELOPT_NEW_ENVIRON, buf, sizeof(buf));
      break;
    }
    case TELNET_TELOPT_MXP:
      if (CONFIG_INT(__RC_ENABLE_MXP__)) {
        on_telnet_will_mxp(ip);
      } else {
        telnet_negotiate(ip->telnet, TELNET_DONT, TELNET_TELOPT_MXP);
      }
      break;
    default:
      debug(telnet, "on_telnet_will: unimplemented command %d.\n", cmd);
      break;
  }
  flush_message(ip);
}

static inline void on_telnet_wont(unsigned char cmd, interactive_t *ip) {
  switch (cmd) {
    case TELNET_TELOPT_ECHO:
      // do nothing.
      break;
    case TELNET_TELOPT_LINEMODE:
      /* If we're in single char mode, we just requested for
       * linemode to be disabled, so don't remove our flag.
       */
      if (!(ip->iflags & SINGLE_CHAR)) {
        ip->iflags &= ~USING_LINEMODE;
      }
      break;
    default:
      debug(telnet, "on_telnet_wont: unimplemented command %d.\n", cmd);
      break;
  }
}

static inline void on_telnet_do(unsigned char cmd, interactive_t *ip) {
  switch (cmd) {
    case TELNET_TELOPT_TM:
      telnet_negotiate(ip->telnet, TELNET_WILL, TELNET_TELOPT_TM);
      break;
    case TELNET_TELOPT_ECHO:
      /* do nothing, but don't send a wont response */
      break;
    case TELNET_TELOPT_SGA:
      ip->iflags |= SUPPRESS_GA;
      telnet_negotiate(ip->telnet, TELNET_WILL, TELNET_TELOPT_SGA);
      break;
    case TELNET_TELOPT_GMCP:
      if (!CONFIG_INT(__RC_ENABLE_GMCP__)) {
#ifdef DEBUG
        debug_message(
            "Bad client: bogus IAC DO GMCP from %s.",
            sockaddr_to_string(reinterpret_cast<const sockaddr *>(&ip->addr), ip->addrlen));
        remove_interactive(ip->ob, false);
#else
        // do nothing
#endif
      }
      on_telnet_do_gmcp(ip);
      break;
    case TELNET_TELOPT_MSSP:
      if (!CONFIG_INT(__RC_ENABLE_MSSP__)) {
#ifdef DEBUG
        debug_message(
            "Bad client: bogus IAC DO MSSP from %s.",
            sockaddr_to_string(reinterpret_cast<const sockaddr *>(&ip->addr), ip->addrlen));
        remove_interactive(ip->ob, false);
#else
        // do nothing
#endif
      }
      on_telnet_do_mssp(ip);
      break;
    case TELNET_TELOPT_ZMP:
      if (!CONFIG_INT(__RC_ENABLE_ZMP__)) {
#ifdef DEBUG
        debug_message(
            "Bad client: bogus IAC DO ZMP from %s.",
            sockaddr_to_string(reinterpret_cast<const sockaddr *>(&ip->addr), ip->addrlen));
        remove_interactive(ip->ob, false);
#else
        // do nothing
#endif
      }
      ip->iflags |= USING_ZMP;
      break;
    case TELNET_TELOPT_COMPRESS2:
      telnet_begin_compress2(ip->telnet);
      break;
    default:
      debug(telnet, "on_telnet_do: unimplemented code: %d.\n", cmd);
      telnet_negotiate(ip->telnet, TELNET_WONT, cmd);
      break;
  }
  flush_message(ip);
}

static inline void on_telnet_dont(unsigned char cmd, interactive_t *ip) {
  switch (cmd) {
    case TELNET_TELOPT_ECHO:
      /* do nothing */
      break;
    case TELNET_TELOPT_SGA:
      if (ip->iflags & USING_LINEMODE) {
        ip->iflags &= ~SUPPRESS_GA;
        telnet_negotiate(ip->telnet, TELNET_WONT, TELNET_TELOPT_SGA);
      }
      break;
    default:
      debug(telnet, "on_telnet_dont: unimplemented code: %d.\n", cmd);
      break;
  }
}

static inline void on_telnet_subnegotiation(unsigned char cmd, const char *buf, unsigned long size,
                                            interactive_t *ip) {
  // NOTE: I received bug report that with following data sequences:
  // 000000 ff fc 22 ff fa 22 ff f0 ff ff fc 03 ff fc 18 ff
  // 000010 fc 1f ff fc 27 ff fe 56 ff fc 5b ff fe 46 ff fe
  // 000020 5d ff fe c9
  // 000024
  // Which translate into WONT LINEMODE, an empty SB LINEMODE (weird), (ff ff fc 03) some lingering
  // data, and other DOs and DONTs. It is suspected that this is sent by a broken client.
  // This causes crash for the driver since "buf" will be empty, which it should never do for SB
  // LINEMODE.
  //
  // Data shows that multiple crash all caused by conns from 208.100.* ip address, which is a huge
  // IP space of hosting company, some google search suggest it maybe doing port scanning or
  // attacking.
  //
  // There might be a issue in their client code which caused this problem.
  //
  // Nonetheless, checking buf and size here will be good safeguard for similar problems.
  if (buf == nullptr || size == 0) {
    return;
  }

  switch (cmd) {
    case TELNET_TELOPT_COMPRESS2:
    case TELNET_TELOPT_ZMP:
    case TELNET_TELOPT_TTYPE:
    case TELNET_TELOPT_ENVIRON:
    case TELNET_TELOPT_NEW_ENVIRON:
    case TELNET_TELOPT_MSSP:
      // These are handled by the libtelnet. We just need to handle the rest.
      break;
    case TELNET_TELOPT_LINEMODE: {
      // The example at the end of RFC1184 is very useful to understand
      // how this works.
      // Basically, we force client to use MODE_EDIT | MODE_TRAPSIG
      // and we ignore SLC settings.
      auto action = static_cast<unsigned char>(buf[0]);
      switch (action) {
        case LM_MODE:
          /* Don't do anything with an ACK */
          if (!(buf[1] & MODE_ACK)) {
            /* Accept only EDIT and TRAPSIG && force them too */
            const unsigned char sb_ack[] = {LM_MODE, MODE_EDIT | MODE_TRAPSIG | MODE_ACK};
            telnet_subnegotiation(ip->telnet, TELNET_TELOPT_LINEMODE,
                                  reinterpret_cast<const char *>(sb_ack), sizeof(sb_ack));
          }
          break;
        case LM_SLC: {
          /* Seriously, no one cares special chars these days,
           * we just ignore it. */
          break;
        }
        /* refuse FORWARDMASK */
        case TELNET_DO: {
          const unsigned char sb_wont[] = {WONT, static_cast<unsigned char>(buf[1])};
          telnet_subnegotiation(ip->telnet, TELNET_TELOPT_LINEMODE,
                                reinterpret_cast<const char *>(sb_wont), sizeof(sb_wont));
          break;
        }
        case TELNET_WILL: {
          const unsigned char sb_dont[] = {DONT, static_cast<unsigned char>(buf[1])};
          telnet_subnegotiation(ip->telnet, TELNET_TELOPT_LINEMODE,
                                reinterpret_cast<const char *>(sb_dont), sizeof(sb_dont));
          break;
        }
        default:
          break;
      }
      break;
    }
    case TELNET_TELOPT_NAWS: {
      if (size >= 4) {
        push_number((static_cast<unsigned char>(buf[0]) << 8) | static_cast<unsigned char>(buf[1]));
        push_number((static_cast<unsigned char>(buf[2]) << 8) | static_cast<unsigned char>(buf[3]));
        set_eval(max_eval_cost);
        safe_apply(APPLY_WINDOW_SIZE, ip->ob, 2, ORIGIN_DRIVER);
      }
      break;
    }
    case TELNET_TELOPT_GMCP: {
      // We need to make sure the string will be NULL-termed.
      char *str = new_string(size, "telnet gmcp");
      str[size] = '\0';
      strncpy(str, buf, size);

      push_malloced_string(str);
      set_eval(max_eval_cost);
      safe_apply(APPLY_GMCP, ip->ob, 1, ORIGIN_DRIVER);
      break;
    }
    default: {
      // translate NUL to 'I', apparently
      char *str = new_string(size, "telnet suboption");
      str[size] = '\0';
      for (int i = 0; i < size; i++) {
        str[i] = (buf[i] ? buf[i] : 'I');
      }
      push_malloced_string(str);
      set_eval(max_eval_cost);
      safe_apply(APPLY_TELNET_SUBOPTION, ip->ob, 1, ORIGIN_DRIVER);
      break;
    }
  }
  flush_message(ip);
}

static inline void on_telnet_environ(const struct telnet_environ_t *values, unsigned long size,
                                     interactive_t *ip) {
  for (int i = 0; i < size; i++) {
    if (values[i].var == nullptr || values[i].value == nullptr) {
      continue;
    }
    auto sanitized_var = u8_sanitize(values[i].var);
    copy_and_push_string(sanitized_var.c_str());

    auto sanitized_value = u8_sanitize(values[i].value);
    copy_and_push_string(sanitized_value.c_str());

    set_eval(max_eval_cost);
    safe_apply(APPLY_RECEIVE_ENVIRON, ip->ob, 2, ORIGIN_DRIVER);
  }
}

static inline void on_telnet_ttype(const char *name, interactive_t *ip) {
  copy_and_push_string(name);
  set_eval(max_eval_cost);
  safe_apply(APPLY_TERMINAL_TYPE, ip->ob, 1, ORIGIN_DRIVER);
}

// Main event handler.
void telnet_event_handler(telnet_t *telnet, telnet_event_t *ev, void *user_data) {
  auto ip = reinterpret_cast<interactive_t *>(user_data);

  switch (ev->type) {
    case TELNET_EV_DATA: {
      on_telnet_data(ev->data.buffer, ev->data.size, ip);
      break;
    }
    case TELNET_EV_SEND: {
      on_telnet_send(ev->data.buffer, ev->data.size, ip);
      break;
    }
    case TELNET_EV_IAC: {
      on_telnet_iac(ev->iac.cmd, ip);
      break;
    }
    case TELNET_EV_WILL: {
      ip->iflags |= USING_TELNET;
      on_telnet_will(ev->neg.telopt, ip);
      break;
    }
    case TELNET_EV_WONT: {
      ip->iflags |= USING_TELNET;
      on_telnet_wont(ev->neg.telopt, ip);
      break;
    }
    case TELNET_EV_DO: {
      ip->iflags |= USING_TELNET;
      on_telnet_do(ev->neg.telopt, ip);
      break;
    }
    case TELNET_EV_DONT: {
      ip->iflags |= USING_TELNET;
      on_telnet_dont(ev->neg.telopt, ip);
      break;
    }
    case TELNET_EV_SUBNEGOTIATION: {
      on_telnet_subnegotiation(ev->sub.telopt, ev->sub.buffer, ev->sub.size, ip);
      break;
    }
    case TELNET_EV_COMPRESS: {
      ip->iflags |= USING_COMPRESS;
      break;
    }
    case TELNET_EV_ZMP: {
      ip->iflags |= USING_ZMP;
      on_telnet_do_zmp(ev->zmp.argv, ev->zmp.argc, ip);
      break;
    }
    case TELNET_EV_TTYPE: {
      if (ev->ttype.cmd == TELNET_TTYPE_IS) {
        on_telnet_ttype(ev->ttype.name, ip);
        break;
      }
      break;
    }
    case TELNET_EV_ENVIRON: {
      if (ev->environ.cmd == TELNET_ENVIRON_IS || ev->environ.cmd == TELNET_ENVIRON_INFO) {
        on_telnet_environ(ev->environ.values, ev->environ.size, ip);
        break;
      }
      break;
    }
    case TELNET_EV_MSSP: {
      on_telnet_do_mssp(ip);
      break;
    }
    case TELNET_EV_WARNING:
      debug(telnet, "TELNET_EV_WARNING: %s. \n", ev->error.msg);
      break;
    case TELNET_EV_ERROR:
      debug(telnet, "TELNET_EV_ERROR: %s. \n", ev->error.msg);
      // fatal_error("TELNET error: %s", event->error.msg);
      break;
    default:
      debug(telnet, "unhandled event: %d", ev->type);
      break;
  }
}

// Server initiated negotiations.
//
// NOTE: Some options need to be sent DO first, and some
// needs WILL, don't change or you will risk breaking clients.
void send_initial_telnet_negotiations(struct interactive_t *user) {
  // Default request linemode, save bytes/cpu.
  set_linemode(user, false);

  // Get rid of GA, save some byte
  telnet_negotiate(user->telnet, TELNET_DO, TELNET_TELOPT_SGA);

  /* Ask permission to ask them for their terminal type */
  telnet_negotiate(user->telnet, TELNET_DO, TELNET_TELOPT_TTYPE);

  /* Ask them for their window size */
  telnet_negotiate(user->telnet, TELNET_DO, TELNET_TELOPT_NAWS);

  // Also newenv
  telnet_negotiate(user->telnet, TELNET_DO, TELNET_TELOPT_NEW_ENVIRON);

/* We support COMPRESS2 */
#ifdef HAVE_ZLIB  // come from libtelnet
  telnet_negotiate(user->telnet, TELNET_WILL, TELNET_TELOPT_COMPRESS2);
#endif

  // Ask them if they support mxp.
  if (CONFIG_INT(__RC_ENABLE_MXP__)) {
    telnet_negotiate(user->telnet, TELNET_DO, TELNET_TELOPT_MXP);
  }

  // gmcp
  if (CONFIG_INT(__RC_ENABLE_GMCP__)) {
    telnet_negotiate(user->telnet, TELNET_WILL, TELNET_TELOPT_GMCP);
  }

  // zmp
  if (CONFIG_INT(__RC_ENABLE_ZMP__)) {
    telnet_negotiate(user->telnet, TELNET_WILL, TELNET_TELOPT_ZMP);
  }

  // And we support mssp
  // http://tintin.sourceforge.net/mssp/ , server send WILL first.
  if (CONFIG_INT(__RC_ENABLE_MSSP__)) {
    telnet_negotiate(user->telnet, TELNET_WILL, TELNET_TELOPT_MSSP);
  }
}

void set_linemode(interactive_t *ip, bool flush) {
  if (ip->telnet) {
    telnet_negotiate(ip->telnet, TELNET_DO, TELNET_TELOPT_LINEMODE);

    const unsigned char sb_mode[] = {LM_MODE, MODE_EDIT | MODE_TRAPSIG};
    telnet_subnegotiation(ip->telnet, TELNET_TELOPT_LINEMODE,
                          reinterpret_cast<const char *>(sb_mode), sizeof(sb_mode));

    if (flush) {
      flush_message(ip);
    }
  }
}

void set_charmode(interactive_t *ip, bool flush) {
  if (ip->telnet) {
    if (ip->iflags & USING_LINEMODE) {
      telnet_negotiate(ip->telnet, TELNET_DONT, TELNET_TELOPT_LINEMODE);
    }
    if (flush) {
      flush_message(ip);
    }
  }
}

void set_localecho(interactive_t *ip, bool enable, bool flush) {
  if (ip->telnet) {
    telnet_negotiate(ip->telnet, enable ? TELNET_WONT : TELNET_WILL, TELNET_TELOPT_ECHO);
    if (flush) {
      flush_message(ip);
    }
  }
}

void telnet_do_naws(struct telnet_t *telnet) {
  telnet_negotiate(telnet, TELNET_DO, TELNET_TELOPT_NAWS);
}

void telnet_dont_naws(struct telnet_t *telnet) {
  telnet_negotiate(telnet, TELNET_DONT, TELNET_TELOPT_NAWS);
}
void telnet_start_request_ttype(struct telnet_t *telnet) {
  telnet_negotiate(telnet, TELNET_DO, TELNET_TELOPT_TTYPE);
}
void telnet_request_ttype(struct telnet_t *telnet) { telnet_begin_sb(telnet, TELNET_TTYPE_SEND); }

void telnet_send_nop(struct telnet_t *telnet) {
  if (telnet) {
    telnet_iac(telnet, TELNET_NOP);
  }
}

void telnet_send_ga(struct telnet_t *telnet) {
  if (telnet) {
    telnet_iac(telnet, TELNET_GA);
  }
}

/* MXP */

void on_telnet_will_mxp(interactive_t *ip) {
  ip->iflags |= USING_MXP;
  /* Mxp is enabled, tell the mudlib about it. */
  set_eval(max_eval_cost);
  safe_apply(APPLY_MXP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
}

bool on_receive_mxp_tag(interactive_t *ip, const char *user_command) {
  copy_and_push_string(user_command);

  set_eval(max_eval_cost);
  auto ret = safe_apply(APPLY_MXP_TAG, ip->ob, 1, ORIGIN_DRIVER);
  if (ret && ret->type == T_NUMBER && ret->u.number) {
    return false;
  }
  return true;
}

/* GMCP */

void on_telnet_do_gmcp(interactive_t *ip) {
  ip->iflags |= USING_GMCP;

  set_eval(max_eval_cost);
  safe_apply(APPLY_GMCP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
}

/* ZMP */

void on_telnet_do_zmp(const char **argv, unsigned long argc, interactive_t *ip) {
  ip->iflags |= USING_ZMP;

  // Push the command
  copy_and_push_string(argv[0]);

  // Push the array
  array_t *arr = allocate_array(argc - 1);
  for (int i = 1; i < argc; i++) {
    arr->item[i].u.string = string_copy(argv[i], "ZMP");
    arr->item[i].type = T_STRING;
    arr->item[i].subtype = STRING_MALLOC;
  }
  push_refed_array(arr);

  set_eval(max_eval_cost);
  safe_apply(APPLY_ZMP, ip->ob, 2, ORIGIN_DRIVER);
}
