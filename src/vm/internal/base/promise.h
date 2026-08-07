#ifndef LPC_PROMISE_H
#define LPC_PROMISE_H

/* It is usually better to include "lpc_incl.h" instead of including this
 directly */

#include <vector>

#include "vm/internal/base/svalue.h"

/*
 * Native LPC promises (issue #1319 phase 1).
 *
 * A promise is a first-class, ref-counted LPC value (T_PROMISE) holding the
 * eventual result of an asynchronous operation. Settlement is delivered on a
 * later gametick (the resolve()/DNS "pending table + next-tick callback"
 * idiom), never synchronously from promise_resolve()/promise_reject(), so
 * handler ordering is deterministic and re-entrancy safe.
 *
 * Ownership model:
 *  - a promise owns its pending reactions (handler funptr refs, the chained
 *    promise ref, the captured command_giver ref);
 *  - a queued (settled) reaction additionally owns a ref on its source
 *    promise until delivered;
 *  - dropping the last reference to a pending promise frees its reactions:
 *    the awaiting side simply never runs, same as an abandoned callback.
 *
 * Fulfilled results are never T_PROMISE: promise_resolve_with() adopts a
 * promise value (flattening) instead of storing it. Rejection reasons may be
 * any value.
 */

enum : uint8_t {
  PROMISE_PENDING = 0,
  PROMISE_FULFILLED = 1,
  PROMISE_REJECTED = 2,
};

struct promise_reaction_t {
  struct funptr_t* on_fulfilled;    /* ref held; may be null (pass-through) */
  struct funptr_t* on_rejected;     /* ref held; may be null (pass-through) */
  struct promise_t* next;           /* ref held; chained promise to settle; may be null */
  struct object_t* command_giver;   /* ref held; may be null */
  struct lpc_coroutine_t* coro;     /* owned; a parked await to resume; may be null */
};

struct promise_t {
  /* first element must be 'ref' (refed_t aliasing: generic svalue
   * ref-counting in assign_svalue_no_free/int_free_svalue). */
  uint32_t ref;
#ifdef DEBUGMALLOC_EXTENSIONS
  int extra_ref;
#endif
  uint8_t state; /* PROMISE_* */
  /* a rejection was (or will be) observed: a reaction was attached, or
   * promise_result() read the rejected value. Unhandled rejections are
   * reported when the promise is deallocated. */
  bool handled;
  svalue_t result; /* valid when state != PROMISE_PENDING */
  std::vector<promise_reaction_t>* reactions; /* pending only; else null */
};

promise_t* promise_alloc();
void free_promise(promise_t* p);
/* internal: ref has reached zero (called via free_compound / int_free_svalue) */
void dealloc_promise(promise_t* p);

/* First settle wins: returns 1 if this call settled the promise, 0 if it was
 * already settled (the value is not consumed in that case). */
int promise_settle(promise_t* p, svalue_t* value, int rejected);
/* Fulfill with a plain value, or adopt the eventual state of a promise value
 * (flattening). Self-resolution rejects the promise. */
void promise_resolve_with(promise_t* p, svalue_t* value);
/* Attach a reaction. All passed refs (funptrs, next, command_giver) are
 * TRANSFERRED to the promise machinery. Delivers on a later gametick if the
 * promise is already settled. */
void promise_add_reaction(promise_t* p, funptr_t* on_fulfilled, funptr_t* on_rejected,
                          promise_t* next, object_t* giver);

void push_refed_promise(promise_t* p);

/*
 * async/await coroutines (issue #1319).
 *
 * An async function body runs inside its own eval_instruction() invocation
 * (run_async_function(), entered from every call path once the call frame --
 * marked FRAME_ASYNC -- is set up). `await` on a pending promise parks the
 * body: the single frame's value-stack slice, resume pc, and any acatch()
 * region markers are copied into an lpc_coroutine_t owned by the awaited
 * promise's reaction list, and the nested eval_instruction() returns. When
 * the promise settles, the frame is rebuilt on the (empty, top-level) stacks
 * and execution continues; a rejection re-enters at the innermost acatch()
 * continuation, or rejects the coroutine's own result promise if there is
 * none.
 */

/* one saved acatch() region marker of a parked coroutine */
struct lpc_coroutine_acatch_t {
  int pc_offset; /* continuation (code after the region), relative to prog->program */
  int sp_offset; /* value-stack top at region entry, relative to fp */
  struct defer_list* defers;
};

struct lpc_coroutine_t {
  uint64_t id;               /* stable identity for async_info() */
  /* the promise being awaited: NOT ref-held (it owns this coroutine, via
   * its reaction list or the microtask queue, so it always outlives it) */
  promise_t* awaiting;
  promise_t* result_promise; /* ref held; settled when the body finishes */
  struct object_t* ob;       /* ref held */
  struct object_t* prev_ob;  /* ref held; may be null */
  struct object_t* command_giver; /* ref held; may be null */
  struct program_t* prog;    /* reference_prog() held */
  uint32_t prog_generation;  /* hot-reload guard (cf. funptr owner_gen) */
  int pc_offset;             /* resume point, relative to prog->program */
  short caller_type;
  int function_index_offset;
  int variable_index_offset;
  long table_index; /* csp->fr.table_index of the async frame (traces) */
  int num_local_variables;
  struct defer_list* defers; /* the async frame's defers */
  svalue_t* frame;           /* owned bitwise copy of [fp .. sp] */
  int frame_size;
  std::vector<lpc_coroutine_acatch_t> markers; /* outermost first */
};

/* Run an async function body whose call frame (FRAME_ASYNC) is already set
 * up and whose args/locals are on the stack. Pushes the result promise on
 * the caller's stack before returning. */
void run_async_function(char* entry_pc);
/* F_AWAIT on a pending promise: park the current async frame as a reaction
 * on `awaited`. On return the caller must return out of eval_instruction().
 * error()s (nothing parked) if the current frame cannot be suspended. */
void coroutine_await_pending(promise_t* awaited);

/* Release an orphaned parked coroutine (its promise is unreachable garbage,
 * so it can never resume). Frees every ref it holds without running LPC.
 * Used by the debug orphan-cycle collector. */
void free_coroutine_orphan(lpc_coroutine_t* coro);

/* One mapping per live suspended coroutine, oldest first (async_info()). */
struct array_t* build_async_info();


#ifdef DEBUGMALLOC_EXTENSIONS
void mark_coroutine(lpc_coroutine_t* coro);
#endif

/* error_handler()/throw_error() treat an error unwinding to this context
 * (the innermost running coroutine body boundary) like a caught one. */
extern struct error_context_t* g_coroutine_econ;

#ifdef DEBUGMALLOC_EXTENSIONS
/* mark direct children of one promise (checkmemory md-sweep TAG_PROMISE) */
void mark_promise(promise_t* p);
/* mark the microtask delivery queue's direct references (mark phase hook) */
void mark_promise_queue();
#endif

/* shutdown: drop queued deliveries (before clear_tick_events()) */
void promise_cleanup();

#endif /* LPC_PROMISE_H */
