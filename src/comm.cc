/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "base/std.h"

#include "comm.h"

#include <stdarg.h>              // for va_end, va_list, va_copy, etc
#include <stdio.h>               // for snprintf, vsnprintf, fwrite, etc
#include <string.h>              // for NULL, memcpy, strlen, etc
#include <unistd.h>              // for gethostname
#include <memory>                // for unique_ptr

#include "cgo.autogen.h" // for Go exported stuff
#include "backend.h"
#include "fliconv.h"
#include "interactive.h"
#include "thirdparty/libtelnet/libtelnet.h"
#include "net/telnet.h"
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
static int call_function_interactive(interactive_t * /*i*/, char * /*str*/);
static void print_prompt(interactive_t * /*ip*/);

#ifdef NO_SNOOP
#define handle_snoop(str, len, who)
#else
#define handle_snoop(str, len, who) \
  if ((who)->snooped_by) receive_snoop(str, len, who->snooped_by)

static void receive_snoop(const char * /*buf*/, int /*len*/, object_t *ob);

#endif

void on_user_command(struct interactive_t *ip, char *command) {
  /* handle snooping - snooper does not see type-ahead due to telnet being in linemode */
  if (!(ip->iflags & NOECHO)) {
    handle_snoop(command, strlen(command), ip);
  }
  // FIXME: this function currently calls into mudlib and will throw errors
  // This catch block should be moved one level down.
  error_context_t econ;
  if (!save_context(&econ)) {
    fatal("BUG: on_user_comamnd can not save context!");
  }
  set_eval(max_cost);
  try {
    process_user_command(ip, command);
  } catch (const char *) {
    restore_context(&econ);
  }
  pop_context(&econ);

  /* Has to be cleared if we jumped out of process_user_command() */
  current_interactive = 0;
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
struct interactive_t* new_user_handler(int external_port_idx, int connIdx, char* remote_hostport) {
  auto port = external_port[external_port_idx];
  /*
   * initialize new user interactive data structure.
   */
  auto user = user_add();
  user->id = connIdx;
  user->connection_type = port.kind;
  user->ob = master_ob;
  user->last_time = get_current_time();

#ifdef USE_ICONV
  user->trans = get_translator("UTF-8");
#else
  user->trans = (struct translation *)master_ob;
// never actually used, but avoids multiple ifdefs later on!
#endif

  user->external_port = external_port_idx;

  user->remote_hostport = remote_hostport;

  set_command_giver(master_ob);
  master_ob->flags |= O_ONCE_INTERACTIVE;

  master_ob->interactive = user;

  set_prompt("> ");

  /*
   * The user object has one extra reference. It is asserted that the
   * master_ob is loaded.  Save a pointer to the master ob incase it
   * changes during APPLY_CONNECT.  We want to free the reference on
   * the right copy of the object.
   */
  object_t *master, *ob;
  svalue_t *ret;

  master = master_ob;
  add_ref(master_ob, "new_user");
  push_number(port.port);
  ret = safe_apply_master_ob(APPLY_CONNECT, 1);
  /* master_ob->interactive can be zero if the master object self
   destructed in the above (don't ask) */
  set_command_giver(0);
  if (ret == 0 || ret == (svalue_t *)-1 || ret->type != T_OBJECT || !master_ob->interactive) {
    debug_message("Can not accept connection from %s due to error in connect().\n",
                  user->remote_hostport);
    if (master_ob->interactive) {
      remove_interactive(master_ob, 0);
    }
    return nullptr;
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
  master_ob->interactive = 0;
  add_ref(ob, "new_user");

  // start reverse DNS probing.
  query_name_by_addr(ob);

  user->telnet = net_telnet_init(user);
  if (user->connection_type == PORT_TELNET) {
    send_initial_telnet_negotiations(user);
  }

  set_command_giver(ob);

  // Call logon() on the object.
  ret = safe_apply(APPLY_LOGON, ob, 0, ORIGIN_DRIVER);
  if (ret == NULL) {
    debug_message("new_user_handler: logon() on object %s has failed, the user is disconnected.\n",
                  ob->obname);
    destruct_object(ob);
    ob = NULL;
  } else if (ob->flags & O_DESTRUCTED) {
    // logon() may decide not to allow user connect by destroying objects.
  }
  set_command_giver(0);

  return user;
} /* new_user_handler() */

/*
 * Shut down new user accept file descriptor.
 */
void shutdown_external_ports() {
    // FIXME: close network connections
  debug_message("closed external ports\n");
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
static int shadow_catch_message(object_t *ob, const char *str) {
  if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
    return 0;
  }
  if (!ob->shadowed) {
    return 0;
  }
  while (ob->shadowed != 0 && ob->shadowed != current_object) {
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
void add_message(object_t *who, const char *data, int len) {
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

  auto ip = who->interactive;
  if (ip->connection_type == PORT_TELNET) {
    int translen;
    char *trans = translate(ip->trans->outgoing, data, len, &translen);

    telnet_send_text(ip->telnet, trans, translen);
  } else {
    ConnWrite(ip->id, ip->external_port, (char *)data, len);
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

void add_vmessage(object_t *who, const char *format, ...) {
  va_list args, args2;
  va_start(args, format);
  va_copy(args2, args);
  do {
    int result = vsnprintf(nullptr, 0, format, args);
    if (result < 0) break;
    std::unique_ptr<char[]> msg(new char[result + 1]);
    result = vsnprintf(msg.get(), result + 1, format, args2);
    if (result < 0) break;
    add_message(who, msg.get(), result);
  } while (0);
  va_end(args2);
  va_end(args);
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message(interactive_t *ip) {
  /*
   * if ip is not valid, do nothing.
   */
  if (!ip) {
    debug(connections, ("flush_message: invalid target!\n"));
    return 0;
  }

  // Flush things normally.
  ConnFlush(ip->id);
  return 0;
}

void flush_message_all() {
  users_foreach([](interactive_t *user) { flush_message(user); });
}

void on_mud_data(interactive_t *ip, char *command) {
      svalue_t value;
      if (restore_svalue(command, &value) == 0) {
        STACK_INC;
        *sp = value;
      } else {
        push_undefined();
      }
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
}

void on_ascii_data(interactive_t *ip, const char *command) {
    if (!(ip->ob->flags & O_DESTRUCTED)) {
      copy_and_push_string(command);
      safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
    }
}

void on_binary_data(interactive_t *ip, const char *buf, int num_bytes) {
  auto buffer = allocate_buffer(num_bytes);
  memcpy(buffer->item, buf, num_bytes);

  push_refed_buffer(buffer);
  safe_apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
}

static int escape_command(interactive_t *ip, char *user_command) {
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

static void process_input(interactive_t *ip, char *user_command) {
  svalue_t *ret;

  if (!(ip->iflags & HAS_PROCESS_INPUT)) {
    parse_command(user_command, command_giver);
    return;
  }

  /*
   * send a copy of user input back to user object to provide
   * support for things like command history and mud shell
   * programming languages.
   */
  copy_and_push_string(user_command);
  ret = apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
  if (!IP_VALID(ip, command_giver)) {
    return;
  }
  if (!ret) {
    ip->iflags &= ~HAS_PROCESS_INPUT;
    parse_command(user_command, command_giver);
    return;
  }

#ifndef NO_ADD_ACTION
  if (ret->type == T_STRING) {
    static char buf[MAX_TEXT];

    strncpy(buf, ret->u.string, MAX_TEXT - 1);
    parse_command(buf, command_giver);
  } else {
    if (ret->type != T_NUMBER || !ret->u.number) {
      parse_command(user_command, command_giver);
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
int process_user_command(interactive_t *ip, char* user_command) {
  if (!ip || !ip->ob || (ip->ob->flags & O_DESTRUCTED)) {
    return 0;
  }

  /* got a command - return it and set command_giver */
  debug(connections, "process_user_command: user_command = (%s)\n", user_command);
  save_command_giver(ip->ob);

  if ((ip->iflags & NOECHO) && !(ip->iflags & SINGLE_CHAR)) {
    /* must not enable echo before the user input is received */
    set_localecho(command_giver->interactive, true);
    ip->iflags &= ~NOECHO;
  }

  ip->last_time = get_current_time();


  if (ip != command_giver->interactive) {
    fatal("BUG: process_user_command.");
  }

  current_interactive = command_giver; /* this is yuck phooey, sigh */
  if (ip) {
    clear_notify(ip->ob);
  }

  // FIXME: move this to somewhere else
  update_load_av();

  debug(connections, "process_user_command: command_giver = /%s\n", command_giver->obname);

  user_command = translate_easy(ip->trans->incoming, user_command);

  if ((ip->iflags & USING_MXP) && user_command[0] == ' ' && user_command[1] == '[' &&
      user_command[3] == 'z') {
    svalue_t *ret;
    copy_and_push_string(user_command);

    ret = safe_apply(APPLY_MXP_TAG, ip->ob, 1, ORIGIN_DRIVER);
    if (ret && ret->type == T_NUMBER && ret->u.number) {
      goto exit;
    }
  }

  if (escape_command(ip, user_command)) {
    if (ip->iflags & SINGLE_CHAR) {
      /* only 1 char ... switch to line buffer mode */
      ip->iflags |= WAS_SINGLE_CHAR;
      ip->iflags &= ~SINGLE_CHAR;
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
    print_prompt(ip);
  }

  current_interactive = 0;
  restore_command_giver();
  return 1;
}

/*
 * Remove an interactive user immediately.
 */
void remove_interactive(object_t *ob, int dested) {
  /* don't have to worry about this dangling, since this is the routine
   * that causes this to dangle elsewhere, and we are protected from
   * getting called recursively by CLOSING.  safe_apply() should be
   * used here, since once we start this process we can't back out,
   * so jumping out with an error would be bad.
   */
  interactive_t *ip = ob->interactive;

  if (!ip) {
    return;
  }

  if (ip->iflags & CLOSING) {
    if (!dested) {
      debug_message("Double call to remove_interactive()\n");
    }
    return;
  }
  debug(connections, "Closing connection from %s.\n", ip->remote_hostport);
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
    safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);
    restore_command_giver();
  }

#ifndef NO_SNOOP
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
    ip->snooped_by = 0;
  }
#endif
  ConnClose(ip->id);
  ip->id = 0;
  // Free telnet handle
  if (ip->telnet != NULL) {
    telnet_free(ip->telnet);
    ip->telnet = NULL;
  }

  clear_notify(ip->ob);
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to) {
    free_object(&ip->input_to->ob, "remove_interactive");
    free_sentence(ip->input_to);
    if (ip->num_carry > 0) {
      free_some_svalues(ip->carryover, ip->num_carry);
    }
    ip->carryover = NULL;
    ip->num_carry = 0;
    ip->input_to = 0;
  }
#endif
  user_del(ip);
  FREE(ip);
  ob->interactive = 0;
  free_object(&ob, "remove_interactive");
  return;
} /* remove_interactive() */

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
static int call_function_interactive(interactive_t *i, char *str) {
  object_t *ob;
  funptr_t *funp;
  char *function;
  svalue_t *args;
  sentence_t *sent;
  int num_arg;
  int was_single = 0;
  int was_noecho = 0;

  i->iflags &= ~NOESC;
  if (!(sent = i->input_to)) {
    return (0);
  }

  /*
   * Special feature: input_to() has been called to setup a call to a
   * function.
   */
  if (sent->ob->flags & O_DESTRUCTED) {
    /* Sorry, the object has selfdestructed ! */
    free_object(&sent->ob, "call_function_interactive");
    free_sentence(sent);
    i->input_to = 0;
    if (i->num_carry) {
      free_some_svalues(i->carryover, i->num_carry);
    }
    i->carryover = NULL;
    i->num_carry = 0;
    i->input_to = 0;
    if (i->iflags & SINGLE_CHAR) {
      /*
       * clear single character mode
       */
      i->iflags &= ~SINGLE_CHAR;
      set_linemode(i, true);
      if (i->iflags & NOECHO) {
        i->iflags &= ~NOECHO;
        set_localecho(i, true);
      }
    }

    return (0);
  }
  /*
   * We must all references to input_to fields before the call to apply(),
   * because someone might want to set up a new input_to().
   */

  /* we put the function on the stack in case of an error */
  STACK_INC;
  if (sent->flags & V_FUNCTION) {
    function = 0;
    sp->type = T_FUNCTION;
    sp->u.fp = funp = sent->function.f;
    funp->hdr.ref++;
  } else {
    sp->type = T_STRING;
    sp->subtype = STRING_SHARED;
    sp->u.string = function = sent->function.s;
    ref_string(function);
  }
  ob = sent->ob;

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
    i->carryover = NULL;
  } else {
    args = NULL;
  }

  i->input_to = 0;
  if (i->iflags & SINGLE_CHAR) {
    /*
     * clear single character mode
     */
    i->iflags &= ~SINGLE_CHAR;
    was_single = 1;
    if (i->iflags & NOECHO) {
      was_noecho = 1;
      i->iflags &= ~NOECHO;
    }
  }

  // FIXME: this logic can be combined with above.
  if (was_single && !(i->iflags & SINGLE_CHAR)) {
    set_linemode(i, true);
  }
  if (was_noecho && !(i->iflags & NOECHO)) {
    set_localecho(i, true);
  }

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
    if (function[0] == APPLY___INIT_SPECIAL_CHAR) {
      error("Illegal function name.\n");
    }
    (void)safe_apply(function, ob, num_arg + 1, ORIGIN_INTERNAL);
  } else {
    safe_call_function_pointer(funp, num_arg + 1);
  }
  // NOTE: we can't use "i" here anymore, it is possible that it
  // has been freed.
  pop_stack(); /* remove `function' from stack */
  return (1);
} /* call_function_interactive() */

int set_call(object_t *ob, sentence_t *sent, int flags) {
  if (ob == 0 || sent == 0) {
    return (0);
  }
  if (ob->interactive == 0 || ob->interactive->input_to) {
    return (0);
  }
  ob->interactive->input_to = sent;
  ob->interactive->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
  if (flags & I_NOECHO) {
    set_localecho(ob->interactive, false);
  }
  if (flags & I_SINGLE_CHAR) {
    set_charmode(ob->interactive);
  }
  return (1);
} /* set_call() */
#endif

void set_prompt(const char *str) {
  if (command_giver && command_giver->interactive) {
    command_giver->interactive->prompt = str;
  }
} /* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt(interactive_t *ip) {
  object_t *ob = ip->ob;

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  if (ip->input_to == 0) {
#endif
    /* give user object a chance to write its own prompt */
    if (!(ip->iflags & HAS_WRITE_PROMPT)) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#ifdef OLD_ED
    else if (ip->ed_buffer) {
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#endif
    else if (!apply(APPLY_WRITE_PROMPT, ip->ob, 0, ORIGIN_DRIVER)) {
      if (!IP_VALID(ip, ob)) {
        return;
      }
      ip->iflags &= ~HAS_WRITE_PROMPT;
      tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
    }
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
  }
#endif
  if (!IP_VALID(ip, ob)) {
    return;
  }
  // Stavros: A lot of clients use this TELNET_GA to differentiate
  // prompts from other text
  if ((ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
    telnet_iac(ip->telnet, TELNET_GA);
  }
} /* print_prompt() */

#ifndef NO_SNOOP
static void receive_snoop(const char *buf, int len, object_t *snooper) {
  /* command giver no longer set to snooper */
  if (CONFIG_INT(__RC_RECEIVE_SNOOP__)) {
    char *str;

    str = new_string(len, "receive_snoop");
    memcpy(str, buf, len);
    str[len] = 0;
    push_malloced_string(str);
    apply(APPLY_RECEIVE_SNOOP, snooper, 1, ORIGIN_DRIVER);
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
int new_set_snoop(object_t *by, object_t *victim) {
  interactive_t *ip;
  object_t *tmp;

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
      users_foreach([by](interactive_t *user) {
        if (user->snooped_by == by) {
          user->snooped_by = 0;
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
    tmp = (tmp->interactive ? tmp->interactive->snooped_by : 0);
  }

  /*
   * Terminate previous snoop, if any.
   */
  new_set_snoop(by, NULL);

  // setup new snoop
  if (ip->snooped_by) {
    ip->snooped_by->flags &= ~O_SNOOP;
  }
  by->flags |= O_SNOOP;
  ip->snooped_by = by;

  return 1;
} /* set_new_snoop() */
#endif

char *query_host_name() {
  static char name[400];

  gethostname(name, sizeof(name));
  name[sizeof(name) - 1] = '\0'; /* Just to make sure */
  return (name);
} /* query_host_name() */

#ifndef NO_SNOOP
object_t *query_snoop(object_t *ob) {
  if (!ob->interactive) {
    return 0;
  }
  return ob->interactive->snooped_by;
} /* query_snoop() */

object_t *query_snooping(object_t *ob) {
  if (!(ob->flags & O_SNOOP)) {
    return 0;
  }
  for (auto &user : users()) {
    if (user->snooped_by == ob) {
      return user->ob;
    }
  }
  // TODO: change this to dfatal
  // fatal("couldn't find snoop target.\n");
  return 0;
} /* query_snooping() */
#endif

int query_idle(object_t *ob) {
  if (!ob->interactive) {
    error("query_idle() of non-interactive object.\n");
  }
  return (get_current_time() - ob->interactive->last_time);
} /* query_idle() */

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
  obfrom->interactive = 0;
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
#endif

#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type() {
  auto ip = command_giver->interactive;
  telnet_request_ttype(ip->telnet);
  flush_message(ip);
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type() {
  auto ip = command_giver->interactive;
  telnet_start_request_ttype(ip->telnet);
  flush_message(ip);
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size() {
  auto ip = command_giver->interactive;

  if ((st_num_arg == 1) && (sp->u.number == 0)) {
    telnet_dont_naws(ip->telnet);
  } else {
    telnet_do_naws(ip->telnet);
  }

  if (st_num_arg == 1) {
    sp--;
  }
  flush_message(ip);
}
#endif

#ifdef F_WEBSOCKET_HANDSHAKE_DONE
void f_websocket_handshake_done() {
  if (!current_interactive) {
    return;
  }
  auto ip = current_interactive->interactive;
  ip->iflags |= HANDSHAKE_COMPLETE;
  send_initial_telnet_negotiations(ip);
}
#endif