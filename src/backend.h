#ifndef BACKEND_H
#define BACKEND_H

#include <chrono>
#include <functional>

/*
 * backend.c
 */

// Global event base
extern struct event_base* g_event_base;

// Initialization of main game loop.
struct event_base* init_backend();

// This is the main game loop. Natively (backend_libevent.cc) it blocks in
// event_base_loop(); on WASM (wasm/backend_wasm.cc) it registers the
// recurring events and returns, and the JS host advances time through
// wasm_backend_advance().
void backend(struct event_base*);

// API for registering game tick event.
// Game ticks provides guaranteed spacing intervals between each invocation.
struct TickEvent {
  bool valid{true};

  using callback_type = std::function<void()>;
  callback_type callback;

  TickEvent(callback_type& callback) : callback(callback) {}
};

// Register a event to run on game ticks.
TickEvent* add_gametick_event(int delay_ticks, TickEvent::callback_type callback);
// Realtime event will be executed as close to designated walltime as possible.
TickEvent* add_walltime_event(std::chrono::milliseconds delay_msecs,
                              TickEvent::callback_type callback);

// Used in shutdownMudos()
void clear_tick_events();

// --- shared core <-> loop implementation interface (backend.cc vs
// backend_libevent.cc / wasm/backend_wasm.cc) ---

// Register the recurring maintenance events; called from backend().
void backend_register_tick_events();
// Run the current gametick's events and advance the counter; called by the
// loop implementation once per elapsed gametick period.
void backend_run_one_gametick();
// Run a due event's callback (if still valid) and dispose of the event;
// the single place TickEvent execution semantics live.
void backend_dispose_tick_event(TickEvent*);
// Drop pending wall-time events (loop-implementation specific); returns
// how many were cleared. Called from clear_tick_events().
int clear_walltime_events();

// WASM loop implementation only: the JS host calls this (exported as
// fluffos_tick) instead of a blocking backend() loop. Runs due wall-time
// events and game ticks; returns the suggested delay in ms until the next
// call.
double wasm_backend_advance(double now_ms);

// Util to help translate gameticks with time.
int time_to_next_gametick(std::chrono::milliseconds msec);
std::chrono::milliseconds gametick_to_time(int ticks);

void update_load_av();
void update_compile_av(int);
char* query_load_av();

#endif
