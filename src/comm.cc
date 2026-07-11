/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "base/std.h"

#include "comm.h"

#include <cstdarg>   // for va_end, va_list, va_copy, etc
#include <cstdio>    // for snprintf, vsnprintf, fwrite, etc
#include <cstring>   // for NULL, memcpy, strlen, etc
#include <unistd.h>  // for gethostname
#include <memory>    // for unique_ptr
// Network stuff
#ifndef _WIN32
#include <netdb.h>       // for getnameinfo, gai_strerror
#include <netinet/in.h>  // for ntohl
#include <sys/socket.h>  // for sockaddr
#else
#include <ws2tcpip.h>
#endif

#include "backend.h"
#include "interactive.h"
#include "thirdparty/libtelnet/libtelnet.h"
#include "net/telnet.h"
#include "net/transport.h"
#include "user.h"
#include "vm/vm.h"

#include "packages/core/add_action.h"  // FIXME?
#include "packages/core/dns.h"         // FIXME?
#include "packages/core/ed.h"          // FIXME?

// in backend.cc
extern void update_load_av();
/*
 * local function prototypes.
 */
static char* get_user_command(interactive_t* /*ip*/);
static char* first_cmd_in_buf(interactive_t* /*ip*/);
static int call_function_interactive(interactive_t* /*i*/, char* /*str*/);
static void print_prompt(interactive_t* /*ip*/);

#ifdef NO_SNOOP
#define handle_snoop(str, len, who)
#else
#define handle_snoop(str, len, who) \
  if ((who)->snooped_by) receive_snoop(str, len, (who)->snooped_by)

static void receive_snoop(const char* /*buf*/, int /*len*/, object_t* ob);

#endif

namespace {
// If a complete command is buffered, ask the connection's transport to
// arrange a process_user_command() run on the driver's scheduler.
void maybe_schedule_user_command(interactive_t* user) {
  if (user->iflags & CMD_IN_BUF) {
    user->transport->schedule_command();
  }
}
}  // namespace

/*
 * Run one buffered command for this user, with the eval budget reset and
 * the longjmp-safety cleanup every scheduler must perform. Transports'
 * schedule_command() implementations funnel here.
 */
void comm_run_scheduled_command(interactive_t* ip) {
  set_eval(max_eval_cost);
  process_user_command(ip);

  /* Has to be cleared if we jumped out of process_user_command() */
  current_interactive = nullptr;
}

// Called upon user, when he's finished negotiations , and ready to logon
void on_user_logon(interactive_t* user) {
  set_command_giver(master_ob);
  master_ob->flags |= O_ONCE_INTERACTIVE;
  master_ob->interactive = user;

  /*
   * The user object has one extra reference. It is asserted that the
   * master_ob is loaded.  Save a pointer to the master ob incase it
   * changes during APPLY_CONNECT.  We want to free the reference on
   * the right copy of the object.
   */
  object_t *master, *ob;
  svalue_t* ret;

  master = master_ob;
  add_ref(master_ob, "new_user");
  push_number(user->local_port);
  set_eval(max_eval_cost);
  ret = safe_apply_master_ob(APPLY_CONNECT, 1);
  /* master_ob->interactive can be zero if the master object self
   destructed in the above (don't ask) */
  set_command_giver(nullptr);
  if (ret == nullptr || ret == (svalue_t*)-1 || ret->type != T_OBJECT || !master_ob->interactive) {
    debug_message("Can not accept connection from %s due to error in connect().\n",
                  sockaddr_to_string(reinterpret_cast<sockaddr*>(&user->addr), user->addrlen));
    if (master_ob->interactive) {
      remove_interactive(master_ob, 0);
    }
    return;
  }
  /*
   * There was an object returned from connect(). Use this as the user
   * object.
   */
  ob = ret->u.ob;
  ob->interactive = master_ob->interactive;
  ob->interactive->ob = ob;
  ob->flags |= O_ONCE_INTERACTIVE;
  /*
   * assume the existance of write_prompt and process_input in user.c
   * until proven wrong (after trying to call them).
   */
  ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);

  free_object(&master, "new_user");

  master_ob->flags &= ~O_ONCE_INTERACTIVE;
  master_ob->interactive = nullptr;
  add_ref(ob, "new_user");

  // start reverse DNS probing.
  query_name_by_addr(ob);

  set_command_giver(ob);

  set_prompt("> ");

  // A fast client may have answered the initial DO NAWS while ip->ob was
  // still the master object; replay the cached report on the real user.
  if (ob->interactive->naws_w) {
    push_number(ob->interactive->naws_w);
    push_number(ob->interactive->naws_h);
    set_eval(max_eval_cost);
    safe_apply(APPLY_WINDOW_SIZE, ob, 2, ORIGIN_DRIVER);
  }

  // Call logon() on the object.
  set_eval(max_eval_cost);
  ret = safe_apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
  if (ret == nullptr) {
    debug_message("new_conn_handler: logon() on object %s has failed, the user is disconnected.\n",
                  ob->obname);
    remove_interactive(ob, false);
  } else if (ob->flags & O_DESTRUCTED) {
    // logon() may decide not to allow user connect by destroying objects.
    remove_interactive(ob, true);
  }
  set_command_giver(nullptr);
}

/*
 * If there is a shadow for this object, then the message should be
 * sent to it. But only if catch_tell() is defined. Beware that one of the
 * shadows may be the originator of the message, which means that we must
 * not send the message to that shadow, or any shadows in the linked list
 * before that shadow.
 *
 * Also note that we don't need to do this in the case of
 * INTERACTIVE_CATCH_TELL, since catch_tell() was already called
 * _instead of_ add_message(), and shadows got their chance then.
 */
#if !defined(NO_SHADOWS)
#define SHADOW_CATCH_MESSAGE
#endif

#ifdef SHADOW_CATCH_MESSAGE
static int shadow_catch_message(object_t* ob, const char* str) {
  if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
    return 0;
  }
  if (!ob->shadowed) {
    return 0;
  }
  while (ob->shadowed != nullptr && ob->shadowed != current_object) {
    ob = ob->shadowed;
  }
  while (ob->shadowing) {
    copy_and_push_string(str);
    if (apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER))
    /* this will work, since we know the */
    /* function is defined */
    {
      return 1;
    }
    ob = ob->shadowing;
  }
  return 0;
}
#endif

/*
 * Send a message to an interactive object. If that object is shadowed,
 * special handling is done.
 */
void add_message(object_t* who, const char* data, int len) {
  /*
   * if who->interactive is not valid, write message on stderr.
   * (maybe)
   */
  if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
      (who->interactive->iflags & (NET_DEAD | CLOSING))) {
    if (CONFIG_INT(__RC_NONINTERACTIVE_STDERR_WRITE__)) {
      putc(']', stderr);
      fwrite(data, len, 1, stderr);
    }
    return;
  }

  inet_packets++;

  auto* ip = who->interactive;
  switch (ip->connection_type) {
    case PORT_TYPE_ASCII:
    case PORT_TYPE_TELNET:
    case PORT_TYPE_WEBSOCKET: {
      // Text connections: charset transcoding, then either through
      // libtelnet (telnet and websocket-telnet subprotocol: IAC escaping,
      // NVT translation, MCCP -- the wire bytes come back via
      // TELNET_EV_SEND -> on_telnet_send() -> transport) or straight to
      // the transport (ascii flavours, no telnet layer).
      auto transdata = u8_convert_encoding(ip->trans, data, len);
      auto result = transdata.empty() ? std::string_view(data, len) : transdata;
      inet_volume += result.size();
      if (ip->telnet != nullptr) {
        telnet_send_text(ip->telnet, result.data(), result.size());
      } else {
        ip->transport->write(result.data(), result.size());
      }
    } break;
    default: {
      // Binary connections (PORT_TYPE_BINARY/MUD): raw bytes.
      inet_volume += len;
      ip->transport->write(data, len);
      break;
    }
  }

#ifdef SHADOW_CATCH_MESSAGE
  /*
   * shadow handling.
   */
  if (shadow_catch_message(who, data)) {
    if (CONFIG_INT(__RC_SNOOP_SHADOWED__)) {
      handle_snoop(data, len, ip);
    }
    return;
  }
#endif /* NO_SHADOWS */
  handle_snoop(data, len, ip);

  add_message_calls++;
} /* add_message() */

void add_vmessage(object_t* who, const char* format, ...) {
  va_list args, args2;
  va_start(args, format);
  va_copy(args2, args);
  static char buf[LARGEST_PRINTABLE_STRING + 1];
  do {
    auto result = vsnprintf(buf, sizeof(buf), format, args);
    if (result < 0) {
      DEBUG_CHECK(result < 0, "Invalid format string: add_vmessage");
      break;
    }
    if (result <= sizeof(buf)) {
      add_message(who, buf, result);
    } else {
      std::unique_ptr<char[]> const msg(new char[result + 1]);
      result = vsnprintf(msg.get(), result + 1, format, args2);
      if (result < 0) break;
      add_message(who, msg.get(), result);
    }
  } while (false);
  va_end(args2);
  va_end(args);
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message(interactive_t* ip) {
  /*
   * if ip is not valid, do nothing.
   */
  if (!ip || !ip->transport) {
    debug(connections, ("flush_message: invalid target!\n"));
    return 0;
  }

  return ip->transport->flush();
}

void flush_message_all() {
  users_foreach([](interactive_t* user) { flush_message(user); });
}

/*
 * Make room in ip->text for at least min_space incoming bytes: compact
 * already-consumed input first and, as a last resort, drop the (overlong)
 * pending command. Returns the usable free space. Shared by every input
 * transport (socket, websocket, WASM JS bridge).
 */
size_t comm_reserve_input_space(interactive_t* ip, size_t min_space) {
  size_t text_space = sizeof(ip->text) - ip->text_end;
  if (text_space < min_space) {
    if (ip->text_start > 0) {
      memmove(ip->text, ip->text + ip->text_start, ip->text_end - ip->text_start);
      text_space += ip->text_start;
      ip->text_end -= ip->text_start;
      ip->text_start = 0;
    }
    if (text_space < min_space) {
      ip->iflags |= SKIP_COMMAND;
      ip->text_start = ip->text_end = 0;
      text_space = sizeof(ip->text);
    }
  }
  return text_space;
}

static int clean_buf(interactive_t* ip) {
  /* skip null input */
  while (ip->text_start < ip->text_end && !*(ip->text + ip->text_start)) {
    ip->text_start++;
  }

  /* if we've advanced beyond the end of the buffer, reset it */
  if (ip->text_start >= ip->text_end) {
    ip->text_start = ip->text_end = 0;
  }

  /* if we're skipping the current command, check to see if it has been
   completed yet.  if it has, flush it and clear the skip bit */
  if (ip->iflags & SKIP_COMMAND) {
    char* p;

    for (p = ip->text + ip->text_start; p < ip->text + ip->text_end; p++) {
      if (*p == '\r' || *p == '\n') {
        ip->text_start += p - (ip->text + ip->text_start) + 1;
        ip->iflags &= ~SKIP_COMMAND;
        return clean_buf(ip);
      }
    }
  }

  return (ip->text_end > ip->text_start);
}

void comm_text_received(interactive_t* ip, const char* data, size_t len) {
  if (!len) {
    return;
  }

  comm_reserve_input_space(ip, len);

  on_user_input(ip, data, len);

  if (cmd_in_buf(ip)) {
    ip->iflags |= CMD_IN_BUF;

    maybe_schedule_user_command(ip);
  }
}

void comm_telnet_received(interactive_t* ip, const char* data, size_t len) {
  if (!len) {
    return;
  }
  comm_reserve_input_space(ip, len);

  int const start = ip->text_end;

  // this will read data into ip->text
  telnet_recv(ip->telnet, data, len);
  // If we read something
  if (ip->text_end > start) {
    /* handle snooping - snooper does not see type-ahead due to
      telnet being in linemode */
    if (!(ip->iflags & NOECHO)) {
      handle_snoop(ip->text + start, ip->text_end - start, ip);
    }

    // search for command.
    if (cmd_in_buf(ip)) {
      ip->iflags |= CMD_IN_BUF;

      maybe_schedule_user_command(ip);
    }
  }
}

// ANSI
static const int ANSI_SUBSTITUTE = 0x20;

// Used by both telnet and ws_ascii, in case of telnet, default is linemode, which means
// client will actually send entire line. In ascii mode, we will get an single char input
// each time.
void on_user_input(interactive_t* ip, const char* data, size_t len) {
  for (int i = 0; i < len; i++) {
    if (ip->text_end == sizeof(ip->text) - 1) {
      // No more space
      break;
    }
    auto c = static_cast<unsigned char>(data[i]);
    switch (c) {
      case 0x08:  // BACKSPACE
      case 0x7f:  // DEL
        if (ip->iflags & SINGLE_CHAR) {
          ip->text[ip->text_end++] = c;
        } else {
          if (ip->text_end > 0) {
            ip->text_end--;
          }
        }
        break;
      case 0x1b:
        // The anti-ANSI-injection rewrite is a line-mode protection; in
        // single-char mode the mudlib asked for raw keystrokes and needs
        // ESC intact to decode arrow/function keys.
        if (!(ip->iflags & SINGLE_CHAR) && CONFIG_INT(__RC_NO_ANSI__) &&
            CONFIG_INT(__RC_STRIP_BEFORE_PROCESS_INPUT__)) {
          ip->text[ip->text_end++] = ANSI_SUBSTITUTE;
          break;
        }
        // fallthrough
      default:
        ip->text[ip->text_end++] = c;
        break;
    }
  }
}

/*
 * Single-char ("get_char") mode delivers one keystroke per callback.  A
 * keystroke is one byte, except when the buffer starts with a complete,
 * well-formed UTF-8 multi-byte sequence -- that is delivered whole, so LPC
 * receives one valid character instead of 2-4 fragment bytes.  Waiting for
 * the tail of a sequence is safe: the transports u8_sanitize() their input,
 * so a valid prefix is always completed by subsequent bytes.  Malformed
 * bytes are delivered one at a time, never stalled on.
 *
 * Returns the byte count to deliver, or 0 to wait for more input.
 */
static int single_char_bytes(const interactive_t* ip) {
  const auto* p = reinterpret_cast<const unsigned char*>(ip->text) + ip->text_start;
  const int avail = ip->text_end - ip->text_start;
  if (avail <= 0) {
    return 0;
  }
  int len;
  if (p[0] < 0xc2 || p[0] > 0xf4) {
    return 1; /* ASCII, or a byte that can't start a valid sequence */
  } else if (p[0] < 0xe0) {
    len = 2;
  } else if (p[0] < 0xf0) {
    len = 3;
  } else {
    len = 4;
  }
  if (avail < len) {
    /* wait for the rest, unless the buffer can never grow */
    return ip->text_end == sizeof(ip->text) - 1 ? 1 : 0;
  }
  for (int i = 1; i < len; i++) {
    if ((p[i] & 0xc0) != 0x80) {
      return 1; /* malformed: deliver the lead byte alone */
    }
  }
  return len;
}

// Also used by ws_ascii.
int cmd_in_buf(interactive_t* ip) {
  char* p;

  /* do standard input buffer cleanup */
  if (!clean_buf(ip)) {
    return 0;
  }

  /* if we're in single character mode, we've got input once a whole
     keystroke (possibly a multi-byte UTF-8 character) is buffered */
  if (ip->iflags & SINGLE_CHAR) {
    return single_char_bytes(ip) > 0;
  }

  for (p = ip->text + ip->text_start; p < ip->text + ip->text_end; p++) {
    if (*p == '\r' || *p == '\n') {
      return 1;
    }
  }

  /* duh, no command */
  return 0;
}

static char* first_cmd_in_buf(interactive_t* ip) {
  char* p;
  static char tmp[5];

  /* do standard input buffer cleanup */
  if (!clean_buf(ip)) {
    return nullptr;
  }

  p = ip->text + ip->text_start;

  /* if we're in single character mode, deliver one keystroke: control
     bytes (including BS/DEL) verbatim, multi-byte UTF-8 characters whole */
  if (ip->iflags & SINGLE_CHAR) {
    const int len = single_char_bytes(ip);
    if (len == 0) {
      /* incomplete UTF-8 sequence: wait for the rest */
      ip->iflags &= ~CMD_IN_BUF;
      return nullptr;
    }
    memcpy(tmp, p, len);
    tmp[len] = '\0';
    for (int i = 0; i < len; i++) {
      ip->text[ip->text_start++] = 0;
    }
    if (!clean_buf(ip)) {
      ip->iflags &= ~CMD_IN_BUF;
    }
    return tmp;
  }

  /* search for the newline */
  while (ip->text_start < ip->text_end && ip->text[ip->text_start] != '\n' &&
         ip->text[ip->text_start] != '\r') {
    ip->text_start++;
  }

  /* check for "\r\n" or "\n\r" */
  if (ip->text_start + 1 < ip->text_end &&
      ((ip->text[ip->text_start] == '\r' && ip->text[ip->text_start + 1] == '\n') ||
       (ip->text[ip->text_start] == '\n' && ip->text[ip->text_start + 1] == '\r'))) {
    ip->text[ip->text_start++] = 0;
  }

  ip->text[ip->text_start++] = 0;

  if (!cmd_in_buf(ip)) {
    ip->iflags &= ~CMD_IN_BUF;
  }

  return p;
}

/*
 * Return the first command of the next user in sequence that has a complete
 * command in their buffer.  A command is defined to be a single character
 * when SINGLE_CHAR is set, or a newline terminated string otherwise.
 */
static char* get_user_command(interactive_t* ip) {
  char* user_command = nullptr;

  if (!ip || !ip->ob || (ip->ob->flags & O_DESTRUCTED)) {
    return nullptr;
  }

  /* if there's a command in the buffer, pull it out! */
  if (ip->iflags & CMD_IN_BUF) {
    user_command = first_cmd_in_buf(ip);
  }

  /* no command found - return NULL */
  if (!user_command) {
    return nullptr;
  }

  /* got a command - return it and set command_giver */
  debug(connections, "get_user_command: user_command = (%s)\n", user_command);
  save_command_giver(ip->ob);

  if ((ip->iflags & NOECHO)) {
    /* must not enable echo before the user input is received */
    set_localecho(command_giver->interactive, true);
    ip->iflags &= ~NOECHO;
  }

  ip->last_time = get_current_time();
  return user_command;
} /* get_user_command() */

static int escape_command(interactive_t* ip, const char* user_command) {
  if (user_command[0] != '!') {
    return 0;
  }
#ifdef OLD_ED
  if (ip->ed_buffer) {
    return 1;
  }
#endif
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to && (!(ip->iflags & NOESC) && !(ip->iflags & I_SINGLE_CHAR))) {
    return 1;
  }
#endif
  return 0;
}

static void process_input(interactive_t* ip, char* user_command) {
  svalue_t* ret;

  if (!(ip->iflags & HAS_PROCESS_INPUT)) {
    safe_parse_command(user_command, command_giver);
    return;
  }

  /*
   * send a copy of user input back to user object to provide
   * support for things like command history and mud shell
   * programming languages.
   */
  copy_and_push_string(user_command);
  ret = safe_apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
  if (!IP_VALID(ip, command_giver)) {
    return;
  }
  if (!ret) {
    ip->iflags &= ~HAS_PROCESS_INPUT;
    safe_parse_command(user_command, command_giver);
    return;
  }

#ifndef NO_ADD_ACTION
  if (ret->type == T_STRING) {
    auto* command = string_copy(ret->u.string, "current_command: " __CURRENT_FILE_LINE__);
    DEFER { FREE_MSTR(command); };
    safe_parse_command(command, command_giver);
  } else {
    if (ret->type != T_NUMBER || !ret->u.number) {
      safe_parse_command(user_command, command_giver);
    }
  }
#endif
}

/*
 * This is the user command handler. This function is called when
 * a user command needs to be processed.
 * This function calls get_user_command() to get a user command.
 * One user command is processed per execution of this function.
 */
int process_user_command(interactive_t* ip) {
  char* user_command;

  /*
   * WARNING: get_user_command() sets command_giver via
   * save_command_giver(), but only when the return is non-zero!
   */
  if (!(user_command = get_user_command(ip))) {
    return 0;
  }

  if (ip != command_giver->interactive) {
    DEBUG_FATAL("BUG: process_user_command.");
  }

  current_interactive = command_giver; /* this is yuck phooey, sigh */
  if (ip) {
    clear_notify(ip->ob);
  }

  // FIXME: move this to somewhere else
  update_load_av();

  debug(connections, "process_user_command: command_giver = /%s\n", command_giver->obname);

  if (!ip) {
    goto exit;
  }

  if (ip->iflags & USING_MXP) {
    if (user_command[0] == ' ' && user_command[1] == '[' && user_command[3] == 'z') {
      if (!on_receive_mxp_tag(ip, user_command)) {
        goto exit;
      }
    }
  }

  if (escape_command(ip, user_command)) {
    if (ip->iflags & SINGLE_CHAR) {
      /* only 1 char ... switch to line buffer mode */
      ip->iflags |= WAS_SINGLE_CHAR;
      ip->iflags &= ~SINGLE_CHAR;
      ip->text_start = ip->text_end = *ip->text = 0;
      set_linemode(ip, true);
    } else {
      if (ip->iflags & WAS_SINGLE_CHAR) {
        /* we now have a string ... switch back to char mode */
        ip->iflags &= ~WAS_SINGLE_CHAR;
        ip->iflags |= SINGLE_CHAR;
        set_charmode(ip, true);
        if (!IP_VALID(ip, command_giver)) {
          goto exit;
        }
      }

      process_input(ip, user_command + 1);
    }

    goto exit;
  }

#ifdef OLD_ED
  if (ip->ed_buffer) {
    ed_cmd(user_command);
    goto exit;
  }
#endif
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (call_function_interactive(ip, user_command)) {
    goto exit;
  }
#endif

  process_input(ip, user_command);

exit:
  /*
   * Print a prompt if user is still here.
   */
  if (IP_VALID(ip, command_giver)) {
    if (ip->input_to == nullptr) {
      print_prompt(ip);
    }
    if (ip->telnet && (ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
      telnet_send_ga(ip->telnet);
    }
    // FIXME: this doesn't belong here, should be moved to event.cc
    maybe_schedule_user_command(ip);
  }

  current_interactive = nullptr;
  restore_command_giver();
  return 1;
}

/*
 * Remove an interactive user immediately.
 */
void remove_interactive(object_t* ob, int dested) {
  /* don't have to worry about this dangling, since this is the routine
   * that causes this to dangle elsewhere, and we are protected from
   * getting called recursively by CLOSING.  safe_apply() should be
   * used here, since once we start this process we can't back out,
   * so jumping out with an error would be bad.
   */
  interactive_t* ip = ob->interactive;

  if (!ip) {
    return;
  }

  if (ip->iflags & CLOSING) {
    if (!dested) {
      debug_message("Double call to remove_interactive()\n");
    }
    return;
  }
  debug(connections, "Closing connection from %s.\n",
        sockaddr_to_string((struct sockaddr*)&ip->addr, ip->addrlen));
  flush_message(ip);
  ip->iflags |= CLOSING;

#ifdef OLD_ED
  if (ip->ed_buffer) {
    save_ed_buffer(ob);
  }
#else
  if (ob->flags & O_IN_EDIT) {
    object_save_ed_buffer(ob);
    ob->flags &= ~O_IN_EDIT;
  }
#endif

  if (!dested) {
    /*
     * auto-notification of net death
     */
    save_command_giver(ob);
    set_eval(max_eval_cost);
    safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);
    restore_command_giver();
  }

#ifndef NO_SNOOP
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
    ip->snooped_by = nullptr;
  }
#endif
  // Tear down the byte transport (socket/TLS buffers, websocket handle,
  // command timer, or the WASM JS bridge).
  if (ip->transport != nullptr) {
    ip->transport->close();
    delete ip->transport;
    ip->transport = nullptr;
  }

  // Free telnet handle
  if (ip->telnet != nullptr) {
    telnet_free(ip->telnet);
    ip->telnet = nullptr;
  }

  // Free translator
  if (ip->trans != nullptr) {
    ucnv_close(ip->trans);
    ip->trans = nullptr;
  }

  clear_notify(ip->ob);

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to) {
    free_object(&ip->input_to->ob, "remove_interactive");
    free_sentence(ip->input_to);
    if (ip->num_carry > 0) {
      free_some_svalues(ip->carryover, ip->num_carry);
    }
    ip->carryover = nullptr;
    ip->num_carry = 0;
    ip->input_to = nullptr;
  }
#endif

  user_del(ip);
  FREE(ip);
  ob->interactive = nullptr;
  free_object(&ob, "remove_interactive");
} /* remove_interactive() */

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
static int call_function_interactive(interactive_t* i, char* str) {
  object_t* ob;
  funptr_t* funp;
  const char* function;
  svalue_t* args;
  sentence_t* sent;
  int num_arg;
  int was_single = 0;
  int was_noecho = 0;
  int ret = 0;

  i->iflags &= ~NOESC;
  if (!(sent = i->input_to)) {
    return (0);
  }

  ob = sent->ob;
  /*
   * Special feature: input_to() has been called to setup a call to a
   * function.
   */
  if (i->iflags & SINGLE_CHAR) {
    /*
     * clear single character mode
     */
    i->iflags &= ~SINGLE_CHAR;
    was_single = 1;
  }
  if (i->iflags & NOECHO) {
    was_noecho = 1;
    i->iflags &= ~NOECHO;
  }
  if (ob->flags & O_DESTRUCTED) {
    /* Sorry, the object has selfdestructed ! */
    free_object(&sent->ob, "call_function_interactive");
    free_sentence(sent);
    i->input_to = nullptr;
    if (i->num_carry) {
      free_some_svalues(i->carryover, i->num_carry);
    }
    i->carryover = nullptr;
    i->num_carry = 0;
    i->input_to = nullptr;
    ret = 0;
  } else {
    /*
     * We must all references to input_to fields before the call to apply(),
     * because someone might want to set up a new input_to().
     */

    /* we put the function on the stack in case of an error */
    STACK_INC;
    if (sent->flags & V_FUNCTION) {
      function = nullptr;
      sp->type = T_FUNCTION;
      sp->u.fp = funp = sent->function.f;
      funp->hdr.ref++;
    } else {
      function = sent->function.s;
      if (function && function[0] == APPLY___INIT_SPECIAL_CHAR) {
        return 0;
      }
      sp->type = T_STRING;
      sp->subtype = STRING_SHARED;
      sp->u.string = function;
      ref_string(function);
    }

    free_object(&sent->ob, "call_function_interactive");
    free_sentence(sent);

    /*
     * If we have args, we have to copy them, so the svalues on the
     * interactive struct can be FREEd
     */
    num_arg = i->num_carry;
    if (num_arg) {
      args = i->carryover;
      i->num_carry = 0;
      i->carryover = nullptr;
    } else {
      args = nullptr;
    }

    i->input_to = nullptr;

    copy_and_push_string(str);
    /*
     * If we have args, we have to push them onto the stack in the order they
     * were in when we got them.  They will be popped off by the called
     * function.
     */
    if (args) {
      transfer_push_some_svalues(args, num_arg);
      FREE(args);
    }
    /* current_object no longer set */
    if (function) {
      (void)safe_apply(function, ob, num_arg + 1, ORIGIN_INTERNAL);
    } else {
      safe_call_function_pointer(funp, num_arg + 1);
    }
    // NOTE: we can't use "i" here anymore, it is possible that it
    // has been freed.
    pop_stack(); /* remove `function' from stack */

    ret = 1;
  }

  if (!(ob->flags & O_DESTRUCTED) && ob->interactive) {
    i = ob->interactive;
    if (was_single && !(i->iflags & SINGLE_CHAR)) {
      i->text_start = i->text_end = 0;
      i->text[0] = '\0';
      i->iflags &= ~CMD_IN_BUF;
      set_linemode(i, true);
    }
    if (was_noecho && !(i->iflags & NOECHO)) {
      set_localecho(i, true);
    }
  }
  return ret;
} /* call_function_interactive() */

int set_call(object_t* ob, sentence_t* sent, int flags) {
  if (ob == nullptr || sent == nullptr) {
    return (0);
  }
  auto* ip = ob->interactive;
  if (ip == nullptr || ip->input_to) {
    return (0);
  }
  ip->input_to = sent;
  ip->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
  if (flags & I_NOECHO) {
    set_localecho(ip, false);
  }
  if (flags & I_SINGLE_CHAR) {
    set_charmode(ip);
  }
  return (1);
} /* set_call() */
#endif

void set_prompt(const char* str) {
  if (command_giver && command_giver->interactive) {
    command_giver->interactive->prompt = str;
  }
} /* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt(interactive_t* ip) {
  if (!ip || !ip->ob || !IP_VALID(ip, ip->ob)) {
    return;
  }
  // With "interactive catch tell" enabled, tell_object() below runs the
  // catch_tell apply, and this driver-initiated path has no surrounding
  // recovery point -- an error() in it (e.g. out of eval cost) used to
  // reach error_handler() with no error context and crash (issue #1047).
  // Contain prompt errors here the same way safe_apply() does.
  error_context_t econ;
  save_context(&econ);
  try {
    /* give user object a chance to write its own prompt */
    if (!(ip->iflags & HAS_WRITE_PROMPT)) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#ifdef OLD_ED
    else if (ip->ed_buffer) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#endif
    else if (!safe_apply(APPLY_WRITE_PROMPT, ip->ob, 0, ORIGIN_DRIVER)) {
      ip->iflags &= ~HAS_WRITE_PROMPT;
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
  } catch (const char*) {
    restore_context(&econ);
  }
  pop_context(&econ);
} /* print_prompt() */

#ifndef NO_SNOOP
static void receive_snoop(const char* buf, int len, object_t* snooper) {
  /* command giver no longer set to snooper */
  if (CONFIG_INT(__RC_RECEIVE_SNOOP__)) {
    char* str;

    str = new_string(len, "receive_snoop");
    memcpy(str, buf, len);
    str[len] = 0;
    push_malloced_string(str);
    set_eval(max_eval_cost);
    safe_apply(APPLY_RECEIVE_SNOOP, snooper, 1, ORIGIN_DRIVER);
  } else {
    /* snoop output is now % in all cases */
    add_message(snooper, "%", 1);
    add_message(snooper, buf, len);
  }
}
#endif

/*
 * Let object 'me' snoop object 'you'. If 'you' is 0, then turn off
 * snooping.
 *
 * This routine is almost identical to the old set_snoop. The main
 * difference is that the routine writes nothing to user directly,
 * all such communication is taken care of by the mudlib. It communicates
 * with master.c in order to find out if the operation is permissble or
 * not. The old routine let everyone snoop anyone. This routine also returns
 * 0 or 1 depending on success.
 */
#ifndef NO_SNOOP
int new_set_snoop(object_t* by, object_t* victim) {
  interactive_t* ip;
  object_t* tmp;

  if (by->flags & O_DESTRUCTED) {
    return 0;
  }
  if (victim && (victim->flags & O_DESTRUCTED)) {
    return 0;
  }

  if (victim) {
    if (!victim->interactive) {
      error("Second argument of snoop() is not interactive!\n");
    }
    ip = victim->interactive;
  } else {
    /*
     * Stop snoop.
     */
    if (by->flags & O_SNOOP) {
      users_foreach([by](interactive_t* user) {
        if (user->snooped_by == by) {
          user->snooped_by = nullptr;
        }
      });
      by->flags &= ~O_SNOOP;
    }
    return 1;
  }

  /*
   * Protect against snooping loops.
   */
  tmp = by;
  while (tmp) {
    if (tmp == victim) {
      return 0;
    }

    /* the person snooping us, if any */
    tmp = (tmp->interactive ? tmp->interactive->snooped_by : nullptr);
  }

  /*
   * Terminate previous snoop, if any.
   */
  new_set_snoop(by, nullptr);

  // setup new snoop
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
  }
  by->flags |= O_SNOOP;
  ip->snooped_by = by;

  return 1;
} /* set_new_snoop() */
#endif

char* query_host_name() {
  static char name[400];

  gethostname(name, sizeof(name));
  name[sizeof(name) - 1] = '\0'; /* Just to make sure */
  return (name);
} /* query_host_name() */

#ifndef NO_SNOOP
object_t* query_snoop(object_t* ob) {
  if (!ob->interactive) {
    return nullptr;
  }
  return ob->interactive->snooped_by;
} /* query_snoop() */

object_t* query_snooping(object_t* ob) {
  if (!(ob->flags & O_SNOOP)) {
    return nullptr;
  }
  for (const auto& user : users()) {
    if (user->snooped_by == ob) {
      return user->ob;
    }
  }
  DEBUG_FATAL("couldn't find snoop target.\n");
  return nullptr;
} /* query_snooping() */
#endif

int query_idle(object_t* ob) {
  if (!ob->interactive) {
    error("query_idle() of non-interactive object.\n");
  }
  return (get_current_time() - ob->interactive->last_time);
} /* query_idle() */

const char* sockaddr_to_string(const sockaddr* addr, socklen_t len) {
  static char result[NI_MAXHOST + NI_MAXSERV];

  char host[NI_MAXHOST], service[NI_MAXSERV];
  int const ret = getnameinfo(addr, len, host, sizeof(host), service, sizeof(service),
                              NI_NUMERICHOST | NI_NUMERICSERV);

  if (ret) {
    debug(sockets, "sockaddr_to_string fail: %s.\n", gai_strerror(ret));
    strcpy(result, "<invalid address>");
    return result;
  }

  snprintf(result, sizeof(result), strchr(host, ':') != nullptr ? "[%s]:%s" : "%s:%s", host,
           service);

  return result;
}
