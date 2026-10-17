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
#ifdef __linux__
  posix_eval_timer_set(etime);
#endif
  outoftime = 0;
  hooks_on_set_eval();
}

int64_t get_eval() {
#ifdef __linux__
  return posix_eval_timer_get() + hooks_eval_credit();
#else
  return max_eval_cost;
#endif
}
