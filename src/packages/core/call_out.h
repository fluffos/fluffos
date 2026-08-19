#ifndef CALL_OUT_H
#define CALL_OUT_H

/*
 * call_out.c
 */

#include <chrono>
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

typedef struct pending_call_s {
  uint64_t target_time;
  union string_or_func function;
  object_t* ob;
  array_t* vs;
  object_t* command_giver;
  LPC_INT handle;
  struct TickEvent* tick_event;
  bool is_walltime;
  /* The promise for this call_out's completion: settled when it fires
   * (with the callback's return value; 0 for a timer-only entry) or
   * rejected if it is removed / its object destructed. A timer-only
   * entry's reason for existing; null otherwise; ref held. */
  struct promise_t* promise;
} pending_call_t;

void call_out(pending_call_t* cop);

/* only at exit */
void clear_call_outs(void);

void reclaim_call_outs(void);
int find_call_out_by_handle(object_t*, LPC_INT);
/* Promise for a pending call_out's completion; null if no such handle.
 * The returned promise carries a reference for the caller. */
struct promise_t* promise_for_call_out(LPC_INT handle);
int remove_call_out_by_handle(object_t*, LPC_INT);
LPC_INT new_call_out(object_t*, svalue_t*, std::chrono::milliseconds delay_msec, int, svalue_t*);
int remove_call_out(object_t*, const char*);
void remove_all_call_out(object_t*);
/* destruct: eagerly reject promises of this object's pending call_outs */
void reject_call_out_promises(object_t* obj);
int find_call_out(object_t*, const char*);
array_t* get_all_call_outs(void);
int print_call_out_usage(outbuffer_t*, int);
int total_callout_size();
void mark_call_outs(void);
void reclaim_call_outs(void);

#endif
