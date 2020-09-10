#include "base/package_api.h"

#include "packages/core/call_out.h"

#include <chrono>
#include <functional>
#include <math.h>
#include <unordered_map>

#include "packages/core/sprintf.h"

#define DBG_CALLOUT(...) debug(call_out, __VA_ARGS__)

/*
 * This file implements delayed calls of functions.
 */

// main callout map, for fastest reference
typedef std::unordered_map<LPC_INT, pending_call_t *> CalloutHandleMapType;
static CalloutHandleMapType g_callout_handle_map;

// Key is the pointer to the object, this provides an fast way for
// remove_call_out() with object only, this map may contains invalidated
// references and only get pruned during reclaim_callouts();
typedef std::unordered_multimap<object_t *, LPC_INT> CalloutObjectMapType;
static CalloutObjectMapType g_callout_object_handle_map;

// TODO: It maybe possible to change to a per-object counter.

// this counter starts with 1 because common wrong usage by passing a 0
// (Uninitialized string) to remove_call_out, this allows us to quickly filter
// that wrong call.
static uint64_t unique = 1;

static void free_call(pending_call_t * /*cop*/);
static void free_called_call(pending_call_t * /*cop*/);
void remove_all_call_out(object_t * /*obj*/);

namespace {
// NOTE: For call_out(0) prevention.
// This is the last gametick when a new call_out(0) is scheduled.
int new_call_out_zero_last_gametick = 0;
// Total number of call_out(0) that was scheduled on this gametick.
int new_call_out_zero_scheduled_on_this_gametick = 0;
}  // namespace

/*
 * Free a call out structure.
 */
static void free_called_call(pending_call_t *cop) {
  if (cop->ob) {
    free_string(cop->function.s);
    free_object(&cop->ob, "free_call");
  } else {
    free_funp(cop->function.f);
  }
  cop->ob = nullptr;
  cop->function.s = nullptr;
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
    if (cop->command_giver) {
      free_object(&cop->command_giver, "free_call");
      cop->command_giver = nullptr;
    }
  }
  if (cop->tick_event != nullptr) {
    cop->tick_event->valid = false;  // Will be freed by tick loop itself.
    cop->tick_event = nullptr;
  }
  FREE(cop);
}

static void free_call(pending_call_t *cop) {
  if (cop->vs) {
    free_array(cop->vs);
  }
  free_called_call(cop);
}

/*
 * Setup a new call out.
 */
LPC_INT new_call_out(object_t *ob, svalue_t *fun, std::chrono::milliseconds delay_msecs,
                     int num_args, svalue_t *arg, bool walltime) {
  DBG_CALLOUT("new_call_out: /%s delay msecs %" PRId64 "\n", ob->obname, delay_msecs.count());

  // call_out(0) loop prevention. This is based on the fact that new call_out(0)
  // will be executed on the same gametick, and when the total exceed the limit
  // new_call_out will error(), thus breaking the loop.
  if (delay_msecs == std::chrono::milliseconds(0)) {
    if (g_current_gametick != new_call_out_zero_last_gametick) {
      // First time call_out(0) on this tick.
      new_call_out_zero_last_gametick = g_current_gametick;
      new_call_out_zero_scheduled_on_this_gametick = 1;
    } else {
      new_call_out_zero_scheduled_on_this_gametick++;
      if (new_call_out_zero_scheduled_on_this_gametick >
          CONFIG_INT(__RC_CALL_OUT_ZERO_NEST_LEVEL__)) {
        error("Nesting call_out(0) level limit exceeded: %d. \n",
              CONFIG_INT(__RC_CALL_OUT_ZERO_NEST_LEVEL__));
      }
    }
  }

  auto *cop = reinterpret_cast<pending_call_t *>(
      DCALLOC(1, sizeof(pending_call_t), TAG_CALL_OUT, "new_call_out"));

  cop->is_walltime = walltime;

  if (cop->is_walltime) {
    cop->target_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            (std::chrono::high_resolution_clock::now() + delay_msecs).time_since_epoch())
            .count();
  } else {
    cop->target_time = g_current_gametick + time_to_gametick(delay_msecs);
  }
  DBG_CALLOUT("  is_walltime: %d\n", cop->is_walltime ? 1 : 0);
  DBG_CALLOUT("  target_time: %" PRIu64 "\n", cop->target_time);

  if (fun->type == T_STRING) {
    DBG_CALLOUT("  function: %s\n", fun->u.string);
    cop->function.s = make_shared_string(fun->u.string);
    cop->ob = ob;
    add_ref(ob, "call_out");
  } else {
    DBG_CALLOUT("  function: <function>\n");
    cop->function.f = fun->u.fp;
    fun->u.fp->hdr.ref++;
    cop->ob = nullptr;
  }

  cop->handle = g_current_gametick + (++unique);
  if (unique > 0xffffffff) {
    unique = 1;  // force wrapping around.
  }
  DBG_CALLOUT("  handle: %" LPC_INT_FMTSTR_P "\n", cop->handle);

  g_callout_handle_map.insert(std::make_pair(cop->handle, cop));
  g_callout_object_handle_map.insert(
      std::make_pair(cop->ob ? cop->ob : fun->u.fp->hdr.owner, cop->handle));

  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
    cop->command_giver = command_giver; /* save current user context */
    if (command_giver) {
      add_ref(command_giver, "new_call_out"); /* Bump its ref */
    }
  }
  if (num_args > 0) {
    cop->vs = allocate_empty_array(num_args);
    memcpy(cop->vs->item, arg, sizeof(svalue_t) * num_args);
  } else {
    cop->vs = nullptr;
  }

  auto callback = [=] { return call_out(cop); };
  if (walltime) {
    cop->tick_event = add_walltime_event(delay_msecs, tick_event::callback_type(callback));
  } else {
    cop->tick_event = add_gametick_event(delay_msecs, tick_event::callback_type(callback));
  }
  return cop->handle;
}

/*
 * See if there are any call outs to be called. Set the 'command_giver'
 * if it is a living object. Check for shadowing objects, which may also
 * be living objects.
 */
void call_out(pending_call_t *cop) {
  current_interactive = nullptr;

  object_t *ob, *new_command_giver;
  ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);

  DBG_CALLOUT("Executing callout: %s\n", ob ? ob->obname : "(null)");

  DBG_CALLOUT("  handle: %" LPC_INT_FMTSTR_P "\n", cop->handle);
  DBG_CALLOUT("  is_walltime: %i\n", cop->is_walltime ? 1 : 0);

  DBG_CALLOUT("  target_time: %" PRIu64 " vs current: %" PRIu64 "\n", cop->target_time,
              cop->is_walltime ? std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::high_resolution_clock::now().time_since_epoch())
                                     .count()
                               : g_current_gametick);

  // Remove self from callout map
  {
    int found = g_callout_handle_map.erase(cop->handle);
    DEBUG_CHECK(!found, "BUG: Rogue callout, not found in map.\n");
  }

  if (!ob || (ob->flags & O_DESTRUCTED)) {
    DBG_CALLOUT("  ob destructed, ignored.\n");
    free_call(cop);
    return;
  }

  // FIXME: Figure out why this is useful. Maybe a security thing.
  if (cop->ob && cop->function.s[0] == APPLY___INIT_SPECIAL_CHAR) {
    DBG_CALLOUT("  Trying to call illegal function, ignored.\n");
    free_call(cop);
    return;
  }

#ifndef NO_SHADOWS
  if (ob) {
    while (ob->shadowing) {
      ob = ob->shadowing;
    }
  }
#endif
  new_command_giver = nullptr;
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
    if (cop->command_giver && !(cop->command_giver->flags & O_DESTRUCTED)) {
      new_command_giver = cop->command_giver;
    } else if (ob && (ob->flags & O_LISTENER)) {
      new_command_giver = ob;
    }
    if (new_command_giver) {
      DBG_CALLOUT("  command_giver: /%s\n", new_command_giver->obname);
    }
  }
  int num_callout_args = 0;

  if (cop->vs) {
    array_t *vec = cop->vs;
    svalue_t *svp = vec->item + vec->size;
    num_callout_args = vec->size;

    while (svp-- > vec->item) {
      if (svp->type == T_OBJECT && (svp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&svp->u.ob, "call_out");
        *svp = const0u;
      }
    }
    /* cop->vs is ref one */
    transfer_push_some_svalues(cop->vs->item, vec->size);
    free_empty_array(cop->vs);
  }

  // Executing LPC callback
  set_eval(max_eval_cost);

  save_command_giver(new_command_giver);
  /* current object no longer set */
  if (cop->ob) {
    DBG_CALLOUT("  func: %s\n", cop->function.s);
    (void)safe_apply(cop->function.s, cop->ob, num_callout_args, ORIGIN_INTERNAL);
  } else {
    DBG_CALLOUT("  func: <function>\n");
    (void)safe_call_function_pointer(cop->function.f, num_callout_args);
  }
  restore_command_giver();

  free_called_call(cop);
}

static int time_left(pending_call_t *cop) {
  if (cop->is_walltime) {
    return (cop->target_time - std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::high_resolution_clock::now().time_since_epoch())
                                   .count()) /
           1000;
  } else {
    return std::chrono::duration_cast<std::chrono::seconds>(
               gametick_to_time(cop->target_time - g_current_gametick))
        .count();
  }
}

/*
 * Throw away a call out.
 * The time left until execution is returned.
 * -1 is returned if no callout with this function is pending.
 */
int remove_call_out(object_t *ob, const char *fun) {
  if (!ob) {
    return -1;
  }

  DBG_CALLOUT("remove_call_out: /%s \"%s\"\n", ob->obname, fun);

  auto range = g_callout_object_handle_map.equal_range(ob);
  auto iter = range.first;
  while (iter != range.second) {
    auto iter_handle = g_callout_handle_map.find(iter->second);
    if (iter_handle == g_callout_handle_map.end()) {
      iter = g_callout_object_handle_map.erase(iter);
      continue;
    }
    auto cop = iter_handle->second;

    if (cop->ob == ob && strcmp(cop->function.s, fun) == 0) {
      auto remaining_time = time_left(cop);
      free_call(cop);
      g_callout_handle_map.erase(iter_handle);
      g_callout_object_handle_map.erase(iter);

      DBG_CALLOUT("  found: remaining time %d.\n", remaining_time);
      return remaining_time;
    }
    iter++;
  }
  DBG_CALLOUT("  not found.\n");
  return -1;
}

int remove_call_out_by_handle(object_t *ob, LPC_INT handle) {
  if (!ob) {
    return -1;
  }

  DBG_CALLOUT("remove_call_out_by_handle: ob: %s, handle: %" LPC_INT_FMTSTR_P ".\n", ob->obname,
              handle);

  if (handle == 0 || handle < unique) {
    DBG_CALLOUT("  invalid handle, ignored.\n");
    return -1;
  }

  auto iter = g_callout_handle_map.find(handle);
  if (iter != g_callout_handle_map.end()) {
    auto cop = iter->second;
    auto remaining_time = time_left(cop);
    free_call(cop);

    g_callout_handle_map.erase(iter);

    DBG_CALLOUT("  found: remaining time %d.\n", remaining_time);
    return remaining_time;
  }
  DBG_CALLOUT("  not found.\n");
  return -1;
}

int find_call_out_by_handle(object_t *ob, LPC_INT handle) {
  DBG_CALLOUT("find_call_out_by_handle: ob: %s, handle: %" LPC_INT_FMTSTR_P "\n", ob->obname,
              handle);

  if (handle == 0 || handle < unique) {
    DBG_CALLOUT("  invalid handle, ignored.\n");
    return -1;
  }

  auto iter = g_callout_handle_map.find(handle);
  if (iter != g_callout_handle_map.end()) {
    auto cop = iter->second;
    if (cop->handle == handle && (cop->ob == ob || cop->function.f->hdr.owner == ob)) {
      auto remaining_time = time_left(cop);
      DBG_CALLOUT("  found: remaining time %d.\n", remaining_time);
      return remaining_time;
    }
  }
  DBG_CALLOUT("  not found.\n");
  return -1;
}

int find_call_out(object_t *ob, const char *fun) {
  if (!ob) {
    return -1;
  }

  DBG_CALLOUT("find_call_out: ob:%s \"%s\"\n", ob->obname, fun);

  auto range = g_callout_object_handle_map.equal_range(ob);
  auto iter = range.first;
  while (iter != range.second) {
    auto iter_handle = g_callout_handle_map.find(iter->second);
    if (iter_handle == g_callout_handle_map.end()) {
      iter = g_callout_object_handle_map.erase(iter);
      continue;
    }
    auto cop = iter_handle->second;
    if (cop->ob == ob && strcmp(cop->function.s, fun) == 0) {
      auto remaining_time = time_left(cop);
      DBG_CALLOUT("  found: remaining time %d.\n", remaining_time);
      return remaining_time;
    }
    iter++;
  }
  DBG_CALLOUT("  not found.\n");
  return -1;
}

int print_call_out_usage(outbuffer_t *ob, int verbose) {
  if (verbose == 1) {
    outbuf_add(ob, "Call out information:\n");
    outbuf_add(ob, "---------------------\n");
    outbuf_addv(ob, "Number of allocated call outs: %8" PRIu64 ", %8" PRIu64 " bytes.\n",
                g_callout_handle_map.size(), g_callout_handle_map.size() * sizeof(pending_call_t));
    outbuf_addv(ob, "Current handle map bucket: %" PRIu64 "\n",
                g_callout_handle_map.bucket_count());
    outbuf_addv(ob, "Current handle map load_factor: %f\n", g_callout_handle_map.load_factor());
    outbuf_addv(ob, "Current object map bucket: %" PRIu64 "\n",
                g_callout_object_handle_map.bucket_count());
    outbuf_addv(ob, "Current object map load_factor: %f\n",
                g_callout_object_handle_map.load_factor());
    outbuf_addv(ob, "Number of garbage entry in object map: %" PRIu64 "\n",
                g_callout_object_handle_map.size() - g_callout_handle_map.size());
  } else {
    if (verbose != -1) {
      outbuf_addv(ob, "call out:\t\t\t%8" PRIu64 " %8" PRIu64 " (load_factor %f)\n",
                  g_callout_handle_map.size(), g_callout_handle_map.size() * sizeof(pending_call_t),
                  g_callout_handle_map.load_factor());
    }
  }
  return g_callout_handle_map.size() * sizeof(pending_call_t);
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_call_outs() {
  for (auto iter = g_callout_handle_map.cbegin(); iter != g_callout_handle_map.cend(); iter++) {
    auto cop = iter->second;
    if (cop->vs) {
      cop->vs->extra_ref++;
    }
    if (cop->ob) {
      cop->ob->extra_ref++;
      EXTRA_REF(BLOCK(cop->function.s))++;
    } else {
      cop->function.f->hdr.extra_ref++;
    }
    if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
      if (cop->command_giver) {
        cop->command_giver->extra_ref++;
      }
    }
  }
}
#endif
/*
 * Construct an array of all pending call_outs. Every item in the array
 * consists of 3 items (but only if the object not is destructed):
 * 0: The object.
 * 1: The function (string).
 * 2: The delay.
 */
array_t *get_all_call_outs() {
  int i = 0;
  for (auto iter : g_callout_handle_map) {
    auto cop = iter.second;
    object_t *ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);
    if (ob && !(ob->flags & O_DESTRUCTED)) {
      i++;
    }
  }

  array_t *v = allocate_empty_array(i);

  i = 0;
  for (auto iter : g_callout_handle_map) {
    auto cop = iter.second;
    array_t *vv;
    object_t *ob;
    ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);
    if (!ob || (ob->flags & O_DESTRUCTED)) {
      continue;
    }
    vv = allocate_empty_array(3);
    if (cop->ob) {
      vv->item[0].type = T_OBJECT;
      vv->item[0].u.ob = cop->ob;
      add_ref(cop->ob, "get_all_call_outs");
      vv->item[1].type = T_STRING;
      vv->item[1].subtype = STRING_SHARED;
      vv->item[1].u.string = make_shared_string(cop->function.s);
    } else {
      outbuffer_t tmpbuf;
      svalue_t tmpval;

      tmpbuf.real_size = 0;
      tmpbuf.buffer = nullptr;

      tmpval.type = T_FUNCTION;
      tmpval.u.fp = cop->function.f;

      svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

      vv->item[0].type = T_OBJECT;
      vv->item[0].u.ob = cop->function.f->hdr.owner;
      add_ref(cop->function.f->hdr.owner, "get_all_call_outs");
      vv->item[1].type = T_STRING;
      vv->item[1].subtype = STRING_SHARED;
      vv->item[1].u.string = make_shared_string(tmpbuf.buffer);
      FREE_MSTR(tmpbuf.buffer);
    }
    vv->item[2].type = T_NUMBER;
    vv->item[2].u.number = time_left(cop);

    v->item[i].type = T_ARRAY;
    v->item[i].u.arr = vv; /* Ref count is already 1 */
    i++;
  }
  return v;
}

void remove_all_call_out(object_t *obj) {
  int i = 0;

  auto range = g_callout_object_handle_map.equal_range(obj);
  auto iter = range.first;

  while (iter != range.second) {
    auto iter_handle = g_callout_handle_map.find(iter->second);
    if (iter_handle == g_callout_handle_map.end()) {
      iter = g_callout_object_handle_map.erase(iter);
      continue;
    }
    auto cop = iter_handle->second;
    if ((cop->ob && ((cop->ob == obj) || (cop->ob->flags & O_DESTRUCTED))) ||
        (!(cop->ob) && (cop->function.f->hdr.owner == obj || !cop->function.f->hdr.owner ||
                        (cop->function.f->hdr.owner->flags & O_DESTRUCTED)))) {
      free_call(cop);
      g_callout_handle_map.erase(iter_handle);
      iter = g_callout_object_handle_map.erase(iter);
      i++;
    } else {
      iter++;
    }
  }
  DBG_CALLOUT("remove_all_call_out: removed %d callouts.\n", i);
}

void clear_call_outs() {
  int i = 0;
  auto iter = g_callout_handle_map.begin();
  while (iter != g_callout_handle_map.end()) {
    auto cop = iter->second;
    free_call(cop);
    iter = g_callout_handle_map.erase(iter);
    i++;
  }
  debug_message("clear_call_outs: %d leftover callouts cleared.\n", i);
}

void reclaim_call_outs() {
  DBG_CALLOUT("!!! reclaiming callouts.\n");

  // removes call_outs to destructed objects
  int i = 0;
  {
    auto iter = g_callout_handle_map.begin();
    while (iter != g_callout_handle_map.end()) {
      auto cop = iter->second;
      if ((cop->ob && (cop->ob->flags & O_DESTRUCTED)) ||
          (!cop->ob && (cop->function.f->hdr.owner->flags & O_DESTRUCTED))) {
        free_call(cop);
        iter = g_callout_handle_map.erase(iter);
        i++;
      } else {
        iter++;
      }
    }
  }
  DBG_CALLOUT("reclaim_call_outs: %d callouts with destructed object.\n", i);

  // GC all invalid object->handle entries
  i = 0;
  {
    auto iter = g_callout_object_handle_map.begin();
    while (iter != g_callout_object_handle_map.end()) {
      if (g_callout_handle_map.find(iter->second) == g_callout_handle_map.end()) {
        iter = g_callout_object_handle_map.erase(iter);
        i++;
      } else {
        iter++;
      }
    }
  }
  DBG_CALLOUT("reclaim_call_outs: %d garbage in object handle map.\n", i);

  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
    i = 0;
    for (auto iter : g_callout_handle_map) {
      auto cop = iter.second;
      if (cop->command_giver && (cop->command_giver->flags & O_DESTRUCTED)) {
        free_object(&cop->command_giver, "reclaim_call_outs");
        cop->command_giver = nullptr;
        i++;
      }
    }
    DBG_CALLOUT("reclaim_call_outs: %d callouts with command_giver gone.\n", i);
  }
}

namespace {
inline void int_call_out(bool walltime) {
  svalue_t *arg = sp - st_num_arg + 1;
  int num = st_num_arg - 2;
  LPC_INT ret;

  LPC_INT delay_msecs = 0;
  switch (arg[1].type) {
    case T_NUMBER:
      delay_msecs = arg[1].u.number * 1000;
      break;
    case T_REAL:
      delay_msecs = floor(arg[1].u.real * 1000.0);
      break;
  }
  if (delay_msecs < 0) {
    delay_msecs = 0;
  }

  if (!(current_object->flags & O_DESTRUCTED)) {
    ret = new_call_out(current_object, arg, std::chrono::milliseconds(delay_msecs), num, arg + 2,
                       walltime);
    /* args have been transfered; don't free them;
     also don't need to free the int */
    sp -= num + 1;
  } else {
    ret = 0;
    pop_n_elems(num);
    sp--;
  }
  /* the function */
  free_svalue(sp, "call_out");
  put_number(ret);
}
}  // namespace

#ifdef F_CALL_OUT
void f_call_out(void) { int_call_out(false); }
#endif

#ifdef F_CALL_OUT_WALLTIME
void f_call_out_walltime(void) { int_call_out(true); }
#endif

#ifdef F_CALL_OUT_INFO
void f_call_out_info(void) { push_refed_array(get_all_call_outs()); }
#endif
