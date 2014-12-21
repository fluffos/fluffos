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
struct tick_event {
  bool valid;

  typedef std::function<void()> callback_type;
  callback_type callback;

  tick_event(callback_type &callback) : valid(true), callback(callback) {}
};

// Register a event to run on game ticks.
tick_event *add_gametick_event(std::chrono::milliseconds delay_msecs,
                               tick_event::callback_type callback);
// Realtime event will be executed as close to designated walltime as possible.
tick_event *add_walltime_event(std::chrono::milliseconds delay_msecs,
                               tick_event::callback_type callback);

// Used in shutdownMudos()
void clear_tick_events();

// Util to help translate gameticks with time.
int time_to_gametick(std::chrono::milliseconds msec);
std::chrono::milliseconds gametick_to_time(int ticks);

void update_load_av(void);
void update_compile_av(int);
char *query_load_av(void);

#endif
