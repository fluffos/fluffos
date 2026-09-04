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

/* The state codes are LPC-visible (promise_status() returns them), so they
 * are defined in include/promise.h and shared with the mudlib rather than
 * duplicated here. */
#include "include/promise.h"

/* promise_all() / promise_any() / promise_race() / promise_all_settled() */
enum : uint8_t {
  PROMISE_COMB_ALL = 0,
  PROMISE_COMB_ANY = 1,
  PROMISE_COMB_RACE = 2,
  PROMISE_COMB_ALL_SETTLED = 3,
};

/* Aggregation state shared by every input reaction of ONE combinator call.
 * Ref-counted with one reference per outstanding input reaction, so it dies
 * with the last of them however they end (delivered, or dropped at shutdown).
 *
 * It is an OFF-GRAPH holder of `result` and `slots` (AGENTS.md section 3):
 * reachable only from reaction records, and reachable from SEVERAL of them at
 * once. That is why the debug ref checker marks it from its own registry
 * rather than from each reaction -- marking per reaction would bump
 * extra_ref once per input against a single real reference. */
struct promise_combinator_t {
  uint32_t ref;
  uint8_t kind;             /* PROMISE_COMB_* */
  struct promise_t* result; /* ref held; settled when the rule below is met */
  struct array_t* slots;    /* ref held; one entry per input, filled in place */
  int remaining;            /* inputs not yet settled */
};

struct promise_reaction_t {
  struct funptr_t* on_fulfilled;    /* ref held; may be null (pass-through) */
  struct funptr_t* on_rejected;     /* ref held; may be null (pass-through) */
  struct promise_t* next;           /* ref held; chained promise to settle; may be null */
  struct object_t* command_giver;   /* ref held; may be null */
  struct lpc_coroutine_t* coro;     /* owned; a parked await to resume; may be null */
  struct promise_combinator_t* comb; /* ref held; may be null */
  int comb_index;                   /* which input of `comb` this reaction is */
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
  /* promise_resolve(p, q) with q still pending leaves p PENDING while it
   * adopts q -- but its fate is committed. This flag makes a second
   * promise_resolve()/promise_reject() error deterministically instead of
   * racing the adoption's delivery (with silent last-write-wins on one
   * ordering). JS's [[AlreadyResolved]], with an error instead of a
   * silent ignore, matching the settled-guard convention. */
  bool resolving;
  /* This promise is an async function body's result channel: only the body
   * settles it, when it returns or fails. promise_resolve()/promise_reject()
   * refuse it, because "first settle wins" would otherwise DISCARD the body's
   * return value with no error -- and the body would run to completion
   * anyway, so it never was the cancellation it looked like. JS keeps this
   * state unreachable by never handing out an async function's resolver;
   * here promises are first-class, so the refusal has to be explicit. */
  bool body_owned;
  /* A cancellation has been requested on this async body and not yet
   * delivered. CONSUME-ONCE: the raise clears it, so a body that catches its
   * cancellation in acatch() can still await cleanup and even return a value
   * -- cancel is a request the body may decline, not a verdict. Lives on the
   * promise rather than on lpc_coroutine_t because the promise is the only
   * LPC-reachable handle AND the durable identity of the body across all its
   * parks: each park allocates a fresh coroutine and each resume frees it,
   * and a body running its first synchronous stretch has no coroutine at
   * all. */
  bool cancelled;
  /* This rejection exists because a cancellation propagated here -- either
   * this promise is the cancelled body, or it inherited that body's reason
   * through await / then / adoption / a combinator. dealloc_promise() skips
   * the unhandled-rejection report: cancel is a delivered outcome, not a
   * fault, and stamping handled on the target alone left every downstream
   * link to spam the driver log (PR #1353). Copied with the reason, not
   * inferred from the string, which a mudlib can forge. */
  bool from_cancel;
  /* the declared payload tag of an `async T f()`'s promise, as the runtime
   * T_* mask (0 = unannotated, e.g. promise_create()). The authoritative
   * copy lives in the svalue's subtype -- this is the record that survives
   * when a promise is handed out by a path that builds a fresh svalue
   * (async_info()'s mapping), so the same promise never renders two
   * different ways. */
  unsigned short value_type;
  svalue_t result; /* valid when state != PROMISE_PENDING */
  std::vector<promise_reaction_t>* reactions; /* pending only; else null */
  /* Where this promise was REJECTED ("/obj/thing:42"), for the unhandled-
   * rejection report. Captured at rejection, not creation: an unhandled
   * rejection is reported when the promise is finally deallocated, which can
   * be arbitrarily far from either, and of the two the site that produced the
   * failure is the one worth naming. Only rejections pay for it -- a fulfilled
   * promise never allocates this. Malloced (FREE_MSTR), null if unknown. */
  char* reject_origin;
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
size_t pending_promise_deliveries();
void promise_add_reaction(promise_t* p, funptr_t* on_fulfilled, funptr_t* on_rejected,
                          promise_t* next, object_t* giver);

void push_refed_promise(promise_t* p);

/* Build a combinator over `inputs` and return its result promise with one
 * reference for the caller. An element that is not a promise counts as
 * already fulfilled with itself (so the output of an ordinary map() works).
 * Never settles the result before the caller receives it unless every input
 * was a plain value. */
promise_t* promise_combinator_start(uint8_t kind, struct array_t* inputs);

/* Request cancellation of the async function body that owns `p`. Returns 1
 * if a cancellation was armed, 0 if there was nothing left to cancel (the
 * body already finished, or its fate is already committed to an adoption).
 * error()s if `p` is not an async function's promise. Runs no LPC. */
int promise_request_cancel(promise_t* p);

/* A promise fulfilled with 0 on the next pass of the event loop -- after the
 * driver has polled sockets, queued commands and fired due timers. This is
 * the cooperative preemption point for a long async function: `await
 * async_yield();` parks the frame at a suspension point that is safe by
 * construction and resumes it on a later drain turn.
 *
 * It is NOT the same as awaiting an already-settled promise, which parks but
 * re-queues into the SAME drain turn (deliberately: that is what lets a
 * sequential await loop run at full speed instead of paying an event-loop
 * turn per iteration). Only this reaches the poll.
 *
 * Returns a promise with one reference for the caller. */
promise_t* promise_async_yield();

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
#ifdef DEBUG
  /* The frame's recorded foreach-temporaries count (control_stack_t::
   * save_temporaries), relative to the BODY's base -- relative for exactly
   * the reason sp_offset is relative to fp: the absolute value is meaningless
   * once the frame is rebuilt on a resume, where the base is different.
   *
   * Without carrying it, resume_coroutine() re-pushes these markers AFTER it
   * has added the body's temporaries back, so each frame records the inflated
   * count instead of the one at region entry -- and an error unwinding to the
   * region then restores the counter to that, leaving it high by however many
   * loops were open inside the region. `acatch { foreach (...) { await p;
   * error(); } }` is the shape. */
  int temporaries_offset = 0;
#endif
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
  struct program_t* prog;    /* reference_prog() held; the DEFINING program
                              * (setup_new_frame() points current_prog at the
                              * base for an inherited function), which is what
                              * pc_offset / the index offsets are relative to */
  /* the OWNER's top-level program at park time, i.e. current_object->prog --
   * NOT ref-held (coro->ob holds the object, which holds its program; and if
   * replace_program() swaps it, comparing a freed pointer for INEQUALITY is
   * exactly the staleness answer we want). Distinct from `prog` above: for an
   * async function defined in an inherited file those differ, so comparing
   * ob->prog against `prog` would reject every inherited coroutine. */
  struct program_t* object_prog;
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
  /* Slots of `frame` that held a T_LVALUE pointing INTO the frame -- a
   * foreach loop variable, typically. The resume rebuilds the slice at a
   * different fp, so those are stored as offsets (and as T_NUMBER, so no
   * stale pointer exists at any point) and re-derived on arrival. */
  std::vector<int> frame_lvalues;
#ifdef DEBUG
  /* open foreach loops whose temporaries travel inside `frame`. Handed back
   * to the global counter on resume; see g_coroutine_temp_base. */
  int temporaries;
#endif
  /* True from the moment a settle hands this coroutine to the microtask queue
   * until the delivery consumes it. Ownership has moved to the queue by then,
   * so destruct-time abandonment must leave it alone -- freeing it would
   * strand the queued delivery. Kept as a flag rather than derived by
   * scanning the queue, because that scan made every destruct_object() in the
   * driver cost O(pending deliveries). */
  bool queued;
  /* Set when destruct-time abandonment found this frame already handed to the
   * microtask queue (so it could not be freed there) and the delivery will
   * abandon it on arrival. The frame is dead from that moment: it can never
   * resume, so async_info() must not list it and it must not hold one of the
   * `max suspended async functions` slots against live frames -- otherwise a
   * burst of destructs both reports phantom suspensions and can push a
   * perfectly ordinary `await` over the ceiling until the next drain.
   * Implies `queued`: a frame that is NOT queued is freed outright at
   * destruct rather than marked. */
  bool abandoned;
  /* Mirrors membership of g_live_coroutines, maintained at the same two
   * points. Lets suspended_coroutine_count() answer "is this one still
   * registered?" without a map lookup -- it runs on the await path, where
   * its own comment promised O(1) while two .count() calls made it
   * O(log n). */
  bool registered;
  std::vector<lpc_coroutine_acatch_t> markers; /* outermost first */
};

/* Run an async function body whose call frame (FRAME_ASYNC) is already set
 * up and whose args/locals are on the stack. Pushes the result promise on
 * the caller's stack before returning. */
void run_async_function(char* entry_pc, const struct function_t* funp);
/* F_AWAIT on a pending promise: park the current async frame as a reaction
 * on `awaited`. On return the caller must return out of eval_instruction().
 * error()s (nothing parked) if the current frame cannot be suspended. */
void coroutine_await_pending(promise_t* awaited);

/* Release an orphaned parked coroutine (its promise is unreachable garbage,
 * so it can never resume). Frees every ref it holds without running LPC.
 * Used by the debug orphan-cycle collector. */
void free_coroutine_orphan(lpc_coroutine_t* coro);

/* Abandon every frame parked inside `ob` (destruct_object()). Rejects each
 * frame's promise; without it a frame awaiting a promise that never settles
 * stays suspended forever. */
void abandon_coroutines_of_object(struct object_t* ob);

/* One mapping per live suspended coroutine, oldest first (async_info()). */
struct array_t* build_async_info();
/* Scheduler counters (async_info(1)): queue depth and drain deferrals. The
 * deferral count is the only portable way to observe that the drain yielded
 * -- wall-clock gaps cannot see it on a target whose host drives a virtual
 * clock, as the wasm test runner does. */
struct mapping_t* build_async_scheduler_info();


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
