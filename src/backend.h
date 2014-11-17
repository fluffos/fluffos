#ifndef BACKEND_H
#define BACKEND_H

#include <functional>

/*
 * backend.c
 */

// API for register event to be executed on each tick.
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

void backend(struct event_base *);

// Call epilog() on master ob and load all returned objects.
void preload_objects();

void clear_state(void);
int parse_command(char *, struct object_t *);
void remove_destructed_objects(void);
void update_load_av(void);
void update_compile_av(int);
char *query_load_av(void);
int query_time_used(void);
void call_heart_beat(void);

#endif
