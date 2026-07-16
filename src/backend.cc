/* 92/04/18 - cleaned up stylistically by Sulam@TMI */
#include "base/std.h"

#include "backend.h"

#include <chrono>
#include <cmath>   // for exp
#include <cstdio>  // for snprintf
#include <deque>   // for deque
#include <map>     // for multimap
#include <algorithm>

#include "vm/vm.h"

#include "packages/core/heartbeat.h"
#include "packages/core/reclaim.h"
#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

/*
 * This file is the event-loop-agnostic core of the backend: the gametick
 * event queue and the recurring maintenance work (resets, clean_up,
 * reclaims) scheduled on it. The actual loop that advances time lives in
 * the per-target implementation:
 *
 *   - backend_libevent.cc: native driver, a repeating libevent timer plus
 *     event_base_loop(); also implements wall-time events.
 *   - wasm/backend_wasm.cc: the JS host calls wasm_backend_advance() on a
 *     timer; also implements wall-time events.
 *
 * Both call backend_register_tick_events() at startup and
 * backend_run_one_gametick() per elapsed gametick.
 */

// This the current game time, which is updated on every gametick. Note we use
// a large type to avoid dealing with rollover.
uint64_t g_current_gametick;

int time_to_next_gametick(std::chrono::milliseconds msec) {
  return std::max(1, (int)(ceil(msec.count() / (double)CONFIG_INT(__RC_GAMETICK_MSEC__))));
}

std::chrono::milliseconds gametick_to_time(int ticks) {
  return std::chrono::milliseconds(CONFIG_INT(__RC_GAMETICK_MSEC__)) * ticks;
}

namespace {

// Global structure to holding all events to be executed on gameticks.
using TickQueue = std::multimap<decltype(g_current_gametick), TickEvent*, std::less<>>;
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
    std::deque<TickEvent*> all_events;
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

    for (auto* event : all_events) {
      backend_dispose_tick_event(event);
    }
  }
}

void look_for_objects_to_swap();

}  // namespace

void backend_dispose_tick_event(TickEvent* event) {
  if (event->valid) {
    event->callback();
  }
  delete event;
}

TickEvent* add_gametick_event(int delay_ticks, TickEvent::callback_type callback) {
  auto* event = new TickEvent(callback);
  g_tick_queue.insert(TickQueue::value_type(g_current_gametick + delay_ticks, event));
  return event;
}

void clear_tick_events() {
  int i = 0;
  if (!g_tick_queue.empty()) {
    for (auto& iter : g_tick_queue) {
      delete iter.second;
      i++;
    }
    g_tick_queue.clear();
  }
  i += clear_walltime_events();
  debug_message("clear_tick_events: %d leftover events cleared.\n", i);
}

// FIXME:
void call_remove_destructed_objects() {
  add_gametick_event(time_to_next_gametick(std::chrono::minutes(5)),
                     TickEvent::callback_type(call_remove_destructed_objects));
  remove_destructed_objects();
}

// Run the events of the current gametick and advance the counter: the
// per-target loop calls this once per elapsed gametick period.
void backend_run_one_gametick() {
  call_tick_events();
  g_current_gametick++;
}

// Register the driver's recurring maintenance events. Called once at
// startup by the per-target backend() implementation.
void backend_register_tick_events() {
  add_gametick_event(0, TickEvent::callback_type(call_heart_beat));
  add_gametick_event(time_to_next_gametick(std::chrono::minutes(5)),
                     TickEvent::callback_type(look_for_objects_to_swap));
  add_gametick_event(time_to_next_gametick(std::chrono::minutes(30)),
                     TickEvent::callback_type([] { return reclaim_objects(true); }));
#ifdef PACKAGE_MUDLIB_STATS
  add_gametick_event(time_to_next_gametick(std::chrono::minutes(60)),
                     TickEvent::callback_type(mudlib_stats_decay));
#endif
  add_gametick_event(time_to_next_gametick(std::chrono::minutes(5)),
                     TickEvent::callback_type(call_remove_destructed_objects));
}

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
  add_gametick_event(time_to_next_gametick(std::chrono::seconds(5 * 60)),
                     TickEvent::callback_type(look_for_objects_to_swap));

  object_t *ob, *next_ob, *last_good_ob;
  /*
   * Objects object can be destructed, which means that next object to
   * investigate is saved in next_ob. If very unlucky, that object can be
   * destructed too. In that case, the loop is simply restarted.
   */
  next_ob = obj_list;
  last_good_ob = obj_list;
  while (true) {
    while ((ob = (object_t*)next_ob)) {
      int ready_for_clean_up = 0;

      if (ob->flags & O_DESTRUCTED) {
        if (last_good_ob->flags & O_DESTRUCTED) {
          ob = obj_list; /* restart */
        } else {
          ob = (object_t*)last_good_ob;
        }
      }
      next_ob = ob->next_all;

      /*
       * Check reference time before reset() is called. An explicit
       * deadline from set_clean_up() overrides the idle-time rule.
       */
      if (ob->next_cleanup > 0) {
        if (g_current_gametick >= ob->next_cleanup) {
          ready_for_clean_up = 1;
          /* one-shot: after the deadline fires, revert to the idle rule */
          ob->next_cleanup = 0;
        }
      } else if (gametick_to_time(g_current_gametick - ob->time_of_ref) >=
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
          int const save_reset_state = ob->flags & O_RESET_STATE;

          /*
           * Set O_RESET_STATE during the apply so LAZY_RESETS' try_reset()
           * doesn't fire an overdue reset() on an object that may be about to
           * destruct; the pending reset fires on the next real touch instead.
           */
          ob->flags |= O_RESET_STATE;

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
          auto* svp = safe_apply(APPLY_CLEAN_UP, ob, 1, ORIGIN_DRIVER);
          if (!svp || (svp->type == T_NUMBER && svp->u.number == 0)) {
            ob->flags &= ~O_WILL_CLEAN_UP;
          }
          ob->flags = (ob->flags & ~O_RESET_STATE) | save_reset_state;
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
const int K_NUM_CONST = 5;
const double CONSTS[K_NUM_CONST]{
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
  if (n < K_NUM_CONST) {
    c = CONSTS[n];
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
  if (n < K_NUM_CONST) {
    c = CONSTS[n];
  } else {
    c = exp(-n / 900.0);
  }
  compile_av = c * compile_av + acc * (1 - c) / n;
  last_time = now;
  acc = 0;
} /* update_compile_av() */

char* query_load_av() {
  static char buff[100];

  snprintf(buff, sizeof(buff), "%.2f cmds/s, %.2f comp lines/s", load_av, compile_av);
  return (buff);
} /* query_load_av() */
