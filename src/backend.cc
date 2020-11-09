/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "base/std.h"

#include "backend.h"

#include <chrono>
#include <event2/dns.h>     // for evdns_set_log_fn
#include <event2/event.h>   // for event_add, etc
#include <event2/thread.h>  // for thread support
#include <math.h>           // for exp
#include <stdio.h>          // for NULL, sprintf
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include <sys/types.h>  // for int64_t
#include <deque>        // for deque
#include <functional>   // for _Bind, less, bind, function
#include <map>          // for multimap, _Rb_tree_iterator
#include <utility>      // for pair, make_pair

#include "vm/vm.h"

#include "packages/core/heartbeat.h"
#include "packages/core/reclaim.h"
#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif
#ifdef PACKAGE_SOCKETS
#include "packages/sockets/socket_efuns.h"
#endif

// FIXME: rewrite other part so this could become static.
struct event_base *g_event_base = nullptr;

namespace {
void libevent_log(int severity, const char *msg) { debug(event, "events:%d:%s\n", severity, msg); }
void libevent_dns_log(int severity, const char *msg) { debug(dns, "dns:%d:%s\n", severity, msg); }
}  // namespace
// Initialize backend
event_base *init_backend() {
  event_set_log_callback(libevent_log);
  evdns_set_log_fn(libevent_dns_log);
#ifdef DEBUG
  event_enable_debug_mode();
#endif
#ifdef _WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif
  g_event_base = event_base_new();
  debug_message("Event backend in use: %s\n", event_base_get_method(g_event_base));
  return g_event_base;
}

// This the current game time, which is updated in on_virtual_time_tick. Note we use a large type
// to avoid dealing with rollover.
uint64_t g_current_gametick;

int time_to_gametick(std::chrono::milliseconds msecs) {
  return msecs.count() / CONFIG_INT(__RC_GAMETICK_MSEC__);
}

std::chrono::milliseconds gametick_to_time(int ticks) {
  return std::chrono::milliseconds(CONFIG_INT(__RC_GAMETICK_MSEC__)) * ticks;
}

namespace {
// TODO: remove the need for this
// Global variable for game ticket event handle.
struct event *g_ev_tick = nullptr;

inline struct timeval gametick_timeval() {
  static struct timeval val {
    CONFIG_INT(__RC_GAMETICK_MSEC__) / 1000,             // secs
        CONFIG_INT(__RC_GAMETICK_MSEC__) % 1000 * 1000,  // usecs
  };
  return val;
}

// Global structure to holding all events to be executed on gameticks.
typedef std::multimap<decltype(g_current_gametick), tick_event *, std::less<>> TickQueue;
TickQueue g_tick_queue;

// Call all events for current tick
inline void call_tick_events() {
  // Skip if nothing to do.
  if (g_tick_queue.empty() || g_tick_queue.begin()->first > g_current_gametick) {
    return;
  }
  // Loop until there are no more events to run.
  //
  // NOTE: some event, like call_out(0), will add event to tick_queue during
  // callback, We need to keep looping until there isn't any eligible events
  // left.
  while (true) {
    std::deque<tick_event *> all_events;
    auto iter_end = g_tick_queue.upper_bound(g_current_gametick);
    // No more eligible events.
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
        event->callback();
      }
      delete event;
    }
  }
}

void on_game_tick(evutil_socket_t fd, short what, void *arg) {
  call_tick_events();
  g_current_gametick++;

  auto ev = *(reinterpret_cast<struct event **>(arg));
  auto t = gametick_timeval();
  event_add(ev, &t);
}

}  // namespace

tick_event *add_gametick_event(std::chrono::milliseconds delay_msecs,
                               tick_event::callback_type callback) {
  auto event = new tick_event(callback);
  g_tick_queue.insert(
      TickQueue::value_type(g_current_gametick + time_to_gametick(delay_msecs), event));
  return event;
}

namespace {
void on_walltime_event(evutil_socket_t fd, short what, void *arg) {
  auto event = reinterpret_cast<tick_event *>(arg);
  if (event->valid) {
    event->callback();
  }
  delete event;
}
}  // namespace

// Schedule a immediate event on main loop.
tick_event *add_walltime_event(std::chrono::milliseconds delay_msecs,
                               tick_event::callback_type callback) {
  auto event = new tick_event(callback);
  struct timeval val {
    (int)(delay_msecs.count() / 1000), (int)(delay_msecs.count() % 1000 * 1000),
  };
  struct timeval *delay_ptr = nullptr;
  if (delay_msecs.count() != 0) {
    delay_ptr = &val;
  }
  event_base_once(g_event_base, -1, EV_TIMEOUT, on_walltime_event, event, delay_ptr);
  return event;
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

namespace {
void look_for_objects_to_swap(void);
}

// FIXME:
void call_remove_destructed_objects() {
  add_gametick_event(std::chrono::minutes(5),
                     tick_event::callback_type(call_remove_destructed_objects));
  remove_destructed_objects();
}
/*
 * This is the backend. We will stay here for ever (almost).
 */
void backend(struct event_base *base) {
  clear_state();
  g_current_gametick = 0;

  // Register various tick events
  add_gametick_event(std::chrono::seconds(0), tick_event::callback_type(call_heart_beat));
  add_gametick_event(std::chrono::minutes(5), tick_event::callback_type(look_for_objects_to_swap));
  add_gametick_event(std::chrono::minutes(30),
                     tick_event::callback_type([] { return reclaim_objects(true); }));
#ifdef PACKAGE_MUDLIB_STATS
  add_gametick_event(std::chrono::minutes(60), tick_event::callback_type(mudlib_stats_decay));
#endif
  add_gametick_event(std::chrono::minutes(5),
                     tick_event::callback_type(call_remove_destructed_objects));

  // NOTE: we don't use EV_PERSITENT here because that use fix-rate scheduling.
  //
  // Schedule a repeating tick for advancing virtual time.
  // Gametick provides a fixed-delay scheduling with a guaranteed minimum delay for
  // heartbeats, callouts, and various cleaning function.
  g_ev_tick = evtimer_new(base, on_game_tick, &g_ev_tick);

  auto t = gametick_timeval();
  event_add(g_ev_tick, &t);

  try {
    event_base_loop(base, 0);
  } catch (...) {  // catch everything
    fatal("BUG: jumped out of event loop!");
  }
  // We've reached here meaning we are in shutdown sequence.
  shutdownMudOS(-1);
} /* backend() */

namespace {
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
void look_for_objects_to_swap() {
  auto time_to_clean_up = CONFIG_INT(__TIME_TO_CLEAN_UP__);

  /* Next time is in 5 minutes */
  add_gametick_event(std::chrono::seconds(5 * 60),
                     tick_event::callback_type(look_for_objects_to_swap));

  object_t *ob, *next_ob, *last_good_ob;
  /*
   * Objects object can be destructed, which means that next object to
   * investigate is saved in next_ob. If very unlucky, that object can be
   * destructed too. In that case, the loop is simply restarted.
   */
  next_ob = obj_list;
  last_good_ob = obj_list;
  while (true) {
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
      if (gametick_to_time(g_current_gametick - ob->time_of_ref) >=
          std::chrono::seconds(time_to_clean_up)) {
        ready_for_clean_up = 1;
      }
      if (!CONFIG_INT(__RC_NO_RESETS__) && !CONFIG_INT(__RC_LAZY_RESETS__)) {
        /*
         * Should this object have reset(1) called ?
         */
        if ((ob->flags & O_WILL_RESET) && (g_current_gametick >= ob->next_reset) &&
            !(ob->flags & O_RESET_STATE)) {
          debug(d_flag, "RESET /%s\n", ob->obname);
          reset_object(ob);
          if (ob->flags & O_DESTRUCTED) {
            continue;
          }
        }
      }
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
          set_eval(max_eval_cost);
          auto svp = safe_apply(APPLY_CLEAN_UP, ob, 1, ORIGIN_DRIVER);
          if (!svp || (svp->type == T_NUMBER && svp->u.number == 0)) {
            ob->flags &= ~O_WILL_CLEAN_UP;
          }
          ob->flags |= save_reset_state;
        }
      }
      last_good_ob = ob;
    }
    break;
  }
} /* look_for_objects_to_swap() */

}  // namespace

namespace {
// TODO: Figure out what to do with this.
const int kNumConst = 5;
const double consts[kNumConst]{
    exp(0 / 900.0), exp(-1 / 900.0), exp(-2 / 900.0), exp(-3 / 900.0), exp(-4 / 900.0),
};
double load_av = 0.0;
}  // namespace

void update_load_av() {
  static long last_time;
  int n;
  double c;
  static int acc = 0;

  auto now = get_current_time();
  acc++;
  if (now == last_time) {
    return;
  }
  n = now - last_time;
  if (n < kNumConst) {
    c = consts[n];
  } else {
    c = exp(-n / 900.0);
  }
  load_av = c * load_av + acc * (1 - c) / n;
  last_time = now;
  acc = 0;
} /* update_load_av() */

static double compile_av = 0.0;

void update_compile_av(int lines) {
  static long last_time;
  int n;
  double c;
  static int acc = 0;

  auto now = get_current_time();
  acc += lines;
  if (now == last_time) {
    return;
  }
  n = now - last_time;
  if (n < kNumConst) {
    c = consts[n];
  } else {
    c = exp(-n / 900.0);
  }
  compile_av = c * compile_av + acc * (1 - c) / n;
  last_time = now;
  acc = 0;
} /* update_compile_av() */

char *query_load_av() {
  static char buff[100];

  sprintf(buff, "%.2f cmds/s, %.2f comp lines/s", load_av, compile_av);
  return (buff);
} /* query_load_av() */
