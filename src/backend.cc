/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "std.h"
#include "lpc_incl.h"
#include "backend.h"
#include "comm.h"
#include "replace_program.h"
#include "reclaim.h"
#include "socket_efuns.h"
#include "call_out.h"
#include "port.h"
#include "master.h"
#include "eval.h"
#include "outbuf.h"

#include "event.h"

#ifdef PACKAGE_ASYNC
#include "packages/async.h"
#endif

#include <deque>
#include <functional>
#include <map>
#include <mutex>

error_context_t *current_error_context = 0;

/*
 * The 'current_time' is updated in the backend loop.
 */
long g_current_virtual_time;

std::mutex g_tick_queue_mutex;  // protects g_tick_queue
static std::multimap<decltype(g_current_virtual_time), tick_event *,
                     std::less<decltype(g_current_virtual_time)>> g_tick_queue;

tick_event *add_tick_event(int delay_secs, tick_event::callback_type callback) {
  std::lock_guard<std::mutex> lock(g_tick_queue_mutex);

  auto event = new tick_event(callback);
  g_tick_queue.insert(std::make_pair(g_current_virtual_time + delay_secs, event));
  return event;
}

void call_tick_events() {
  {
    std::lock_guard<std::mutex> lock(g_tick_queue_mutex);
    if (g_tick_queue.empty()) {
      return;
    }
  }

  // FIXME: push econ check into all event callback!
  error_context_t econ;
  if (!save_context(&econ)) {
    fatal("BUG: call_tick_events can not save context!");
  }

  // Loop until there are no more events to run.
  //
  // NOTE: some event, like call_out(0), will add event to tick_queue during
  // callback, We need to keep looping until there isn't any eligible events
  // left.
  while (true) {
    std::deque<tick_event *> all_events;
    {
      std::lock_guard<std::mutex> lock(g_tick_queue_mutex);

      auto iter_end = g_tick_queue.upper_bound(g_current_virtual_time);
      // No eligible events.
      if (iter_end == g_tick_queue.begin()) {
        break;
      }
      auto iter_start = g_tick_queue.begin();

      // Extract all eligible events
      all_events.clear();
      for (auto iter = iter_start; iter != iter_end; iter++) {
        all_events.push_back(iter->second);
      }
      g_tick_queue.erase(iter_start, iter_end);
    }

    // TODO: randomly shuffle the events

    for (auto event : all_events) {
      if (event->valid) {
        try {
          event->callback();
        } catch (const char *) {
          restore_context(&econ);
        }
      }
      delete event;
    }
  }
  pop_context(&econ);
}

void clear_tick_events() {
  std::lock_guard<std::mutex> lock(g_tick_queue_mutex);

  int i = 0;
  if (!g_tick_queue.empty()) {
    for (auto iter : g_tick_queue) {
      delete iter.second;
      i++;
    }
    g_tick_queue.clear();
  }
  debug_message("clear_tick_events: %d leftover events cleared.\n", i);
}

void virtual_time_tick() {
  int64_t real_time = get_current_time();

  while (g_current_virtual_time < real_time) {
    call_tick_events();
    g_current_virtual_time++;
  }
#ifdef PACKAGE_ASYNC
  // TODO: Move this into timer based.
  check_reqs();
#endif
}

static void look_for_objects_to_swap(void);

/*
 * There are global variables that must be zeroed before any execution.
 * In case of errors, there will be a LONGJMP(), and the variables will
 * have to be cleared explicitly. They are normally maintained by the
 * code that use them.
 *
 * This routine must only be called from top level, not from inside
 * stack machine execution (as stack will be cleared).
 */
void clear_state() {
  current_object = 0;
  set_command_giver(0);
  current_interactive = 0;
  previous_ob = 0;
  current_prog = 0;
  caller_type = 0;
  reset_machine(0); /* Pop down the stack. */
} /* clear_state() */

#if 0
static void report_holes()
{
  if (current_object && current_object->name) {
    debug_message("current_object is /%s\n", current_object->name);
  }
  if (command_giver && command_giver->name) {
    debug_message("command_giver is /%s\n", command_giver->name);
  }
  if (current_interactive && current_interactive->name) {
    debug_message("current_interactive is /%s\n", current_interactive->name);
  }
  if (previous_ob && previous_ob->name) {
    debug_message("previous_ob is /%s\n", previous_ob->name);
  }
  if (current_prog && current_prog->name) {
    debug_message("current_prog is /%s\n", current_prog->name);
  }
  if (caller_type) {
    debug_message("caller_type is %s\n", caller_type);
  }
}
#endif

void call_remove_destructed_objects() {
  add_tick_event(5 * 60, tick_event::callback_type(call_remove_destructed_objects));
  if (obj_list_replace || obj_list_destruct) {
    remove_destructed_objects();
  }
}
/*
 * This is the backend. We will stay here for ever (almost).
 */
void backend(struct event_base *base) {
  // FIXME: handle this in call_tick_events().
  error_context_t econ;
  save_context(&econ);

  clear_state();

  // Register various tick events
  add_tick_event(0, tick_event::callback_type(call_heart_beat));
  add_tick_event(5 * 60, tick_event::callback_type(look_for_objects_to_swap));
  add_tick_event(30 * 60, tick_event::callback_type(std::bind(reclaim_objects, true)));
#ifdef PACKAGE_MUDLIB_STATS
  add_tick_event(60 * 60, tick_event::callback_type(mudlib_stats_decay));
#endif
  add_tick_event(5 * 60, tick_event::callback_type(call_remove_destructed_objects));

  g_current_virtual_time = get_current_time();
  clear_state();

  try {
    /* Run event loop for at most 1 second, this current handles
     * listening socket events, user socket events, and lpc socket events.
     *
     * It currently also handles user command, longer term plan is to
     * merge all callbacks execution into tick event loop and move all
     * I/O to dedicated threads.
     */
    run_event_loop(base);
  } catch (...) {  // catch everything
    fatal("BUG: jumped out of event loop!");
  }
  shutdownMudOS(-1);
} /* backend() */

/*
 * Despite the name, this routine takes care of several things.
 * It will run once every 5 minutes.
 *
 * . It will loop through all objects.
 *
 *   . If an object is found in a state of not having done reset, and the
 *     delay to next reset has passed, then reset() will be done.
 *
 *   . If the object has a existed more than the time limit given for swapping,
 *     then 'clean_up' will first be called in the object
 *
 * There are some problems if the object self-destructs in clean_up, so
 * special care has to be taken of how the linked list is used.
 */
static void look_for_objects_to_swap() {
  /* Next time is in 5 minutes */
  add_tick_event(5 * 60, tick_event::callback_type(look_for_objects_to_swap));

  object_t *ob;
  volatile object_t *next_ob, *last_good_ob;
  error_context_t econ;

  /*
   * Objects object can be destructed, which means that next object to
   * investigate is saved in next_ob. If very unlucky, that object can be
   * destructed too. In that case, the loop is simply restarted.
   */
  next_ob = obj_list;
  last_good_ob = obj_list;
  save_context(&econ);
  while (1) try {
      while ((ob = (object_t *)next_ob)) {
        int ready_for_clean_up = 0;

        if (ob->flags & O_DESTRUCTED) {
          if (last_good_ob->flags & O_DESTRUCTED) {
            ob = obj_list; /* restart */
          } else {
            ob = (object_t *)last_good_ob;
          }
        }
        next_ob = ob->next_all;

        /*
         * Check reference time before reset() is called.
         */
        if (g_current_virtual_time - ob->time_of_ref >= time_to_clean_up) {
          ready_for_clean_up = 1;
        }
#if !defined(NO_RESETS) && !defined(LAZY_RESETS)
        /*
         * Should this object have reset(1) called ?
         */
        if ((ob->flags & O_WILL_RESET) && (ob->next_reset <= g_current_virtual_time) &&
            !(ob->flags & O_RESET_STATE)) {
          debug(d_flag, "RESET /%s\n", ob->obname);
          set_eval(max_cost);
          reset_object(ob);
          if (ob->flags & O_DESTRUCTED) {
            continue;
          }
        }
#endif
        if (time_to_clean_up > 0) {
          /*
           * Has enough time passed, to give the object a chance to
           * self-destruct ? Save the O_RESET_STATE, which will be cleared.
           *
           * Only call clean_up in objects that has defined such a function.
           *
           * Only if the clean_up returns a non-zero value, will it be called
           * again.
           */

          if (ready_for_clean_up && (ob->flags & O_WILL_CLEAN_UP)) {
            int save_reset_state = ob->flags & O_RESET_STATE;
            svalue_t *svp;

            debug(d_flag, "clean up /%s\n", ob->obname);

            /*
             * Supply a flag to the object that says if this program is
             * inherited by other objects. Cloned objects might as well
             * believe they are not inherited. Swapped objects will not
             * have a ref count > 1 (and will have an invalid ob->prog
             * pointer).
             *
             * Note that if it is in the apply_low cache, it will also
             * get a flag of 1, which may cause the mudlib not to clean
             * up the object.  This isn't bad because:
             * (1) one expects it is rare for objects that have untouched
             * long enough to clean_up to still be in the cache, especially
             * on busy MUDs.
             * (2) the ones that are are the more heavily used ones, so
             * keeping them around seems justified.
             */

            push_number(ob->flags & (O_CLONE) ? 0 : ob->prog->ref);
            set_eval(max_cost);
            svp = apply(APPLY_CLEAN_UP, ob, 1, ORIGIN_DRIVER);
            if (ob->flags & O_DESTRUCTED) {
              continue;
            }
            if (!svp || (svp->type == T_NUMBER && svp->u.number == 0)) {
              ob->flags &= ~O_WILL_CLEAN_UP;
            }
            ob->flags |= save_reset_state;
          }
        }
        last_good_ob = ob;
      }
      break;
    } catch (const char *) {
      restore_context(&econ);
    }
  pop_context(&econ);
} /* look_for_objects_to_swap() */

/* New version used when not in -o mode. The epilog() in master.c is
 * supposed to return an array of files (castles in 2.4.5) to load. The array
 * returned by apply() will be freed at next call of apply(), which means that
 * the ref count has to be incremented to protect against deallocation.
 *
 * The master object is asked to do the actual loading.
 */
void preload_objects(int eflag) {
  volatile array_t *prefiles;
  svalue_t *ret;
  volatile int ix;
  error_context_t econ;

  save_context(&econ);
  try {
    push_number(eflag);
    ret = apply_master_ob(APPLY_EPILOG, 1);
  } catch (const char *) {
    restore_context(&econ);
    pop_context(&econ);
    return;
  }

  pop_context(&econ);
  if ((ret == 0) || (ret == (svalue_t *)-1) || (ret->type != T_ARRAY)) {
    return;
  } else {
    prefiles = ret->u.arr;
  }
  if ((prefiles == 0) || (prefiles->size < 1)) {
    return;
  }

  debug_message("\nLoading preloaded files ...\n");
  prefiles->ref++;
  ix = 0;
  /* in case of an error, effectively do a 'continue' */
  save_context(&econ);
  while (1) try {
      for (; ix < prefiles->size; ix++) {
        if (prefiles->item[ix].type != T_STRING) {
          continue;
        }

        set_eval(max_cost);

        push_svalue(((array_t *)prefiles)->item + ix);
        (void)apply_master_ob(APPLY_PRELOAD, 1);
      }
      free_array((array_t *)prefiles);
      break;
    } catch (const char *) {
      restore_context(&econ);
      ix++;
    }
  pop_context(&econ);
} /* preload_objects() */

/* All destructed objects are moved into a sperate linked list,
 * and deallocated after program execution.  */

void remove_destructed_objects() {
  object_t *ob, *next;

  if (obj_list_replace) {
    replace_programs();
  }
  for (ob = obj_list_destruct; ob; ob = next) {
    next = ob->next_all;
    destruct2(ob);
  }
  obj_list_destruct = 0;
} /* remove_destructed_objects() */

static double load_av = 0.0;

void update_load_av() {
  static int last_time;
  int n;
  double c;
  static int acc = 0;

  acc++;
  if (g_current_virtual_time == last_time) {
    return;
  }
  n = g_current_virtual_time - last_time;
  if (n < NUM_CONSTS) {
    c = consts[n];
  } else {
    c = exp(-n / 900.0);
  }
  load_av = c * load_av + acc * (1 - c) / n;
  last_time = g_current_virtual_time;
  acc = 0;
} /* update_load_av() */

static double compile_av = 0.0;

void update_compile_av(int lines) {
  static int last_time;
  int n;
  double c;
  static int acc = 0;

  acc += lines;
  if (g_current_virtual_time == last_time) {
    return;
  }
  n = g_current_virtual_time - last_time;
  if (n < NUM_CONSTS) {
    c = consts[n];
  } else {
    c = exp(-n / 900.0);
  }
  compile_av = c * compile_av + acc * (1 - c) / n;
  last_time = g_current_virtual_time;
  acc = 0;
} /* update_compile_av() */

char *query_load_av() {
  static char buff[100];

  sprintf(buff, "%.2f cmds/s, %.2f comp lines/s", load_av, compile_av);
  return (buff);
} /* query_load_av() */
