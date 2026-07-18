// Public hook API for the LPC source-level debugger (see DESIGN.md).
//
// This header is the ONLY debugger header included from core driver code
// (interpret.cc, simulate.cc, program.cc, mainlib.cc) and from the
// packages/debugger efuns.  It is deliberately self-contained: no VM or
// libwebsockets headers, so it can be included anywhere.
//
// The native implementation lives in src/debugger/*.cc; the WASM target links
// src/debugger/debugger_stub.cc instead (per-target link-time selection, same
// pattern as the event loop / crash handler singletons).

#ifndef DEBUGGER_DEBUG_HOOK_H
#define DEBUGGER_DEBUG_HOOK_H

#include <cstdint>

struct program_t;

// Bitmask of pending debugger work.  0 whenever no client is attached, so the
// per-instruction cost in eval_instruction() is a single load+test+branch
// (same class as the existing DBG_LPC / TRACE_CODE checks).
extern uint32_t g_lpc_debug_flags;

enum : uint32_t {
  LPC_DEBUG_ATTACHED = 1u << 0,     // a client is attached: service transport periodically
  LPC_DEBUG_BREAKPOINTS = 1u << 1,  // resolved breakpoint addresses exist
  LPC_DEBUG_STEPPING = 1u << 2,     // a step (in/over/out) is in flight
  LPC_DEBUG_PAUSE_ARMED = 1u << 3,  // stop at the next executed instruction
};

// Boot / shutdown (no-ops unless "debugger port" is configured).
void lpc_debugger_init();
void lpc_debugger_shutdown();

// Called at the top of the eval_instruction() dispatch loop when
// g_lpc_debug_flags != 0.  pc points at the opcode about to be fetched.
void lpc_debugger_instruction_hook();

// Called from the top of error_handler() while the control stack is still
// intact.  `caught` is nonzero when a FRAME_CATCH protects this throw.
void lpc_debugger_on_error(const char* err, int caught);

// Program lifecycle notifications, for pending-breakpoint resolution and
// breakpoint-address invalidation.
void lpc_debugger_on_program_loaded(program_t* prog);
void lpc_debugger_on_program_freed(program_t* prog);

// Efun backing (packages/debugger).
int lpc_debugger_attached();
void lpc_debugger_break();

// True when the compiler should emit a local/argument name table for the
// debugger's variable inspector (DESIGN.md §9): "debugger port" is set.
// Checked once per compiled function (rule_func(),
// compiler/internal/grammar_rules.cc) -- deliberately keyed on the config
// value, not on a client currently being attached, since names are baked
// into the program at compile time and a client may attach to an
// already-running mud later.
int lpc_debugger_wants_local_names();

#endif  // DEBUGGER_DEBUG_HOOK_H
