#include "base/std.h"

#include <chrono>

volatile int outoftime = 0;
uint64_t max_eval_cost;

namespace {
    std::chrono::steady_clock::time_point deadline;
}

void set_eval(uint64_t etime) {
  if (etime > std::chrono::microseconds::max().count()) {
    etime = std::chrono::microseconds::max().count();
  }
  deadline = std::chrono::steady_clock::now() + std::chrono::microseconds(etime);
  outoftime = 0;
}

int64_t get_eval() {
  auto now = std::chrono::steady_clock::now();
  if (now >= deadline) {
    return 0;
  } else {
    return (deadline - now).count();
  }
}
