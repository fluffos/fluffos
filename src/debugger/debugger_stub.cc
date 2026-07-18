// WASM / no-debugger stub: satisfies the hook API (debug_hook.h) for targets
// that don't link the real debugger (the browser build has no listening
// sockets; a jsbridge-based transport is future work -- see DESIGN.md §13.6).
// g_lpc_debug_flags stays 0 forever, so the eval-loop hook never fires.

#include "base/std.h"

#include "debugger/debug_hook.h"

uint32_t g_lpc_debug_flags = 0;

void lpc_debugger_init() {}
void lpc_debugger_shutdown() {}
void lpc_debugger_instruction_hook() {}
void lpc_debugger_on_error(const char* /*err*/, int /*caught*/) {}
void lpc_debugger_on_program_loaded(program_t* /*prog*/) {}
void lpc_debugger_on_program_freed(program_t* /*prog*/) {}
int lpc_debugger_attached() { return 0; }
void lpc_debugger_break() {}
