#ifndef BASE_INTERNAL_CRASH_HANDLER_H
#define BASE_INTERNAL_CRASH_HANDLER_H

/*
 * Crash handling / native backtraces. Implemented over backward-cpp in
 * base/internal/crash_handler.cc for the native driver; targets without a
 * native unwinder (the WASM build -- the JS engine already prints wasm
 * stacks) compile wasm/crash_handler_wasm.cc.
 */

// Install process-wide fatal-signal handlers that print a backtrace.
// Returns false if handlers could not be installed.
bool install_crash_handler();

// One-line human-readable description of the backtrace mechanism compiled
// in (for the startup banner).
const char* crash_handler_backtrace_support();

// Best-effort: print the current native stack to stderr.
void print_native_stacktrace();

#endif /* BASE_INTERNAL_CRASH_HANDLER_H */
