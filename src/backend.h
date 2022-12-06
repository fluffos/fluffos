#ifndef BACKEND_H
#define BACKEND_H

#include <chrono>
#include <functional>

/*
 * backend.c
 */

// Global event base
extern struct event_base *g_event_base;

// Initialization of main game loop.
struct event_base *init_backend();

// This is the main game loop.
void backend(struct event_base *);

// API for registering game tick event.
// Game ticks provides guaranteed spacing intervals between each invocation.
struct TickEvent {
  bool valid{true};

  using callback_type = std::function<void()>;
  callback_type callback;

  TickEvent(callback_type &callback) : callback(callback) {}
};

// Register a event to run on game ticks.
TickEvent *add_gametick_event(int delay_ticks, TickEvent::callback_type callback);
// Realtime event will be executed as close to designated walltime as possible.
TickEvent *add_walltime_event(std::chrono::milliseconds delay_msecs,
                              TickEvent::callback_type callback);

// Used in shutdownMudos()
void clear_tick_events();

// Util to help translate gameticks with time.
int time_to_next_gametick(std::chrono::milliseconds msec);
std::chrono::milliseconds gametick_to_time(int ticks);

void update_load_av();
void update_compile_av(int);
char *query_load_av();

#endif
