/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "base/std.h"

#include "backend.h"

#include <event2/dns.h>    // for evdns_set_log_fn
#include <event2/event.h>  // for event_add, etc
#include <math.h>          // for exp
#include <stdio.h>         // for NULL, sprintf
#include <sys/time.h>      // for timeval
#include <sys/types.h>     // for int64_t
#include <deque>           // for deque
#include <functional>      // for _Bind, less, bind, function
#include <map>             // for multimap, _Rb_tree_iterator
#include <utility>         // for pair, make_pair

#include "vm/vm.h"

#ifdef PACKAGE_ASYNC
#include "packages/async/async.h"
#endif
#include "packages/core/heartbeat.h"
#include "packages/core/reclaim.h"
#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif
#ifdef PACKAGE_SOCKETS
#include "packages/sockets/socket_efuns.h"
#endif

// TODO: remove the need for this
static struct event *g_ev_tick = NULL;
static void on_virtual_time_tick(int fd, short what, void *arg);

// TODO: Figure out what to do with this.
static const int kNumConst = 5;
static const double consts[kNumConst]{
    exp(0 / 900.0), exp(-1 / 900.0), exp(-2 / 900.0), exp(-3 / 900.0), exp(-4 / 900.0),
};

/*
 * This the current game time, which is updated in the backend loop.
 */
long g_current_virtual_time;

static std::multimap<decltype(g_current_virtual_time), tick_event *,
                     std::less<decltype(g_current_virtual_time)>> g_tick_queue;

tick_event *add_tick_event(int delay_secs, tick_event::callback_type callback) {
  auto event = new tick_event(callback);
  g_tick_queue.insert(std::make_pair(g_current_virtual_time + delay_secs, event));
  return event;
}

void call_tick_events() {
  if (g_tick_queue.empty()) {
    return;
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

static void libevent_log(int severity, const char *msg) { debug(event, "%d:%s\n", severity, msg); }

static void libevent_dns_log(int severity, const char *msg) {
  debug(dns, "%d:%s\n", severity, msg);
}

// FIXME: rewrite other part so this could become static.
struct event_base *g_event_base = NULL;

// Init a new event loop.
event_base *init_backend() {
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);
#ifdef DEBUG
  event_enable_debug_mode();
#endif

  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n", event_base_get_method(g_event_base));
  return g_event_base;
}

static void on_main_loop_event(int fd, short what, void *arg) {
  auto event = (realtime_event *)arg;
  event->callback();
  delete event;
}

// Schedule a immediate event on main loop.
void add_realtime_event(realtime_event::callback_type callback) {
  auto event = new realtime_event(callback);
  event_base_once(g_event_base, -1, EV_TIMEOUT, on_main_loop_event, event, NULL);
}

static void look_for_objects_to_swap(void);

// FIXME:
void call_remove_destructed_objects() {
  add_tick_event(5 * 60, tick_event::callback_type(call_remove_destructed_objects));
  remove_destructed_objects();
}
/*
 * This is the backend. We will stay here for ever (almost).
 */
void backend(struct event_base *base) {
  clear_state();
  g_current_virtual_time = get_current_time();

  // Register various tick events

  // TODO: Have package_core register it instead.
  void call_heart_beat(void);
  add_tick_event(0, tick_event::callback_type(call_heart_beat));
  add_tick_event(5 * 60, tick_event::callback_type(look_for_objects_to_swap));
  add_tick_event(30 * 60, tick_event::callback_type(std::bind(reclaim_objects, true)));
#ifdef PACKAGE_MUDLIB_STATS
  add_tick_event(60 * 60, tick_event::callback_type(mudlib_stats_decay));
#endif
  add_tick_event(5 * 60, tick_event::callback_type(call_remove_destructed_objects));

  try {
    /* Run event loop for at most 1 second, this current handles
     * listening socket events, user socket events, and lpc socket events.
     *
     * It currently also handles user command, longer term plan is to
     * merge all callbacks execution into tick event loop and move all
     * I/O to dedicated threads.
     */
    struct timeval one_second = {1, 0};

    // Schedule a repeating tick for advancing virtual time.
    g_ev_tick = evtimer_new(base, on_virtual_time_tick, NULL);

    event_add(g_ev_tick, &one_second);
    event_base_loop(base, 0);
  } catch (...) {  // catch everything
    fatal("BUG: jumped out of event loop!");
  }
  // We've reached here meaning we are in shutdown sequence.
  shutdownMudOS(-1);
} /* backend() */

static void on_virtual_time_tick(int fd, short what, void *arg) {
  virtual_time_tick();

  struct timeval one_second = {1, 0};
  event_add(g_ev_tick, &one_second);
}
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
  auto time_to_clean_up = CONFIG_INT(__TIME_TO_CLEAN_UP__);

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
  if (n < kNumConst) {
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
  if (n < kNumConst) {
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
