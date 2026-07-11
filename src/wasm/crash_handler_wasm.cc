/*
 * crash_handler_wasm.cc -- crash handling on WASM.
 *
 * There is no native unwinder in a wasm module; uncaught traps surface in
 * the JS console with a full wasm stack already, so all of this is a
 * no-op. The native driver compiles base/internal/crash_handler.cc
 * instead of this file.
 */

#include "base/std.h"

#include "base/internal/crash_handler.h"

bool install_crash_handler() {
  // Nothing to install; the JS host reports traps.
  return true;
}

const char* crash_handler_backtrace_support() { return "JS host (wasm)"; }

void print_native_stacktrace() {
  // The JS engine prints the wasm stack for uncaught traps.
}
