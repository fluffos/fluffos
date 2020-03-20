#include "base/std.h"

#include "eval_limit.h"
#include "posix_timers.h"

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
}

int64_t get_eval() {
#ifdef __linux__
  return posix_eval_timer_get();
#else
  return max_eval_cost;
#endif
}
