#include "base/std.h"

#include "eval_limit.h"
#include "posix_timers.h"
#include "vm/internal/hooks.h"

volatile int outoftime = 0;
uint64_t max_eval_cost;

void init_eval() {
#ifdef __linux__
  init_posix_timers();
#else
  debug_message("WARNING: Platform doesn't support eval limit!\n");
#endif
}

void set_eval(uint64_t etime) {
  // Clear BEFORE arming: the timer is one-shot, and a short one can expire
  // (its SIGVTALRM handled, on this thread or a worker) before the clear
  // would run -- erasing the only expiry and leaving evaluation unmetered.
  outoftime = 0;
#ifdef __linux__
  posix_eval_timer_set(etime);
#endif
  hooks_on_set_eval();
}

int64_t get_eval() {
#ifdef __linux__
  return posix_eval_timer_get() + hooks_eval_credit();
#else
  return max_eval_cost;
#endif
}
