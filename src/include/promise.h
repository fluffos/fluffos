/*
 * promise.h -- the LPC-visible promise surface: promise_status() codes
 * and the rejection reasons the driver itself produces.
 *
 * Shared between driver and mudlib: the driver rejects with these, LPC
 * compares against them. Defined once here so the two can never drift.
 *
 * Every one is a constant string with a leading "*", the driver's marker for
 * a value it authored. None of them is an error(): they are delivered
 * outcomes that arrive at an await() or acatch() like any other rejection,
 * and none reaches the master's error_handler(). No trailing newline -- these
 * are values handed to a rejection handler, not messages printed by error().
 *
 * They are matched by CONTENT, which means a mudlib can forge one with
 * throw(). That is accepted: discriminating them authoritatively would need a
 * driver-reserved value kind, and promise_status() already answers the
 * question from outside.
 */

#ifndef _PROMISE_H_
#define _PROMISE_H_

/* promise_status() return codes. The driver's own state field is typed from
 * these, so the two cannot disagree. */
#define PROMISE_PENDING   0
#define PROMISE_FULFILLED 1
#define PROMISE_REJECTED  2

/* What a cancelled body's next await raises, and what its promise rejects
 * with if nothing catches it. Identical on every delivery path -- body
 * running, queued, or parked -- so one comparison catches all three. */
#define PROMISE_REASON_CANCELLED "*async function cancelled"

/* The suspended body's owner went away underneath it, so the body can never
 * continue. PROMISE_REASON_DESTRUCTED is shared by both abandon routes
 * (destruct_object()'s eager sweep and resume_coroutine()'s check) because
 * which route a frame takes is an internal scheduling detail. */
#define PROMISE_REASON_DESTRUCTED "*async function owner was destructed while suspended"
#define PROMISE_REASON_RECOMPILED "*async function owner was recompiled while suspended"
#define PROMISE_REASON_REPLACED_PROGRAM "*async function owner's program was replaced while suspended"

/* No stack left to rebuild the frame on when the body was resumed. */
#define PROMISE_REASON_STACK_OVERFLOW "*stack overflow while resuming async function"

/* A promise lost its last reference before it settled, so whoever was
 * waiting can never be told. Which one you see says who was waiting: the
 * adoption source of a resolve-with-promise, a parked body, or a combinator
 * input. */
#define PROMISE_REASON_ADOPTION_COLLECTED "*promise adoption source was collected before settling"
#define PROMISE_REASON_AWAITED_COLLECTED "*awaited promise was collected before settling"
#define PROMISE_REASON_COLLECTED "*promise was collected before settling"

/* promise_resolve(p, p) -- a promise cannot adopt itself. */
#define PROMISE_REASON_SELF_RESOLVED "*promise resolved with itself"

/* promise_any() over an empty array: nothing can ever fulfil it. */
#define PROMISE_REASON_ANY_EMPTY "*promise_any: no promises to wait for"

/* An async_yield() still queued when the driver shut down. */
#define PROMISE_REASON_YIELD_SHUTDOWN "*async_yield never ran: the driver shut down"

/* promise_reject(p) with no reason. Substituted so a bare reject is never
 * falsy -- acatch signals failure by yielding the reason, so a falsy reason
 * would read as success. */
#define PROMISE_REASON_NO_REASON "*promise rejected"

#endif /* _PROMISE_H_ */
