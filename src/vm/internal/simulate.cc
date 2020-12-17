#include "base/std.h"

#include "vm/internal/simulate.h"

#include <fcntl.h>     // for O_RDONLY
#include <stdlib.h>    // for exit
#include <sys/stat.h>  // for load_object struct stat
#include <stdarg.h>    // for va_start
#include <unistd.h>    // for open()
#ifdef HAVE_SIGNAL_H
#include <signal.h>  // for signal*
#include <net/telnet.h>
#endif
#ifdef _WIN32
#include <winsock.h>  // for WSACleanup()
#endif

#include "base/internal/tracing.h"

#include "applies_table.autogen.h"
#include "backend.h"      // for clear_tick_events , FIXME
#include "user.h"         // for users_foreach, FIXME
#include "interactive.h"  // for interactive_t, FIXME
#include "vm/internal/apply.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/master.h"
#include "vm/internal/otable.h"
#include "vm/internal/simul_efun.h"
#include "compiler/internal/lex.h"  // for total_lines, FIXME

#include "packages/core/add_action.h"
#include "packages/core/call_out.h"
#include "packages/core/ed.h"
#include "packages/core/file.h"
#include "packages/core/heartbeat.h"
#ifdef PACKAGE_ASYNC
#include "packages/async/async.h"
#endif
#ifdef PACKAGE_DB
void db_cleanup(void);  // FIXME
#endif
#ifdef PACKAGE_SOCKETS
#include "packages/sockets/socket_efuns.h"
#endif
#ifdef PACKAGE_UIDS
#include "packages/uids/uids.h"
#endif
#ifdef PACKAGE_PARSER
#include "packages/parser/parser.h"
#endif

#include "comm.h"  // FIXME

#include "vm/internal/trace.h"  // for dump_trace && get_svalue_trace
/*
 * This one is called from HUP.
 */
int MudOS_is_being_shut_down;

void startshutdownMudOS(int sig) { MudOS_is_being_shut_down = 1; }

/*
 * This one is called from the command "shutdown".
 * We don't call it directly from HUP, because it is dangerous when being
 * in an interrupt.
 */
void shutdownMudOS(int exit_code) {
  shout_string("FluffOS driver shouts: shutting down immediately.\n");

#ifdef PACKAGE_MUDLIB_STATS
  save_stat_files();
#endif
#ifdef PACKAGE_ASYNC
  complete_all_asyncio();
#endif

#ifdef PACKAGE_DB
  db_cleanup();
#endif
  shutdown_external_ports();

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
  lpc_socks_closeall();
#endif
  flush_message_all();

  /* clean up heap allocations so valgrind don't consider them lost.*/
  reset_machine(0);
  clear_call_outs();
  clear_tick_events();
  clear_heartbeats();
#ifdef PROFILING
  monitor(0, 0, 0, 0, 0); /* cause gmon.out to be written */
#endif
  Tracer::collect();

#ifdef _WIN32
  WSACleanup();
#endif

  exit(exit_code);
}

/* prevents infinite inherit loops.
 No, mark-and-sweep solution won't work.  Exercise for reader.  */
static int num_objects_this_thread = 0;

#ifndef NO_ENVIRONMENT
static object_t *restrict_destruct;
#endif

object_t *obj_list, *obj_list_destruct;
#ifdef DEBUG
object_t *obj_list_dangling = 0;
#endif
object_t *current_object;      /* The object interpreting a function. */
object_t *command_giver;       /* Where the current command came from. */
object_t *current_interactive; /* The user who caused this execution */

#ifdef PRIVS
static void init_privs_for_object(object_t *);
#endif
#ifdef PACKAGE_UIDS
static int give_uid_to_object(object_t * /*ob*/);
#endif
static int init_object(object_t * /*ob*/);
static object_t *load_virtual_object(const char * /*name*/, int /*clone*/);
static char *make_new_name(const char * /*str*/);
#ifndef NO_ENVIRONMENT
static void send_say(object_t * /*ob*/, const char * /*text*/, array_t * /*avoid*/);
#endif

void check_legal_string(const char *s) {
  if (strlen(s) > LARGEST_PRINTABLE_STRING) {
    error("Printable strings limited to length of %d.\n", LARGEST_PRINTABLE_STRING);
  }
}

#ifdef PRIVS
static void init_privs_for_object(object_t *ob) {
  svalue_t *value;

  if (!current_object
#ifdef PACKAGE_UIDS
      || !current_object->uid
#endif
  ) {
    ob->privs = NULL;
    return;
  }
  push_malloced_string(add_slash(ob->obname));

  if (master_ob) {
    value = apply_master_ob(APPLY_PRIVS_FILE, 1);
  } else {
    value = apply(applies_table[APPLY_PRIVS_FILE], ob, 1, ORIGIN_DRIVER);
  }

  if (value == NULL || value->type != T_STRING) {
    ob->privs = NULL;
  } else {
    ob->privs = make_shared_string(value->u.string);
  }
}
#endif /* PRIVS */

/*
 * Give the correct uid and euid to a created object.
 */
#ifdef PACKAGE_UIDS
static int give_uid_to_object(object_t *ob) {
  svalue_t *ret;
  const char *creator_name;

  if (!master_ob) {
    ob->uid = add_uid("NONAME");
#ifdef AUTO_SETEUID
    ob->euid = ob->uid;
#else
    ob->euid = NULL;
#endif
    return 1;
  }
  /*
   * Ask master object who the creator of this object is.
   */
  push_malloced_string(add_slash(ob->obname));

  ret = apply_master_ob(APPLY_CREATOR_FILE, 1);
  if (!ret) error("master object: No function %s() defined!\n", applies_table[APPLY_CREATOR_FILE]);
  if (!ret || ret == (svalue_t *)-1 || ret->type != T_STRING) {
    destruct_object(ob);
    if (!ret) {
      error("Master object has no function %s().\n", applies_table[APPLY_CREATOR_FILE]);
    }
    if (ret == (svalue_t *)-1) {
      error("Can't load objects without a master object.");
    }
    error(
        "Illegal object to load: return value of master::%s() was not a "
        "string.\n",
        applies_table[APPLY_CREATOR_FILE]);
  }
  creator_name = ret->u.string;
  /*
   * Now we are sure that we have a creator name. Do not call apply()
   * again, because creator_name will be lost !
   */
  if (strcmp(current_object->uid->name, creator_name) == 0) {
/*
 * The loaded object has the same uid as the loader.
 */
#ifndef COMPAT_32
    ob->uid = current_object->uid;
#else
    ob->uid = current_object->euid;
#endif
    ob->euid = current_object->euid;
    return 1;
  }
#ifdef AUTO_TRUST_BACKBONE
  if (strcmp(backbone_uid->name, creator_name) == 0) {
    /*
     * The object is loaded from backbone. This is trusted, so we let it
     * inherit the value of eff_user.
     */
    ob->uid = current_object->euid;
    ob->euid = current_object->euid;
    return 1;
  }
#endif

  /*
   * The object is not loaded from backbone, nor from from the loading
   * objects path. That should be an object defined by another wizard. It
   * can't be trusted, so we give it the same uid as the creator. Also give
   * it eff_user 0, which means that user 'a' can't use objects from user
   * 'b' to load new objects nor modify files owned by user 'b'.
   *
   * If this effect is wanted, user 'b' must let his object do 'seteuid()' to
   * himself. That is the case for most rooms.
   */
  ob->uid = add_uid(creator_name);
#ifdef AUTO_SETEUID
  ob->euid = ob->uid;
#else
  ob->euid = NULL;
#endif
  return 1;
}
#endif

static int init_object(object_t *ob) {
#ifdef PACKAGE_MUDLIB_STATS
  init_stats_for_object(ob);
#endif
#ifdef PRIVS
  init_privs_for_object(ob);
#endif /* PRIVS */
#ifdef PACKAGE_MUDLIB_STATS
  add_objects(&ob->stats, 1);
#endif
#ifdef NO_ADD_ACTION
  if (function_exists(APPLY_CATCH_TELL, ob, 1) || function_exists(APPLY_RECEIVE_MESSAGE, ob, 1)) {
    ob->flags |= O_LISTENER;
  }
#endif
#ifdef PACKAGE_UIDS
  return give_uid_to_object(ob);
#else
  return 1;
#endif
}

static object_t *load_virtual_object(const char *name, int clone) {
  int argc = 2;
  char *new_name;
  object_t *new_ob, *ob;
  array_t *args = nullptr;
  svalue_t *v;

  if (!master_ob) {
    if (clone) {
      pop_n_elems(clone - 1);
    }
    return nullptr;
  }

  if (clone > 1) {
    args = allocate_empty_array(clone - 1);
    while (clone-- > 1) {
      args->item[clone - 1] = *sp--;
    }
    argc++;
    clone = 1;
  }
  push_malloced_string(add_slash(name));
  push_number(clone);
  if (args) {
    push_refed_array(args);
  }
  v = apply_master_ob(APPLY_COMPILE_OBJECT, argc);
  if (!v || (v->type != T_OBJECT)) {
    return nullptr;
  }
  new_ob = v->u.ob;

  if (!clone) {
    ob = ObjectTable::instance().find(name);
    if (ob && ob != new_ob) {
      /*
       * If we rename, we're going to have a duplicate name here.  Don't
       * allow this.  Destruct the new object and raise an error.
       */
      destruct_object(new_ob);
      error("Virtual object name duplicates an existing object name: '%s'.\n", name);
    }
    /* Make sure O_CLONE is NOT set */
    new_ob->flags &= ~O_CLONE;
    new_name = alloc_cstring(name, "load_virtual_object");
    SET_TAG(new_name, TAG_OBJ_NAME);
  } else {
    /* Make sure O_CLONE is set */
    new_ob->flags |= O_CLONE;
    new_name = make_new_name(name);
  }

#ifdef PACKAGE_MUDLIB_STATS
  add_objects(&new_ob->stats, -1);
#endif

  /* perform the object rename */
  ObjectTable::instance().remove(new_ob->obname);
  if (new_ob->obname) {
    FREE((char *)new_ob->obname);
  }
  SETOBNAME(new_ob, new_name);
  ObjectTable::instance().insert(new_ob->obname, new_ob);

  /* finish initialization */
  new_ob->flags |= O_VIRTUAL;
  new_ob->load_time = get_current_time();
#ifdef PACKAGE_MUDLIB_STATS
  init_stats_for_object(new_ob);
  add_objects(&new_ob->stats, 1);
#endif
#ifdef PRIVS
  if (new_ob->privs) {
    free_string(new_ob->privs);
  }
  init_privs_for_object(new_ob);
#endif
#ifdef PACKAGE_UIDS
  /* reassign uid */
  give_uid_to_object(new_ob);
#endif
  apply(APPLY_VIRTUAL_START, new_ob, 0, ORIGIN_DRIVER);
  return new_ob;
}

int filename_to_obname(const char *src, char *dest, int size) {
  char last_c = 0;
  char *p = dest;
  char *end = dest + size - 1;

  while (*src == '/') {
    src++;
  }

  while (*src && p < end) {
    if (last_c == '/' && *src == '/') {
      src++;
    } else {
      last_c = (*p++ = *src++);
    }
  }

  /* In some cases, (for example, object loading) this currently gets
   * run twice, once in find_object, and once in load object.  The
   * net effect of this is:
   * /foo.c -> /foo [no such exists, try to load] -> /foo created
   * /foo.c.c -> /foo.c [no such exists, try to load] -> /foo created
   *
   * causing a duplicate object crash.  There are two ways to fix this:
   * (1) strip multiple .c's so that the output of this routine is something
   *     that doesn't change if this is run again.
   * (2) make sure this routine is only called once on any name.
   *
   * The first solution is the one currently in use.
   */
  while (p - dest > 2 && p[-1] == 'c' && p[-2] == '.') {
    p -= 2;
  }

  *p = 0;
  return 1;
}

/*
 * Load an object definition from file. If the object wants to inherit
 * from an object that is not loaded, discard all, load the inherited object,
 * and reload again.
 *
 * In mudlib3.0 when loading inherited objects, their reset() is not called.
 * - why is this??  it makes no sense and causes a problem when a developer
 * inherits code from a real used item.  Say a room for example.  In this case
 * the room is loaded but is never set up properly, so when someone enters it
 * it's all messed up.  Realistically, I know that it's pretty bad style to
 * inherit from an object that's actually being used and isn't just a building
 * block, but I see no reason for this limitation.  It happens, and when it
 * does occur, produces mysterious results than can be hard to track down.
 * for now, I've reenabled resetting.  We'll see if anything breaks. -WF
 *
 * Save the command_giver, because reset() in the new object might change
 * it.
 *
 */
object_t *load_object(const char *lname, int callcreate) {
  ScopedTracer _tracer("LPC Load Object", EventCategory::VM_LOAD_OBJECT, json{lname});

  auto inherit_chain_size = CONFIG_INT(__INHERIT_CHAIN_SIZE__);

  int f;
  program_t *prog;
  object_t *ob;
  svalue_t *mret;
  struct stat c_st;
  char real_name[400], name[400], actualname[400], obname[400];

  const char *pname = check_valid_path(lname, master_ob, "load_object", 0);
  if (!pname) {
    error("Read access denied.\n");
  }
  if (++num_objects_this_thread > inherit_chain_size) {
    error("Inherit chain too deep: > %d when trying to load '%s'.\n", inherit_chain_size, lname);
  }
#ifdef PACKAGE_UIDS
  if (current_object && current_object->euid == nullptr) {
    error("Can't load objects when no effective user.\n");
  }
#endif
  if (strrchr(pname, '#')) {
    error("Cannot load a clone.\n");
  }
  if (!filename_to_obname(lname, name, sizeof name)) {
    error("Filenames with consecutive /'s in them aren't allowed (%s).\n", lname);
  }
  if (!filename_to_obname(pname, actualname, sizeof actualname)) {
    error("Filenames with consecutive /'s in them aren't allowed (%s).\n", pname);
  }

  /*
   * First check that the c-file exists.
   */
  (void)strcpy(real_name, actualname);
  (void)strcat(real_name, ".c");

  (void)strcpy(obname, name);
  (void)strcat(obname, ".c");

  if (stat(real_name, &c_st) == -1 || S_ISDIR(c_st.st_mode)) {
    save_command_giver(command_giver);
    ob = load_virtual_object(actualname, 0);
    restore_command_giver();
    num_objects_this_thread--;
    return ob;
  }
  /*
   * Check if it's a legal name.
   */
  if (!legal_path(real_name)) {
    debug_message("Illegal pathname: /%s\n", real_name);
    error("Illegal path name '/%s'.\n", real_name);
  }

  f = open(real_name, O_RDONLY);
#ifdef _WIN32
  // TODO: change everything to use fopen instead.
  _setmode(f, _O_BINARY);
#endif
  if (f == -1) {
    debug_perror("compile_file", real_name);
    error("Could not read the file '/%s'.\n", real_name);
  }
  save_command_giver(command_giver);
  prog = compile_file(f, obname);
  restore_command_giver();
  update_compile_av(total_lines);
  total_lines = 0;
  close(f);

  /* Sorry, can't handle objects without programs yet. */
  if (inherit_file == nullptr && (num_parse_error > 0 || prog == nullptr)) {
    if (num_parse_error == 0 && prog == nullptr) {
      error("No program in object '/%s'!\n", name);
    }

    if (prog) {
      free_prog(&prog);
    }
    error("Error in loading object '/%s'\n", name);
  }
  /*
   * This is an iterative process. If this object wants to inherit an
   * unloaded object, then discard current object, load the object to be
   * inherited and reload the current object again. The global variable
   * "inherit_file" will be set by lang.y to point to a file name.
   */
  if (inherit_file) {
    object_t *inh_obj;
    char inhbuf[MAX_OBJECT_NAME_SIZE];

    if (!filename_to_obname(inherit_file, inhbuf, sizeof inhbuf)) {
      strcpy(inhbuf, inherit_file);
    }
    FREE(inherit_file);
    inherit_file = nullptr;

    if (prog) {
      free_prog(&prog);
      prog = nullptr;
    }
    if (strcmp(inhbuf, name) == 0) {
      error("Illegal to inherit self.\n");
    }

    if ((inh_obj = ObjectTable::instance().find(inhbuf))) {
#ifdef DEBUG
      fatal("Inherited object is already loaded!");
#endif
    } else {
      inh_obj = load_object(inhbuf, 1);
    }
    if (!inh_obj) error("Inherited file '/%s' does not exist!\n", inhbuf);

    /*
     * Yes, the following is necessary.  It is possible that when we
     * loaded the inherited object, it loaded this object from it's
     * create function. Without this check, that would crash the driver.
     * -Beek
     */
    if (!(ob = ObjectTable::instance().find(name))) {
      ob = load_object(name, 1);
      /* sigh, loading the inherited file removed us */
      if (!ob) {
        num_objects_this_thread--;
        return nullptr;
      }
      ob->load_time = get_current_time();
    }
    num_objects_this_thread--;
    return ob;
  }
  ob = get_empty_object(prog->num_variables_total);
  /* Shared string is no good here */
  SETOBNAME(ob, alloc_cstring(name, "load_object"));
  SET_TAG(ob->obname, TAG_OBJ_NAME);
  ob->prog = prog;
  ob->flags |= O_WILL_RESET; /* must be before reset is first called */
  ob->next_all = obj_list;
  ob->prev_all = nullptr;
  if (obj_list) {
    obj_list->prev_all = ob;
  }
  obj_list = ob;
  ObjectTable::instance().insert(ob->obname, ob); /* add name to fast object lookup table */
  save_command_giver(command_giver);
  push_object(ob);
  mret = apply_master_ob(APPLY_VALID_OBJECT, 1);
  if (mret && !MASTER_APPROVED(mret)) {
    destruct_object(ob);
    error("master object: %s() denied permission to load '/%s'.\n",
          applies_table[APPLY_VALID_OBJECT], name);
  }

  if (init_object(ob) && callcreate) {
    call_create(ob, 0);
  }
  if (!(ob->flags & O_DESTRUCTED) && function_exists(APPLY_CLEAN_UP, ob, 1)) {
    ob->flags |= O_WILL_CLEAN_UP;
  }
  restore_command_giver();

  if (ob) {
    debug(d_flag, "--/%s loaded.\n", ob->obname);
  }

  ob->load_time = get_current_time();
  num_objects_this_thread--;

  return ob;
}

static char *make_new_name(const char *str) {
  static unsigned int i;
  char *p = reinterpret_cast<char *>(DMALLOC(strlen(str) + 12, TAG_OBJ_NAME, "make_new_name"));

  (void)sprintf(p, "%s#%u", str, i);
  i++;
  return p;
}

/*
 * Save the command_giver, because reset() in the new object might change
 * it.
 */
object_t *clone_object(const char *str1, int num_arg) {
  object_t *ob, *new_ob;

#ifdef PACKAGE_UIDS
  if (current_object && current_object->euid == nullptr) {
    error("Object must call seteuid() prior to calling clone_object().\n");
  }
#endif

  save_command_giver(command_giver);

  num_objects_this_thread = 0;
  ob = find_object(str1);
  if (ob && !object_visible(ob)) {
    ob = nullptr;
  }
  /*
   * If the object self-destructed...
   */
  if (ob == nullptr) { /* fix from 3.1.1 */
    restore_command_giver();
    pop_n_elems(num_arg);
    return (nullptr);
  }

  if (ob->flags & O_CLONE) {
    error("Cannot clone from a clone\n");
  }

  if (ob->flags & O_VIRTUAL) {
    new_ob = load_virtual_object(ob->obname, 1 + num_arg);
    restore_command_giver();
    return new_ob;
    /*
     * we can skip all of the stuff below since we were already
     * cloned once to have gotten to this stage.
     */
  }

  /* We do not want the heart beat to be running for unused copied objects */
  if (ob->flags & O_HEART_BEAT) {
    (void)set_heart_beat(ob, 0);
  }
  new_ob = get_empty_object(ob->prog->num_variables_total);
  SETOBNAME(new_ob, make_new_name(ob->obname));
  new_ob->flags |= (O_CLONE | (ob->flags & (O_WILL_CLEAN_UP | O_WILL_RESET)));
  new_ob->load_time = ob->load_time;
  new_ob->prog = ob->prog;
  reference_prog(ob->prog, "clone_object");
  DEBUG_CHECK(!current_object, "clone_object() from no current_object !\n");

  new_ob->next_all = obj_list;
  obj_list->prev_all = new_ob;
  new_ob->prev_all = nullptr;
  obj_list = new_ob;
  ObjectTable::instance().insert(new_ob->obname, new_ob); /* Add name to fast object lookup table */
  init_object(new_ob);

  call_create(new_ob, num_arg);
  restore_command_giver();
  /* Never know what can happen ! :-( */
  if (new_ob->flags & O_DESTRUCTED) {
    return (nullptr);
  }
  return (new_ob);
}

#ifndef NO_ENVIRONMENT
object_t *environment(svalue_t *arg) {
  object_t *ob = current_object;

  if (arg && arg->type == T_OBJECT) {
    ob = arg->u.ob;
  }
  if (ob == nullptr || ob->super == nullptr || (ob->flags & O_DESTRUCTED)) {
    return nullptr;
  }
  if (ob->flags & O_DESTRUCTED) {
    error("environment() of destructed object.\n");
  }
  return ob->super;
}
#endif

/*
 * With no argument, present() looks in the inventory of the current_object,
 * the inventory of our super, and our super.
 * If the second argument is nonzero, only the inventory of that object
 * is searched.
 */

#ifdef F_PRESENT
static object_t *object_present2(const char * /*str*/, object_t * /*ob*/);

object_t *object_present(svalue_t *v, object_t *ob) {
  svalue_t *ret;
  object_t *ret_ob;
  int specific = 0;

  if (ob == nullptr) {
    ob = current_object;
  } else {
    specific = 1;
  }
  if (ob->flags & O_DESTRUCTED) {
    return nullptr;
  }
  if (v->type == T_OBJECT) {
    if (specific) {
      if (v->u.ob->super == ob) {
        return v->u.ob;
      } else {
        return nullptr;
      }
    }
    if (v->u.ob->super == ob || (v->u.ob->super == ob->super && ob->super != nullptr)) {
      return v->u.ob->super;
    }
    return nullptr;
  }
  ret_ob = object_present2(v->u.string, ob->contains);
  if (ret_ob) {
    return ret_ob;
  }
  if (specific) {
    return nullptr;
  }
  if (ob->super) {
    push_svalue(v);
    ret = apply(APPLY_ID, ob->super, 1, ORIGIN_DRIVER);
    if (ob->super->flags & O_DESTRUCTED) {
      return nullptr;
    }
    if (!IS_ZERO(ret)) {
      return ob->super;
    }
    return object_present2(v->u.string, ob->super->contains);
  }
  return nullptr;
}
// If string doesn't end with number, then Look for a object that
// id(str) returns true, return that object.
// If string is in format of "xxx 1", then look for the <digits>-th
// object that id("xx") returns true.
static object_t *object_present2(const char *str, object_t *ob) {
  svalue_t *ret;

  const char *name = nullptr;
  int namelen = 0, count = 0;

  if (str[0] == '\0') {
    return nullptr;
  }

  // default case
  name = str;
  namelen = strlen(str);

  // If string ends with number, try to separate name and count.
  if (uisdigit(name[namelen - 1])) {
    const char *ptr = strrchr(name, ' ');
    if (ptr != nullptr) {
      // Make sure second part has only digits
      if (strspn(ptr + 1, "1234567890") == strlen(ptr + 1)) {
        namelen = ptr - name;
        count = atoi(ptr + 1);
      }
    }
  }

  for (; ob; ob = ob->next_inv) {
    char *str_to_push = new_string(namelen, "object_present2");
    memcpy(str_to_push, name, namelen);
    str_to_push[namelen] = 0;
    push_malloced_string(str_to_push);

    ret = apply(APPLY_ID, ob, 1, ORIGIN_DRIVER);
    if (ob->flags & O_DESTRUCTED) {
      return nullptr;
    }
    if (IS_ZERO(ret)) {
      continue;
    }
    if (--count > 0) {
      continue;
    }
    return ob;
  }
  return nullptr;
}
#endif

static const char *saved_master_name;
static const char *saved_simul_name;

static void fix_object_names() {
  SETOBNAME(master_ob, saved_master_name);
  SETOBNAME(simul_efun_ob, saved_simul_name);
}

/*
 * Remove an object. It is first moved into the destruct list, and
 * not really destructed until later. (see destruct2()).
 */
void destruct_object(object_t *ob) {
  object_t **pp;
// int removed;
#ifndef NO_ENVIRONMENT
  object_t *super;
  object_t *save_restrict_destruct = restrict_destruct;

  if (restrict_destruct && restrict_destruct != ob) {
    error("Only this_object() can be destructed from move_or_destruct.\n");
  }
#endif

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
  /*
   * check if object has an efun socket referencing it for a callback. if
   * so, close the efun socket.
   */
  if (ob->flags & O_EFUN_SOCKET) {
    close_referencing_sockets(ob);
  }
#endif
#ifdef PACKAGE_PARSER
  if (ob->pinfo) {
    parse_free(ob->pinfo);
    ob->pinfo = nullptr;
  }
#endif

  if (ob->flags & O_DESTRUCTED) {
    return;
  }
  remove_object_from_stack(ob);
/*
 * If this is the first object being shadowed by another object, then
 * destruct the whole list of shadows.
 */
#ifndef NO_SHADOWS
  if (ob->shadowed && !ob->shadowing) {
    object_t *otmp;
    object_t *ob2;

    /*
     * move from bottom to top of shadow chain
     */
    for (ob2 = ob->shadowed; ob2->shadowed; ob2 = ob2->shadowed) {
      ;
    }
    /*
     * remove shadows top...down being careful to unlink shadow being
     * destructed from chain
     */
    for (; ob2;) {
      otmp = ob2;
      ob2 = ob2->shadowing;
      if (ob2) {
        ob2->shadowed = nullptr;
      }
      otmp->shadowing = nullptr;
      destruct_object(otmp);
    }
    return;
  }
  /*
   * The chain of shadows is a double linked list. Take care to update it
   * correctly.
   */
  if (ob->shadowing) {
    ob->shadowing->shadowed = ob->shadowed;
  }
  if (ob->shadowed) {
    ob->shadowed->shadowing = ob->shadowing;
  }
  ob->shadowing = nullptr;
  ob->shadowed = nullptr;
#endif

  debug(d_flag, "Deobject_t /%s (ref %d)", ob->obname, ob->ref);

#ifndef NO_ENVIRONMENT
  /* try to move our contents somewhere */
  super = ob->super;

  while (ob->contains) {
    object_t *otmp;

    otmp = ob->contains;
    /*
     * An error here will not leave destruct() in an inconsistent
     * stage.
     */
    push_object(super);

    restrict_destruct = ob->contains;
    (void)apply(APPLY_MOVE, ob->contains, 1, ORIGIN_DRIVER);
    restrict_destruct = save_restrict_destruct;
    /* OUCH! we could be dested by this. -Beek */
    if (ob->flags & O_DESTRUCTED) {
      return;
    }
    if (otmp == ob->contains) {
      destruct_object(otmp);
    }
  }
#endif

#ifdef PACKAGE_MUDLIB_STATS
  add_objects(&ob->stats, -1);
#endif
#ifdef OLD_ED
  if (ob->interactive && ob->interactive->ed_buffer) {
    save_ed_buffer(ob);
  }
#else
  if (ob->flags & O_IN_EDIT) {
    object_save_ed_buffer(ob);
    ob->flags &= ~O_IN_EDIT;
  }
#endif
#ifndef NO_SNOOP
  if (ob->flags & O_SNOOP) {
    users_foreach([ob](interactive_t *user) {
      if (user->snooped_by == ob) {
        user->snooped_by = nullptr;
      }
    });
    ob->flags &= ~O_SNOOP;
  }
#endif
#ifndef NO_ENVIRONMENT
  /*
   * Remove us out of this current room (if any). Remove all sentences
   * defined by this object from all objects here.
   */
  if (ob->super) {
#ifndef NO_LIGHT
    add_light(ob->super, -ob->total_light);
#endif
    remove_sent(ob->super, ob);
    remove_sent(ob, ob->super);
    for (pp = &ob->super->contains; *pp;) {
      remove_sent(*pp, ob);
      if (*pp != ob) {
        remove_sent(ob, *pp);
        pp = &(*pp)->next_inv;
      } else {
        *pp = (*pp)->next_inv;
      }
    }
  }
#endif

  /* At this point, we can still back out, but this is the very last
   * minute we can do so.  Make sure we have a new object to replace
   * us if this is a vital object.
   */
  if (ob == master_ob || ob == simul_efun_ob) {
    object_t *new_ob, *tmp_ob;
    const char *tmp = ob->obname;

    STACK_INC;
    sp->type = T_ERROR_HANDLER;
    sp->u.error_handler = fix_object_names;
    saved_master_name = master_ob->obname;
    saved_simul_name = simul_efun_ob->obname;

    /* hack to make sure we don't find ourselves at several points
     in the following process */
    SETOBNAME(ob, "");

    /* handle these two carefully, since they are rather vital */
    new_ob = load_object(tmp, 1);
    if (!new_ob) {
      SETOBNAME(ob, tmp);
      sp--;
      error("Destruct on vital object failed: new copy failed to reload.");
    }

    if (ob == master_ob) {
      set_master(new_ob);
    }
    if (ob == simul_efun_ob) {
      set_simul_efun(new_ob);
    }

    /* Set the name back so we can remove it from the hash table.
     Also be careful not to remove the new object, which has
     the same name. */
    sp--; /* error handler */
    SETOBNAME(ob, tmp);
    tmp = new_ob->obname;
    SETOBNAME(new_ob, "");
    ObjectTable::instance().remove(ob->obname);
    SETOBNAME(new_ob, tmp);
    ObjectTable::instance().insert(new_ob->obname, new_ob);
    tmp_ob = ob;
    free_object(&tmp_ob, "vital object reference");
    // still need ob below!
  } else {
    ObjectTable::instance().remove(ob->obname);
  }

  /*
   * Now remove us out of the list of all objects. This must be done last,
   * because an error in the above code would halt execution.
   */
  // removed = 0;
  if (ob->prev_all) {
    ob->prev_all->next_all = ob->next_all;
    if (ob->next_all) {
      ob->next_all->prev_all = ob->prev_all;
    }
  } else {
    obj_list = ob->next_all;
    obj_list->prev_all = nullptr;
  }
  /*
   for (pp = &obj_list; *pp; pp = &(*pp)->next_all) {
   if (*pp != ob)
   continue;
   *pp = (*pp)->next_all;
   removed = 1;
   break;
   }
   DEBUG_CHECK(!removed, "Failed to delete object.\n");//*/

  remove_living_name(ob);
#ifndef NO_ENVIRONMENT
  ob->super = nullptr;
  ob->next_inv = nullptr;
  ob->contains = nullptr;
#endif
  ob->next_all = obj_list_destruct;
  if (obj_list_destruct) {
    obj_list_destruct->prev_all = ob;
  }
  ob->prev_all = nullptr;
  obj_list_destruct = ob;
  set_heart_beat(ob, 0);
  ob->flags |= O_DESTRUCTED;
  /* moved this here from destruct2() -- see comments in destruct2() */
  if (ob->interactive) {
    remove_interactive(ob, 1);
  }
#ifdef F_SET_HIDE
  if (ob->flags & O_HIDDEN) {
    num_hidden--;
  }
#endif
}

/*
 * This one is called when no program is executing from the main loop.
 */
void destruct2(object_t *ob) {
#ifndef NO_ADD_ACTION
  sentence_t *s;
#endif

  debug(d_flag, "Destruct-2 object /%s (ref %d)", ob->obname, ob->ref);

  /*
   * We must deallocate variables here, not in 'free_object()'. That is
   * because one of the local variables may point to this object, and
   * deallocation of this pointer will also decrease the reference count of
   * this object. Otherwise, an object with a variable pointing to itself,
   * would never be freed. Just in case the program in this object would
   * continue to execute, change string and object variables into the
   * number 0.
   */
  if (ob->prog && ob->prog->num_variables_total > 0) {
    /*
     * Deallocate variables in this object. The space of the variables
     * are not deallocated until the object structure is freed in
     * free_object().
     */
    int i;

    for (i = 0; i < ob->prog->num_variables_total; i++) {
      free_svalue(&ob->variables[i], "destruct2");
      ob->variables[i] = const0u;
    }
  }

#ifndef NO_ADD_ACTION
  /*
   * For much the same reason as described above, we must remove sentences
   * for this object.  The reason is because the sentence callback could be
   * a function pointer that is bound to this object and thus holds a
   * reference to this object.
   */
  for (s = ob->sent; s;) {
    sentence_t *next;

    next = s->next;
    free_sentence(s);
    s = next;
  }
  ob->sent = nullptr;
#endif

#ifdef DEBUG
  tot_dangling_object++;
  ob->next_all = obj_list_dangling;
  if (obj_list_dangling) {
    obj_list_dangling->prev_all = ob;
  }
  obj_list_dangling = ob;
  ob->prev_all = 0;
#endif

  free_object(&ob, "destruct_object");
}

/*
 * say() efun - send a message to:
 *  all objects in the inventory of the source,
 *  all objects in the same environment as the source,
 *  and the object surrounding the source.
 *
 * when there is no command_giver, current_object is used as the source,
 *  otherwise, command_giver is used.
 *
 * message never goes to objects in the avoid array, or the source itself.
 *
 * rewritten, bobf@metronet.com (Blackthorn) 9/6/93
 */

#ifndef NO_ENVIRONMENT
static void send_say(object_t *ob, const char *text, array_t *avoid) {
  int valid, j;

  for (valid = 1, j = 0; j < avoid->size; j++) {
    if (avoid->item[j].type != T_OBJECT) {
      continue;
    }
    if (avoid->item[j].u.ob == ob) {
      valid = 0;
      break;
    }
  }

  if (!valid) {
    return;
  }

  tell_object(ob, text, strlen(text));
}

void say(svalue_t *v, array_t *avoid) {
  object_t *ob, *origin;
  const char *buff;

  check_legal_string(v->u.string);
  buff = v->u.string;

  if (current_object->flags & O_LISTENER || current_object->interactive) {
    save_command_giver(current_object);
  } else {
    save_command_giver(command_giver);
  }
  if (command_giver) {
    origin = command_giver;
  } else {
    origin = current_object;
  }

  /* To our surrounding object... */
  if ((ob = origin->super)) {
    if (ob->flags & O_LISTENER || ob->interactive) {
      send_say(ob, buff, avoid);
    }

    /* And its inventory... */
    for (ob = origin->super->contains; ob; ob = ob->next_inv) {
      if (ob != origin && (ob->flags & O_LISTENER || ob->interactive)) {
        send_say(ob, buff, avoid);
        if (ob->flags & O_DESTRUCTED) {
          break;
        }
      }
    }
  }
  /* Our inventory... */
  for (ob = origin->contains; ob; ob = ob->next_inv) {
    if (ob->flags & O_LISTENER || ob->interactive) {
      send_say(ob, buff, avoid);
      if (ob->flags & O_DESTRUCTED) {
        break;
      }
    }
  }

  restore_command_giver();
}

/*
 * Sends a string to all objects inside of a specific object.
 * Revised, bobf@metronet.com 9/6/93
 */
#ifdef F_TELL_ROOM
void tell_room(object_t *room, svalue_t *v, array_t *avoid) {
  object_t *ob;
  const char *buff;
  int valid, j;
  char txt_buf[LARGEST_PRINTABLE_STRING + 1];

  switch (v->type) {
    case T_STRING:
      check_legal_string(v->u.string);
      buff = v->u.string;
      break;
    case T_OBJECT:
      buff = v->u.ob->obname;
      break;
    case T_NUMBER:
      buff = txt_buf;
      sprintf(txt_buf, "%" LPC_INT_FMTSTR_P, v->u.number);
      break;
    case T_REAL:
      buff = txt_buf;
      sprintf(txt_buf, "%" LPC_FLOAT_FMTSTR_P, v->u.real);
      break;
    default:
      bad_argument(v, T_OBJECT | T_NUMBER | T_REAL | T_STRING, 2, F_TELL_ROOM);
#ifdef DEBUG
      buff = 0;
#endif
  }

  for (ob = room->contains; ob; ob = ob->next_inv) {
    if (!ob->interactive && !(ob->flags & O_LISTENER)) {
      continue;
    }

    for (valid = 1, j = 0; j < avoid->size; j++) {
      if (avoid->item[j].type != T_OBJECT) {
        continue;
      }
      if (avoid->item[j].u.ob == ob) {
        valid = 0;
        break;
      }
    }

    if (!valid) {
      continue;
    }

    if (!ob->interactive) {
      tell_npc(ob, buff);
      if (ob->flags & O_DESTRUCTED) {
        break;
      }
    } else {
      tell_object(ob, buff, strlen(buff));
      if (ob->flags & O_DESTRUCTED) {
        break;
      }
    }
  }
}
#endif
#endif

void shout_string(const char *str) {
  object_t *ob;

  check_legal_string(str);

  for (ob = obj_list; ob; ob = ob->next_all) {
    if (!(ob->flags & O_LISTENER) || (ob == command_giver)
#ifndef NO_ENVIRONMENT
        || !ob->super
#endif
    ) {
      continue;
    }
    tell_object(ob, str, strlen(str));
  }
}

#ifdef F_INPUT_TO
/*
 * Set up a function in this object to be called with the next
 * user input string.
 */
int input_to(svalue_t *fun, int flag, int num_arg, svalue_t *args) {
  sentence_t *s;
  svalue_t *x;
  int i;

  if (!command_giver || command_giver->flags & O_DESTRUCTED) {
    return 0;
  }
  s = alloc_sentence();
  if (set_call(command_giver, s, flag & ~I_SINGLE_CHAR)) {
    /*
     * If we have args, we copy them, and adjust the stack automatically
     * (elsewhere) to avoid double free_svalue()'s
     */
    if (num_arg) {
      i = num_arg * sizeof(svalue_t);
      if ((x = reinterpret_cast<svalue_t *>(DMALLOC(i, TAG_INPUT_TO, "input_to: 1"))) == nullptr) {
        fatal("Out of memory!\n");
      }
      memcpy(x, args, i);
    } else {
      x = nullptr;
    }

    command_giver->interactive->carryover = x;
    command_giver->interactive->num_carry = num_arg;
    if (fun->type == T_STRING) {
      s->function.s = make_shared_string(fun->u.string);
      s->flags = 0;
    } else {
      s->function.f = fun->u.fp;
      fun->u.fp->hdr.ref++;
      s->flags = V_FUNCTION;
    }
    s->ob = current_object;
    add_ref(current_object, "input_to");

    // need to send out our own GA
    if (auto ip = command_giver->interactive) {
      if (ip->telnet && (ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA)) {
        telnet_send_ga(ip->telnet);
      }
    }
    return 1;
  }
  free_sentence(s);
  return 0;
}
#endif

/*
 * Set up a function in this object to be called with the next
 * user input character.
 */
#ifdef F_GET_CHAR
int get_char(svalue_t *fun, int flag, int num_arg, svalue_t *args) {
  sentence_t *s;
  svalue_t *x;
  int i;

  if (!command_giver || command_giver->flags & O_DESTRUCTED) {
    return 0;
  }
  s = alloc_sentence();
  if (set_call(command_giver, s, flag | I_SINGLE_CHAR)) {
    /*
     * If we have args, we copy them, and adjust the stack automatically
     * (elsewhere) to avoid double free_svalue()'s
     */
    if (num_arg) {
      i = num_arg * sizeof(svalue_t);
      if ((x = reinterpret_cast<svalue_t *>(DMALLOC(i, TAG_TEMPORARY, "get_char: 1"))) == nullptr) {
        fatal("Out of memory!\n");
      }
      memcpy(x, args, i);
    } else {
      x = nullptr;
    }

    command_giver->interactive->carryover = x;
    command_giver->interactive->num_carry = num_arg;
    if (fun->type == T_STRING) {
      s->function.s = make_shared_string(fun->u.string);
      s->flags = 0;
    } else {
      s->function.f = fun->u.fp;
      fun->u.fp->hdr.ref++;
      s->flags = V_FUNCTION;
    }
    s->ob = current_object;
    add_ref(current_object, "get_char");
    return 1;
  }
  free_sentence(s);
  return 0;
}
#endif

void print_svalue(svalue_t *arg) {
  char tbuf[LARGEST_PRINTABLE_STRING + 1];
  int len;

  if (arg == nullptr) {
    tell_object(command_giver, "<NULL>", 6);
  } else {
    switch (arg->type) {
      case T_STRING:
        len = SVALUE_STRLEN(arg);
        if (len > LARGEST_PRINTABLE_STRING) {
          error("Printable strings limited to length of %d.\n", LARGEST_PRINTABLE_STRING);
        }

        tell_object(command_giver, arg->u.string, len);
        break;
      case T_OBJECT:
        sprintf(tbuf, "OBJ(/%s)", arg->u.ob->obname);
        tell_object(command_giver, tbuf, strlen(tbuf));
        break;
      case T_NUMBER:
        sprintf(tbuf, "%" LPC_INT_FMTSTR_P, arg->u.number);
        tell_object(command_giver, tbuf, strlen(tbuf));
        break;
      case T_REAL:
        sprintf(tbuf, "%" LPC_FLOAT_FMTSTR_P, arg->u.real);
        tell_object(command_giver, tbuf, strlen(tbuf));
        break;
      case T_ARRAY:
        tell_object(command_giver, "<ARRAY>", strlen("<ARRAY>"));
        break;
      case T_MAPPING:
        tell_object(command_giver, "<MAPPING>", strlen("<MAPPING>"));
        break;
      case T_FUNCTION:
        tell_object(command_giver, "<FUNCTION>", strlen("<FUNCTION>"));
        break;
      case T_BUFFER:
        tell_object(command_giver, "<BUFFER>", strlen("<BUFFER>"));
        break;
      default:
        tell_object(command_giver, "<UNKNOWN>", strlen("<UNKNOWN>"));
        break;
    }
  }
  return;
}

void do_write(svalue_t *arg) {
  object_t *ob = command_giver;

#ifndef NO_SHADOWS
  if (ob == nullptr && current_object->shadowing) {
    ob = current_object;
  }
  if (ob) {
    /* Send the message to the first object in the shadow list */
    while (ob->shadowing) {
      ob = ob->shadowing;
    }
  }
#else
  if (!ob) {
    ob = current_object;
  }
#endif /* NO_SHADOWS */
  save_command_giver(ob);
  print_svalue(arg);
  restore_command_giver();
}

/* Find an object. If not loaded, load it !
 * The object may selfdestruct, which is the only case when 0 will be
 * returned.
 */

object_t *find_object(const char *str) {
  object_t *ob;
  char tmpbuf[MAX_OBJECT_NAME_SIZE];

  if (!filename_to_obname(str, tmpbuf, sizeof tmpbuf)) {
    return nullptr;
  }

  if ((ob = ObjectTable::instance().find(tmpbuf))) {
    return ob;
  }
  ob = load_object(tmpbuf, 1);
  if (!ob || (ob->flags & O_DESTRUCTED)) { /* *sigh* */
    return nullptr;
  }
  return ob;
}

/* Look for a loaded object. Return 0 if non found. */
object_t *find_object2(const char *str) {
  object_t *ob;
  char p[MAX_OBJECT_NAME_SIZE];

  if (!filename_to_obname(str, p, sizeof p)) {
    return nullptr;
  }

  if ((ob = ObjectTable::instance().find(p))) {
    return ob;
  }
  return nullptr;
}

#ifndef NO_ENVIRONMENT
/*
 * Transfer an object.
 * The object has to be taken from one inventory list and added to another.
 * The main work is to update all command definitions, depending on what is
 * living or not. Note that all objects in the same inventory are affected.
 */
void move_object(object_t *item, object_t *dest) {
  object_t **pp, *ob;

  save_command_giver(command_giver);

  /* Recursive moves are not allowed. */
  for (ob = dest; ob; ob = ob->super) {
    if (ob == item) {
      error("Can't move object inside itself.\n");
    }
  }
#ifndef NO_SHADOWS
  if (item->shadowing) {
    error("Can't move an object that is shadowing.\n");
  }
#endif

  if (!CONFIG_INT(__RC_NO_RESETS__) && CONFIG_INT(__RC_LAZY_RESETS__)) {
    try_reset(dest);
  }
#ifndef NO_LIGHT
  add_light(dest, item->total_light);
#endif
  if (item->super) {
    int okay = 0;

    remove_sent(item->super, item);
    remove_sent(item, item->super);
#ifndef NO_LIGHT
    add_light(item->super, -item->total_light);
#endif
    for (pp = &item->super->contains; *pp;) {
      if (*pp != item) {
        remove_sent(item, *pp);
        remove_sent(*pp, item);
        pp = &(*pp)->next_inv;
        continue;
      }
      /*
       * unlink object from original inventory list
       */
      *pp = item->next_inv;
      okay = 1;
    }
#ifdef DEBUG
    if (!okay) {
      fatal("Failed to find object /%s in super list of /%s.\n", item->obname, item->super->obname);
    }
#endif
  }
  /*
   * link object into target's inventory list
   */
  item->next_inv = dest->contains;
  dest->contains = item;
  item->super = dest;

  setup_new_commands(dest, item);
  restore_command_giver();
}
#endif

#ifndef NO_LIGHT
/*
 * Every object has a count of the number of light sources it contains.
 * Update this.
 */

void add_light(object_t *p, int n) {
  if (n == 0) {
    return;
  }
  p->total_light += n;
#ifndef NO_ENVIRONMENT
  while ((p = p->super)) {
    p->total_light += n;
  }
#endif
}
#endif

static sentence_t *sent_free = nullptr;
uint64_t tot_alloc_sentence;

sentence_t *alloc_sentence() {
  sentence_t *p;

  if (sent_free == nullptr) {
    p = reinterpret_cast<sentence_t *>(DMALLOC(sizeof(sentence_t), TAG_SENTENCE, "alloc_sentence"));
    tot_alloc_sentence++;
  } else {
    p = sent_free;
    sent_free = sent_free->next;
  }
#ifndef NO_ADD_ACTION
  p->verb = nullptr;
#endif
  p->function.s = nullptr;
  p->next = nullptr;
  p->ob = nullptr;
  p->flags = 0;
  return p;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_free_sentences() {
  sentence_t *sent = sent_free;

  while (sent) {
    DO_MARK(sent, TAG_SENTENCE);
    /* Freed sentences should have been freed.  right?
     if (sent->function)
     EXTRA_REF(BLOCK(sent->function))++;
     if (sent->verb)
     EXTRA_REF(BLOCK(sent->verb))++;
     */
    sent = sent->next;
  }
}
#endif

void free_sentence(sentence_t *p) {
  if (p->flags & V_FUNCTION) {
    if (p->function.f) {
      free_funp(p->function.f);
    } else {
      p->function.f = nullptr;
    }
  } else {
    if (p->function.s) {
      free_string(p->function.s);
    } else {
      p->function.s = nullptr;
    }
  }
#ifndef NO_ADD_ACTION
  if (p->verb) {
    free_string(p->verb);
  }
  p->verb = nullptr;
#endif
  p->next = sent_free;
  sent_free = p;
}

[[noreturn]] void fatal(const char *fmt, ...) {
  static int in_fatal = 0;
  char msg_buf[2049];
  va_list args;

  switch (in_fatal) {
    default:
      debug_message("Fatal error while shutting down.  Aborting.\n");
      break;
    case 0:
      in_fatal = 1;
      va_start(args, fmt);
      vsnprintf(msg_buf, 2048, fmt, args);
      va_end(args);
      debug_message("******** FATAL ERROR: %s\n", msg_buf);
#ifdef DEBUG
      // make DEBUG driver directly crash, if there is debugger
      // it will catch the problem and allow debugging.
      break;
#endif
      if (Tracer::enabled()) {
        Tracer::collect();
      }
      debug_message("FluffOS driver attempting to exit gracefully.\n");
      if (current_file) {
        debug_message("(occurred during compilation of %s at line %d)\n", current_file,
                      current_line);
      }
      if (current_object) {
        debug_message("(current object was /%s)\n", current_object->obname);
      }

      dump_trace(1);
#ifdef PACKAGE_MUDLIB_STATS
      save_stat_files();
#endif
    /* fall through */
    case 1:
      in_fatal = 2;
      if (CONFIG_INT(__RC_TRAP_CRASHES__)) {
        copy_and_push_string(msg_buf);
        push_object(command_giver);
        push_object(current_object);
        set_eval(0x7fffffff);
        safe_apply_master_ob(APPLY_CRASH, 3);
        debug_message("crash() in master called successfully.  Aborting.\n");
      }
  }

  in_fatal = 0;

  signal(SIGABRT, SIG_DFL);
  std::abort();
}

static int num_error = 0;
static int num_mudlib_error = 0;

/*
 * Error() has been "fixed" so that users can catch and throw them.
 * To catch them nicely, we really have to provide decent error information.
 * Hence, all errors that are to be caught
 * (error_recovery_context_exists == 2) construct a string containing
 * the error message, which is returned as the
 * thrown value.  Users can throw their own error values however they choose.
 */

/*
 * This is here because throw constructs its own return value; we dont
 * want to replace it with the system's error string.
 */

[[noreturn]] void throw_error() {
  if (((current_error_context->save_csp + 1)->framekind & FRAME_MASK) == FRAME_CATCH) {
    throw("throw error");
    fatal("Throw_error failed!");
  }
  error("Throw with no catch.\n");
}

static void debug_message_with_location(char *err) {
  if (current_object && current_prog) {
    debug_message("%sprogram: /%s, object: /%s, file: %s\n", err, current_prog->filename,
                  current_object->obname, get_line_number(pc, current_prog));
  } else if (current_object) {
    debug_message("%sprogram: (none), object: /%s, file: (none)\n", err, current_object->obname);
  } else {
    debug_message("%sprogram: (none), object: (none), file: (none)\n", err);
  }
}

static void add_message_with_location(char *err) {
  if (current_object && current_prog) {
    add_vmessage(command_giver, "%sprogram: /%s, object: /%s, file: %s\n", err,
                 current_prog->filename, current_object->obname, get_line_number(pc, current_prog));
  } else if (current_object) {
    add_vmessage(command_giver, "%sprogram: (none), object: /%s, file: (none)\n", err,
                 current_object->obname);
  } else {
    add_vmessage(command_giver, "%sprogram: (none), object: (none), file: (none)\n", err);
  }
}

static void mudlib_error_handler(char *err, int katch) {
  mapping_t *m;
  const char *file = nullptr;
  int line = 0;
  svalue_t *mret;

  m = allocate_mapping(6);
  add_mapping_string(m, "error", err);
  if (current_prog) {
    add_mapping_malloced_string(m, "program", add_slash(current_prog->filename));
  }
  if (current_object) {
    add_mapping_object(m, "object", current_object);
  }
  add_mapping_array(m, "trace", get_svalue_trace());
  if (current_prog) {
    get_line_number_info(&file, &line);
  }
  if (file) {
    add_mapping_malloced_string(m, "file", add_slash(file));
  }
  if (line) {
    add_mapping_pair(m, "line", line);
  }

  push_refed_mapping(m);
  if (katch) {
    STACK_INC;
    *sp = const1;
    mret = apply_master_ob(APPLY_ERROR_HANDLER, 2);
  } else {
    mret = apply_master_ob(APPLY_ERROR_HANDLER, 1);
  }
  if ((mret == (svalue_t *)-1) || !mret) {
    debug_message("No error handler for error: ");
    debug_message_with_location(err);
    dump_trace(0);
  } else if (mret->type == T_STRING) {
    debug_message("%s", mret->u.string);
  }
}

namespace {
void _error_handler(char *err) {
  const char *object_name;

  debug_message_with_location(err + 1);
  object_name = dump_trace(CONFIG_INT(__RC_TRACE_CODE__));
  if (object_name) {
    object_t *ob;

    ob = find_object2(object_name);
    if (!ob) {
      if (command_giver) {
        add_vmessage(command_giver, "error when executing program in destroyed object /%s\n",
                     object_name);
      }
      debug_message("error when executing program in destroyed object /%s\n", object_name);
    }
  }
  auto default_error_message = CONFIG_STR(__DEFAULT_ERROR_MESSAGE__);
  if (command_giver && command_giver->interactive) {
#ifndef NO_WIZARDS
    if ((command_giver->flags & O_IS_WIZARD) || !strlen(default_error_message)) {
#endif
      add_message_with_location(err + 1);
#ifndef NO_WIZARDS
    } else {
      add_vmessage(command_giver, "%s\n", default_error_message);
    }
#endif
  }
  if (g_current_heartbeat_obj) {
    static char hb_message[] = "FluffOS driver tells you: You have no heart beat!\n";
    set_heart_beat(g_current_heartbeat_obj, 0);
    debug_message("Heart beat in /%s turned off.\n", g_current_heartbeat_obj->obname);
    if (g_current_heartbeat_obj->interactive) {
      add_message(g_current_heartbeat_obj, hb_message, sizeof(hb_message) - 1);
    }

    g_current_heartbeat_obj = nullptr;
  }
}

}  // namespace

[[noreturn]] void error_handler(char *err) {
/* in case we're going to jump out of load_object */
#ifndef NO_ENVIRONMENT
  restrict_destruct = nullptr;
#endif
  num_objects_this_thread = 0; /* reset the count */

  if (((current_error_context->save_csp + 1)->framekind & FRAME_MASK) == FRAME_CATCH) {
    /* user catches this error */
    /* This is added so that catches generate messages in the log file. */
    if (!CONFIG_INT(__RC_MUDLIB_ERROR_HANDLER__)) {
      debug_message_with_location(err);
      (void)dump_trace(0);
    } else {
      if (num_mudlib_error) {
        debug_message("Error in error handler: ");
        num_error++;
        debug_message_with_location(err);
        (void)dump_trace(0);
        num_error--;
        num_mudlib_error = 0;
      } else {
        if (max_eval_error) {
          set_eval(INT64_MAX);
          outoftime = 0;
        }

        if (!too_deep_error) {
          num_mudlib_error++;
          mudlib_error_handler(err, 1);
          num_mudlib_error--;
          set_eval(max_eval_cost);
        }
      }
    }

    if (max_eval_error) {
      outoftime = 1;
    }
    free_svalue(&catch_value, "caught error");
    catch_value.type = T_STRING;
    catch_value.subtype = STRING_MALLOC;
    catch_value.u.string = string_copy(err, "caught error");

    throw("error handler");
  }

  if (num_error > 0) {
    /* This can happen via errors in the object_name() apply. */
    debug_message("Error '%s' while trying to print error trace -- trace suppressed.\n", err);
    too_deep_error = max_eval_error = 0;
    if (current_error_context) {
      throw("error handler error");
    }
    fatal("Driver BUG: no error context.");
  }

  num_error++;
#ifdef PACKAGE_MUDLIB_STATS
  if (current_object) {
    add_errors(&current_object->stats, 1);
  }
#endif
  // No mudlib error handling.
  if (!CONFIG_INT(__RC_MUDLIB_ERROR_HANDLER__)) {
    _error_handler(err);
    goto exit;
  }

  // For 'too deep recurision' we can't call mudlib error handlers, so we must handle this in
  // driver.
  if (too_deep_error) {
    _error_handler(err);
    goto exit;
  }

  // Error occured while running mudlib error handler.
  if (num_mudlib_error) {
    debug_message("Error in mudlib error handler: ");
    debug_message_with_location(err);
    (void)dump_trace(0);
    num_mudlib_error = 0;
  } else {
    num_mudlib_error++;
    num_error--;
    outoftime = 0;
    mudlib_error_handler(err, 0);
    if (max_eval_error) {
      outoftime = 1;
    }
    num_mudlib_error--;
    num_error++;
  }
exit:
  num_error--;
  too_deep_error = max_eval_error = 0;
  if (current_error_context) {
    throw("error handler error2");
  }
  fatal("Driver BUG: no error context.");
}

[[noreturn]] void error_needs_free(char *s) {
  char err_buf[2048];
  strncpy(err_buf + 1, s, 2047);
  err_buf[0] = '*'; /* all system errors get a * at the start */
  err_buf[2047] = '\0';
  FREE_MSTR(s);

  error_handler(err_buf);
}

[[noreturn]] void error(const char *const fmt, ...) {
  char err_buf[2048];
  va_list args;

  va_start(args, fmt);
  vsnprintf(err_buf + 1, 2046, fmt, args);
  va_end(args);
  err_buf[0] = '*'; /* all system errors get a * at the start */
  error_handler(err_buf);
}

void do_message(svalue_t *lclass, svalue_t *msg, array_t *scope, array_t *exclude, int recurse) {
  int i, j, valid;
  object_t *ob;

  for (i = 0; i < scope->size; i++) {
    switch (scope->item[i].type) {
      case T_STRING:
        ob = find_object(scope->item[i].u.string);
        if (!ob || !object_visible(ob)) {
          continue;
        }
        break;
      case T_OBJECT:
        ob = scope->item[i].u.ob;
        break;
      default:
        continue;
    }
    if (ob->flags & O_LISTENER || ob->interactive) {
      for (valid = 1, j = 0; j < exclude->size; j++) {
        if (exclude->item[j].type != T_OBJECT) {
          continue;
        }
        if (exclude->item[j].u.ob == ob) {
          valid = 0;
          break;
        }
      }
      if (valid) {
        push_svalue(lclass);
        push_svalue(msg);
        apply(APPLY_RECEIVE_MESSAGE, ob, 2, ORIGIN_DRIVER);
      }
    }
#ifndef NO_ENVIRONMENT
    else if (recurse) {
      array_t *tmp;

      tmp = all_inventory(ob, 1);
      do_message(lclass, msg, tmp, exclude, 0);
      free_array(tmp);
    }
#endif
  }
}

void try_reset(object_t *ob) {
  if ((ob->next_reset < g_current_gametick) && !(ob->flags & O_RESET_STATE)) {
    debug(d_flag, "(lazy) RESET /%s\n", ob->obname);

    /* need to set the flag here to prevent infinite loops in apply_low */
    ob->flags |= O_RESET_STATE;
    reset_object(ob);
  }
}

#ifndef NO_ENVIRONMENT
#ifdef F_FIRST_INVENTORY
object_t *first_inventory(svalue_t *arg) {
  object_t *ob;

  if (arg->type == T_STRING) {
    ob = find_object(arg->u.string);
    if (ob && !object_visible(ob)) {
      ob = nullptr;
    }
  } else {
    ob = arg->u.ob;
  }
  if (ob == nullptr) {
    bad_argument(arg, T_STRING | T_OBJECT, 1, F_FIRST_INVENTORY);
  }
  ob = ob->contains;

#ifdef F_SET_HIDE
  while (ob && (ob->flags & O_HIDDEN) && !object_visible(ob)) {
    ob = ob->next_inv;
  }
#endif

  return ob;
}
#endif
#endif

void tell_npc(object_t *ob, const char *str) {
  copy_and_push_string(str);
  apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER);
}

/*
 * tell_object: send a message to an object.
 * If it is an interactive object, it will go to his
 * screen. Otherwise, it will go to a local function
 * catch_tell() in that object. This enables communications
 * between users and NPC's, and between other NPC's.
 * If INTERACTIVE_CATCH_TELL is defined then the message always
 * goes to catch_tell unless the target of tell_object is interactive
 * and is the current_object in which case it is written via add_message().
 */
void tell_object(object_t *ob, const char *str, int len) {
  if (!ob || (ob->flags & O_DESTRUCTED)) {
    add_message(nullptr, str, len);
    return;
  }
  if (CONFIG_INT(__RC_INTERACTIVE_CATCH_TELL__)) {
    tell_npc(ob, str);
  } else {
    /* if this is on, EVERYTHING goes through catch_tell() */
    if (ob->interactive) {
      add_message(ob, str, len);
    } else {
      tell_npc(ob, str);
    }
  }
}
