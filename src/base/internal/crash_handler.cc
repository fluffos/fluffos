/*
 * crash_handler.cc -- native crash handling, over backward-cpp.
 *
 * The WASM build compiles wasm/crash_handler_wasm.cc instead of this file.
 */

#include "base/std.h"

#include "base/internal/crash_handler.h"

#include <cstdio>

#include "thirdparty/backward-cpp/backward.hpp"

bool install_crash_handler() {
  // backward-cpp doesn't yet work on win32; constructing SignalHandling is
  // still safe there, it simply reports not loaded.
  static backward::SignalHandling sh;
  return sh.loaded();
}

const char* crash_handler_backtrace_support() {
#if BACKWARD_HAS_DW == 1
  return "libdw";
#elif BACKWARD_HAS_BFD == 1
  return "libbfd";
#else
  return "none (libdw or libbfd not found, you will only get very limited crash stacktrace)";
#endif
}

void print_native_stacktrace() {
  using namespace backward;
  static StackTrace st;
  static Printer p;

  st.load_here(64);
  p.object = true;
  p.color_mode = ColorMode::automatic;
  p.address = true;
  p.print(st, stderr);
}
