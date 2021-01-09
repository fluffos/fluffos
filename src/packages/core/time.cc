#include "base/package_api.h"

#include <chrono>
#ifdef F_PERF_COUNTER_NS
void f_perf_counter_ns() {
  auto now = std::chrono::high_resolution_clock::now();
  push_number(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
}
#endif
