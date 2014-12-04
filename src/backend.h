#ifndef BACKEND_H
#define BACKEND_H

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

// API for register realtime event.
// Realtime event will be executed as close to designated walltime as possible.
struct realtime_event {
  typedef std::function<void()> callback_type;

  callback_type callback;
  realtime_event(callback_type &callback) : callback(callback) {}
};
void add_realtime_event(realtime_event::callback_type);

// API for registering game tick event.
// Game ticks provides guaranteed spacing intervals between each invocation.
struct tick_event {
  bool valid;

  typedef std::function<void()> callback_type;
  callback_type callback;

  tick_event(callback_type &callback) : valid(true), callback(callback) {}
};

// Register a event to run on game ticks. Safe to call from any thread.
tick_event *add_tick_event(int, tick_event::callback_type);

// Used in shutdownMudos()
void clear_tick_events();

void update_load_av(void);
void update_compile_av(int);
char *query_load_av(void);

#endif
