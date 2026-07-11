/*
 * backend_wasm.cc -- the WASM event loop.
 *
 * On WASM the page owns the event loop: there is no libevent base and no
 * blocking loop. The JS host calls the exported fluffos_tick(), which
 * lands in wasm_backend_advance(); that runs due wall-time events and as
 * many gameticks as have elapsed. The event-queue logic itself is in
 * backend.cc; the native driver compiles backend_libevent.cc instead of
 * this file.
 */

#include "base/std.h"

#include "backend.h"

#include <algorithm>
#include <chrono>
#include <map>

#include "vm/vm.h"

// backend.h exposes this for code that still reaches for the raw event
// base. Nothing in the WASM build may dereference it.
struct event_base* g_event_base = nullptr;

event_base* init_backend() {
  debug_message("Event backend in use: wasm-tick (host driven)\n");
  return nullptr;
}

namespace {
// Wall-time events, keyed by absolute host time in ms. The host clock is
// whatever the page passes to wasm_backend_advance() (performance.now()).
using WallQueue = std::multimap<double, TickEvent*, std::less<>>;
WallQueue g_wall_queue;
// Host "now" as last reported by wasm_backend_advance().
double s_now_ms = 0;
// Host time at which the last game tick was accounted for.
double s_last_gametick_ms = 0;

void call_walltime_events() {
  // Drain until quiet: a walltime callback may schedule another walltime
  // event at (or before) the current time.
  while (!g_wall_queue.empty() && g_wall_queue.begin()->first <= s_now_ms) {
    auto iter = g_wall_queue.begin();
    auto* event = iter->second;
    g_wall_queue.erase(iter);
    backend_dispose_tick_event(event);
  }
}
}  // namespace

// Schedule a wall-clock event; runs from wasm_backend_advance().
TickEvent* add_walltime_event(std::chrono::milliseconds delay_msecs,
                              TickEvent::callback_type callback) {
  auto* event = new TickEvent(callback);
  g_wall_queue.insert(WallQueue::value_type(s_now_ms + delay_msecs.count(), event));
  return event;
}

int clear_walltime_events() {
  int i = 0;
  for (auto& iter : g_wall_queue) {
    delete iter.second;
    i++;
  }
  g_wall_queue.clear();
  return i;
}

/*
 * WASM equivalent of the native blocking loop: register the recurring
 * maintenance events, then RETURN. The page's event loop is the real
 * "loop": it must call wasm_backend_advance() periodically (exported as
 * fluffos_tick, see wasm/main_wasm.cc).
 */
void backend(struct event_base* /*base*/) {
  clear_state();
  g_current_gametick = 0;
  s_last_gametick_ms = s_now_ms;

  backend_register_tick_events();
}

/*
 * Advance driver time to now_ms (host monotonic clock, milliseconds).
 *
 * Runs all due wall-time events, then advances the game tick counter by
 * however many gametick periods have elapsed, running tick events for
 * each. Catch-up is capped so a long-suspended tab doesn't replay hours
 * of ticks. Returns the suggested delay in ms until the next call.
 */
double wasm_backend_advance(double now_ms) {
  s_now_ms = now_ms;

  call_walltime_events();

  const double tick_msec = (double)CONFIG_INT(__RC_GAMETICK_MSEC__);
  int pending_ticks = (int)((now_ms - s_last_gametick_ms) / tick_msec);
  if (pending_ticks > 0) {
    const int kMaxCatchup = 100;
    if (pending_ticks > kMaxCatchup) {
      pending_ticks = kMaxCatchup;
      s_last_gametick_ms = now_ms - pending_ticks * tick_msec;
    }
    while (pending_ticks-- > 0) {
      backend_run_one_gametick();
      s_last_gametick_ms += tick_msec;
      // Run any wall-time events that became due during tick processing.
      call_walltime_events();
    }
  }

  // Suggest the next wake-up: the earlier of the next gametick boundary
  // and the next wall-time event.
  double next = s_last_gametick_ms + tick_msec - now_ms;
  if (!g_wall_queue.empty()) {
    next = std::min(next, g_wall_queue.begin()->first - now_ms);
  }
  return std::max(next, 1.0);
}
