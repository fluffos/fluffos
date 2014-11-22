#include "base/std.h"

#include <cstdint>

#include "vm/internal/posix_timers.h"

// This variable is used in the signal handler, so it must be marked as volatile.
volatile int outoftime = 0;

uint64_t max_cost;

void set_eval(uint64_t etime) {
  posix_eval_timer_set(etime);
  outoftime = 0;
}

uint64_t get_eval() { return posix_eval_timer_get(); }
