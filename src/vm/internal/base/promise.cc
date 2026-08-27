#include "base/std.h"

#include "vm/internal/base/machine.h"

#include <chrono>
#include <deque>
#include <set>

#include "backend.h"
#include "thirdparty/scope_guard/scope_guard.hpp"  // DEFER
#include "vm/internal/eval_limit.h"

/*
 * Native LPC promises (issue #1319 phase 1). See promise.h for the ownership
 * model. A settle queues its reactions and arms the drain as a gametick
 * event, so delivery runs after the current execution finishes but within
 * the same tick (call_tick_events() re-loops until the tick's queue drains).
 * A drain that cannot finish in one turn continues through the EVENT LOOP
 * (schedule_drain_continue) rather than the tick queue, so network I/O and
 * command scheduling proceed between turns.
 */

/* The error context of the innermost running coroutine body boundary
 * (run_coroutine_body). error_handler()/throw_error() treat an error that
 * unwinds to this context like a caught one: the message lands in
 * catch_value and becomes a rejection (or resumes an acatch() region). */
error_context_t* g_coroutine_econ = nullptr;

#ifdef PACKAGE_DWLIB
/* dwlib's re-entrancy latch, reset by unwind_to_acatch_marker() exactly as
 * restore_context() resets it. Declared out here at file scope on purpose:
 * everything below lives in an anonymous namespace, and declaring it there
 * would name a DIFFERENT, never-defined symbol and fail to link. */
extern int _in_reference_allowed;
#endif

namespace {

/* set by coroutine_await_pending(); read by run_coroutine_body() to tell a
 * parked body from a completed one after eval_instruction() returns */
bool g_coroutine_suspended = false;
/* Live parked coroutines, keyed by park order (the id is also what
 * async_info() reports). Bounded by __RC_MAX_SUSPENDED_ASYNC__: each holds
 * a heap copy of its frame, so this is the runaway-exhaustion guard.
 * Enumeration only -- ref-marking runs through the owning promise's
 * reaction list / the microtask queue, so this registry must NOT mark
 * anything itself or extra_ref would double-count. */
std::map<uint64_t, lpc_coroutine_t*> g_live_coroutines;
/* Owner index over g_live_coroutines, so destruct_object() does not scan every
 * parked frame in the driver to discover it has none of its own. Kept in step
 * at exactly the two points that maintain the registry itself (park, and
 * free_coroutine); coro->ob is assigned once at creation and never reassigned,
 * so an entry cannot go stale while its coroutine lives. Holds no reference --
 * the coroutine already owns one on its object, and the ids here are also in
 * g_live_coroutines, which is what the ref checker walks. */
std::unordered_map<object_t*, std::vector<uint64_t>> g_coroutines_by_owner;
uint64_t g_next_coroutine_id = 0;
/* the result promise of the innermost running coroutine body */
promise_t* g_coroutine_promise = nullptr;

void free_coroutine(lpc_coroutine_t* coro, svalue_t* reject_with, bool run_lpc);
bool run_coroutine_body(char* entry_pc, promise_t* p, control_stack_t* async_frame);

/* A settled reaction awaiting delivery. Holds a ref on everything it points
 * at, including the source promise (for the result value). */
struct QueuedReaction {
  funptr_t* on_fulfilled;
  funptr_t* on_rejected;
  promise_t* next;
  object_t* command_giver;
  lpc_coroutine_t* coro;
  promise_t* source;
};

std::deque<QueuedReaction> g_promise_microtasks;
bool g_drain_scheduled = false;
/* Latched by promise_cleanup(): the queue is dead, deliveries can never run
 * again, so a late settle (something freed after it in the shutdown
 * sequence) frees its reaction on the spot instead of queueing a leak. */
bool g_promises_shut_down = false;
/* The reaction currently being delivered: popped off the deque, so its refs
 * live only in a C++ local and would be invisible to the ref checker if LPC
 * called check_memory() from a handler (AGENTS.md section 3). */
QueuedReaction* g_delivering = nullptr;
/* The coroutine currently being resumed: still registered, but no longer
 * "suspended", so async_info() must not list it. */
lpc_coroutine_t* g_resuming_coro = nullptr;
/* The coroutine being torn down while its defer() handlers run: already
 * detached from its promise's reaction list and from the microtask queue,
 * yet it still holds its frame slice, result promise, objects and program
 * until the teardown finishes -- and a defer handler is arbitrary LPC that
 * can call check_memory(). Marked like g_resuming_coro. */
lpc_coroutine_t* g_freeing_coro = nullptr;

/* How many registered coroutines are BOTH queued and abandoned -- dead frames
 * still owned by the microtask queue. Maintained as a counter, not derived by
 * scanning: suspended_coroutine_count() is on the `await` hot path and must
 * stay O(1).
 *
 * The invariant is deliberately tied to `queued`, not to registry membership:
 * suspended_coroutine_count() also discounts g_resuming_coro/g_freeing_coro,
 * and a coroutine that is abandoned AND being resumed would otherwise be
 * subtracted twice, wrapping the size_t. Note which way that fails, because
 * it is not the obvious one: LPC_INT is int64_t and size_t is 64-bit on the
 * native targets, so the wrapped count casts to a small NEGATIVE number and
 * `suspended >= limit` is simply false -- the `max suspended async
 * functions` runaway guard silently stops guarding. (It reads as a colossal
 * count only where size_t is 32 bits, i.e. wasm.) Clearing `queued` is
 * exactly the moment ownership leaves the queue for a resume or a free, i.e.
 * the moment the other discounts take over.
 *
 * One intended consequence: since an abandoned frame no longer occupies a
 * ceiling slot, a single synchronous stretch of LPC can hold more than the
 * configured maximum alive at once (park, settle, destruct the owner, repeat)
 * until the next drain frees them. They are dead frames awaiting collection,
 * which is precisely what the ceiling should not be counting; the eval-cost
 * limit bounds how many one execution can pile up. */
static size_t g_abandoned_queued = 0;

/* Ownership of `coro` is leaving the microtask queue (delivered, or dropped
 * undelivered). Keeps g_abandoned_queued in step with the flag. */
static void coroutine_clear_queued(lpc_coroutine_t* coro) {
  coro->queued = false;
  if (coro->abandoned) {
    coro->abandoned = false;
    g_abandoned_queued--;
  }
}

/* Not suspended, though still registered in g_live_coroutines: the one being
 * resumed, and the one being torn down while its defer() handlers run (both
 * can be the caller's own context). The count and the skip below MUST agree
 * exactly -- they are what keeps `i` inside the allocated array -- so they
 * share this predicate, which also covers the case where the two globals
 * point at the same coroutine. */
static bool coroutine_is_running(const lpc_coroutine_t* coro) {
  return coro == g_resuming_coro || coro == g_freeing_coro;
}

/* How many registered coroutines are actually suspended. At most two can be
 * "running", so derive this in O(1) rather than scanning the map: `await` is
 * a hot path, and a scan there made N parked coroutines cost O(N^2) overall.
 * Both globals are checked for membership because a coroutine is erased from
 * the map only at the end of free_coroutine(). */
static size_t suspended_coroutine_count() {
  size_t n = g_live_coroutines.size();
  if (g_resuming_coro != nullptr && g_resuming_coro->registered) {
    n--;
  }
  if (g_freeing_coro != nullptr && g_freeing_coro != g_resuming_coro && g_freeing_coro->registered) {
    n--;
  }
  /* Frames whose owner was destructed while their delivery was already
   * queued: still registered (the queue owns them), but dead. */
  n -= g_abandoned_queued;
  return n;
}


#ifdef DEBUGMALLOC_EXTENSIONS
/* Result promises whose only reference is a C++ local in a running
 * run_async_function() frame (one per nested synchronous first-run body).
 * Invisible to the Debug ref-checker's object-graph sweep (AGENTS.md
 * section 3), so mark_promise_queue() walks this stack -- LPC inside the
 * body can call check_memory(). The resume path needs no entry here: those
 * refs are owned by the coroutine struct, covered by marking
 * g_resuming_coro. */
std::vector<promise_t*> g_active_body_promises;
#endif

/* Deliveries per turn of the event loop when nothing is configured. At the
 * ~1-2 us a trivial delivery measures (testsuite/command/speed.lpc) this is
 * on the order of a millisecond of work before the loop gets to poll I/O
 * again -- small enough to keep the driver responsive under a large backlog,
 * large enough that the per-turn overhead (one walltime event and one loop
 * iteration) is amortised away. */
constexpr LPC_INT kDefaultDrainBudgetUs = 1000;

/* The rejection reason for a frame abandoned because its object went away.
 * Shared by BOTH routes -- destruct_object()'s eager sweep and
 * resume_coroutine()'s check when a delivery arrives for a destructed owner
 * -- because which route a given frame takes is an internal scheduling
 * detail, and a mudlib matching on the reason should not have to know it.
 * They previously differed in wording and in whether they carried a trailing
 * newline. No trailing newline: this is a value handed to a rejection
 * handler, not a message printed by error(). */
constexpr const char* kDestructedRejection = "*async function owner was destructed while suspended";

/* Consecutive slices that ended with work still queued. Routine under load;
 * a long run of them means the queue is not keeping up. Reset by any slice
 * that empties the queue. */
int g_consecutive_drain_yields = 0;
/* Every slice that ever yielded, monotonic. Reported by async_info(1): the
 * only portable way for a mudlib (or a test) to observe that the scheduler
 * yielded. Wall-clock timing cannot do it -- on a host-driven virtual clock
 * like the wasm test runner's, the resumption costs no real time at all, so
 * a yielded delivery is indistinguishable from an immediate one by elapsed
 * time alone. */
LPC_INT g_drain_yields_total = 0;
/* Settles arriving from OUTSIDE gametick dispatch that are served by an
 * event-loop arming, monotonic. That is the path a package/async I/O
 * completion takes, and the only load-independent way to observe which
 * vehicle a settle chose: wall-clock latency cannot do it, because under a
 * busy queue a correctly-armed delivery still waits behind whatever is
 * already pending. Counts the settle, not the add_walltime_event() call --
 * a settle that finds a loop-armed drain already pending needed no new
 * event and is served just as promptly. Reported by async_info(1). */
LPC_INT g_drain_arms_loop_total = 0;
/* Promises handed out by async_yield(), waiting for the next pass of the
 * event loop. The registry HOLDS A REFERENCE on each, which is what keeps a
 * yield alive when the awaiting frame is the only other holder -- and what
 * makes them an OFF-GRAPH reference the ref checker cannot see on its own,
 * so mark_promise_queue() marks them (AGENTS.md section 3).
 *
 * One event serves all of them: they all resolve at the same instant, so a
 * burst of async_yield() calls in one execution costs one TickEvent, not one
 * each. */
std::vector<promise_t*> g_pending_yields;
TickEvent* g_yield_event = nullptr;

/* True while drain_promise_microtasks() is running. A settle made BY a
 * delivery is not an external arrival, and counting it would drown the
 * signal: a drain re-posts itself as a walltime event, so from its second
 * turn on backend_in_tick_events() is false and every mid-drain settle would
 * be tallied. Ordinary promise traffic with no I/O at all moved the counter
 * ~250/second before this exclusion. */
bool g_draining = false;
/* Whether the currently pending drain event is a gametick one. A gametick
 * event armed from inside dispatch is fine -- it runs in that same pass --
 * but if it has NOT run yet and an external settle arrives afterwards, that
 * settle would piggyback on it and wait for the next gametick, which is the
 * latency this arming split exists to avoid. Tracked so such a settle can
 * upgrade the pending event to a walltime one. */
bool g_drain_event_is_tick = false;
/* Log a sustained backlog this often (in consecutive yields), so the signal
 * is bounded rather than one line per slice. */
constexpr int kDrainYieldLogInterval = 1000;

void drain_promise_microtasks();

/* Arming path for a SETTLE. The contract is the microtask one -- deliver once
 * the current execution finishes, as soon as possible -- and "as soon as
 * possible" is a different mechanism depending on where the settle happened.
 *
 * Inside gametick dispatch, a delay-0 gametick event is ideal: the
 * call_tick_events() loop re-checks its queue after every callback, so the
 * drain runs in that same pass with no trip through the event loop.
 *
 * From ANYWHERE ELSE it is the wrong answer, and badly so. call_tick_events()
 * is only ever entered from backend_run_one_gametick(), so a gametick event
 * armed from a walltime callback or a socket read is not looked at again
 * until the next gametick -- up to a full second at the default 1000ms
 * period. That is precisely the path package/async's I/O completions take:
 * async_read, async_write, async_getdir and async_db_exec all settle their
 * promise from a walltime callback, so every one of them used to pay up to a
 * gametick of latency before its handler ran. Those settles arm a walltime
 * event instead, which the loop dispatches on its next pass.
 *
 * Continuing a drain that ran out of batch is a different thing again and
 * goes through schedule_drain_continue() below. Because a mid-drain settle
 * has usually already armed an event here (setting g_drain_scheduled), that
 * continuation must CANCEL the pending one first, hence g_drain_event. */
TickEvent* g_drain_event = nullptr;

void schedule_drain() {
  if (g_drain_scheduled) {
    /* Already armed. If that arming was a gametick event and this settle is
     * an EXTERNAL arrival (an I/O completion, say), the pending event will
     * not be looked at until the next gametick -- so upgrade it to a
     * walltime event, which the loop dispatches on its next pass. Without
     * this the split below is only correct for the first settle. */
    if (!g_draining && !backend_in_tick_events()) {
      if (g_drain_event_is_tick && g_drain_event != nullptr) {
        g_drain_event->valid = false;
        g_drain_event = add_walltime_event(
            std::chrono::milliseconds(0),
            TickEvent::callback_type([] { drain_promise_microtasks(); }));
        g_drain_event_is_tick = false;
      }
      /* Counted whether this settle did the upgrade or found the pending
       * arming already loop-based: either way it is an external arrival that
       * will be served by the event loop and not by the next gametick, which
       * is what the counter reports. Counting only the upgrade made the
       * number depend on how busy the drain happened to be -- with a backlog
       * in flight a continuation is usually already armed, so a perfectly
       * prompt delivery registered as no arming at all. */
      g_drain_arms_loop_total++;
    }
    return;
  }
  g_drain_scheduled = true;
  TickEvent::callback_type cb([] { drain_promise_microtasks(); });
  /* delay 0 here is a one-shot, not a self-re-posting callback, so it cannot
   * spin the loop the way a zero-delay continuation would (see
   * schedule_drain_continue) -- it just runs on the next pass. */
  if (backend_in_tick_events()) {
    g_drain_event = add_gametick_event(0, cb);
    g_drain_event_is_tick = true;
  } else {
    g_drain_event_is_tick = false;
    /* An EXTERNAL arrival: not gametick dispatch, and not a settle made by a
     * delivery inside a running drain. This is the package/async I/O
     * completion case, which is what the counter is for. */
    if (!g_draining) {
      g_drain_arms_loop_total++;
    }
    g_drain_event = add_walltime_event(std::chrono::milliseconds(0), cb);
  }
}

/* Continue a drain that still has work, by way of the EVENT LOOP rather than
 * the gametick queue.
 *
 * This is the difference between "the game keeps running" and "the game
 * keeps ticking". A gametick event -- even at delay 0 -- is serviced from
 * inside call_tick_events(), which never returns to the loop implementation,
 * so nothing else happens while it runs: no socket reads, no command
 * scheduling. A walltime event is dispatched by the loop itself, so every
 * re-post is a point where pending network I/O is read, commands are queued,
 * and timers can fire.
 *
 * The vehicle is add_loop_yield_event(), NOT a timer, and the difference is
 * the subtle part.
 *
 * A zero-delay walltime event does not mean "run me after the next poll":
 * libevent treats a zero timeout as "activate now" and puts the callback
 * straight on the active queue ("If the event is going to become active
 * immediately, don't put it on the timeout queue", event.c
 * event_base_once). A callback that re-posts itself that way is dispatched
 * again from the same event_process_active() pass, so the loop never gets
 * back to polling sockets -- exactly the hogging this re-post exists to
 * prevent. Measured: a driver under a self-feeding promise chain never ran a
 * call_out scheduled six seconds out, in ninety seconds of wall time.
 *
 * A 1ms timer does yield, and was what this used before, but it bounds
 * delivery at one turn per millisecond -- measured at 635k deliveries in 5s.
 * add_loop_yield_event() uses libevent's active_later queue instead, which
 * is promoted to active at the TOP of the next loop iteration, before
 * dispatch(), while keeping N_ACTIVE_CALLBACKS non-zero so that iteration
 * polls without blocking. Poll, then run -- a true yield with no millisecond
 * floor, and 9.0M deliveries in the same 5s (14x) with login latency under
 * sustained load still ~1ms median.
 *
 * The wasm backend gets the same guarantee from the other side: its loop is
 * host-driven, so returning to the page IS the yield. */

void schedule_drain_continue() {
  if (!g_drain_scheduled) {
    g_drain_scheduled = true;
    /* This arming is a WALLTIME event, so record that. Leaving the flag at
     * whatever the previous arming set it to (a gametick event, if the drain
     * that just yielded had been armed from inside tick dispatch) makes
     * schedule_drain()'s upgrade branch fire against a continuation that is
     * already loop-armed: it cancels the 1ms re-post and replaces it with a
     * 0ms one. libevent activates a zero timeout immediately rather than
     * queueing it, so under sustained external settles -- exactly the
     * package/async I/O load this backpressure exists for -- every settle
     * converts the throttled continuation back into an immediate one and the
     * 1ms delay stops doing its job. */
    g_drain_event_is_tick = false;
    g_drain_event = add_loop_yield_event(TickEvent::callback_type([] {
      drain_promise_microtasks();
    }));
  }
}

/* How many deliveries one turn of the event loop may make.
 *
 * The bound is what stops the drain hogging the loop. It is deliberately a
 * COUNT and not a wall-clock deadline: a delivery is arbitrary LPC and
 * cannot be preempted part-way, so a deadline could not bound a single slow
 * handler either -- it would only add a clock read per delivery for a
 * guarantee it cannot make. What a count does buy is that the ordinary case
 * (many cheap deliveries) is chopped into loop turns of a predictable size.
 *
 * Work created DURING a turn counts toward the same batch, so a chain of
 * causally dependent deliveries (a sequential `await` loop) runs at full
 * speed rather than paying a loop turn per link. The batch is therefore the
 * only bound, which makes its size a responsiveness setting: a turn holds
 * the driver for one batch of deliveries, so a very large value lets a
 * self-feeding chain monopolise it. */
LPC_INT drain_eval_budget_us() {
  auto const configured = CONFIG_INT(__RC_ASYNC_DRAIN_EVAL_BUDGET__);
  return configured > 0 ? configured : kDefaultDrainBudgetUs;
}

void free_queued_reaction(QueuedReaction* qr) {
  if (qr->on_fulfilled) {
    free_funp(qr->on_fulfilled);
    qr->on_fulfilled = nullptr;
  }
  if (qr->on_rejected) {
    free_funp(qr->on_rejected);
    qr->on_rejected = nullptr;
  }
  if (qr->next) {
    free_promise(qr->next);
    qr->next = nullptr;
  }
  if (qr->command_giver) {
    free_object(&qr->command_giver, "free_queued_reaction");
  }
  if (qr->coro) {
    /* never delivered: free without running any LPC */
    coroutine_clear_queued(qr->coro);
    free_coroutine(qr->coro, nullptr, false);
    qr->coro = nullptr;
  }
  if (qr->source) {
    free_promise(qr->source);
    qr->source = nullptr;
  }
}

void enqueue_reaction(promise_t* source, promise_reaction_t* r) {
  source->ref++;
  QueuedReaction qr{r->on_fulfilled, r->on_rejected, r->next, r->command_giver, r->coro, source};
  if (qr.coro != nullptr) {
    qr.coro->queued = true;
  }
  if (g_promises_shut_down) {
    free_queued_reaction(&qr);
    return;
  }
  g_promise_microtasks.push_back(qr);
  schedule_drain();
}

/* Call fp with one argument, capturing an error as a rejection reason
 * instead of propagating it. The FRAME_CATCH marker below the call makes
 * error_handler() take the caught path (catch() reporting semantics + the
 * message lands in catch_value). Returns true and the handler's result in
 * *out on success; false and the error value in *out on error. *out is
 * owned by the caller either way. */
bool invoke_handler(funptr_t* fp, svalue_t* arg, svalue_t* out) {
  error_context_t econ;
  save_context(&econ);
  push_control_stack(FRAME_CATCH);
  try {
    push_svalue(arg);
    svalue_t* ret = call_function_pointer(fp, 1);
    if (ret) {
      assign_svalue_no_free(out, ret);
    } else {
      *out = const0;
    }
    pop_control_stack();
    pop_context(&econ);
    return true;
  } catch (const char*) {
    /* Latched before restore_context(), which pops control frames and so runs
     * defer() handlers -- see unwind_to_acatch_marker(). Taking ownership of
     * the caught value, like do_catch(). */
    svalue_t caught = catch_value;
    catch_value = const1;
    restore_context(&econ);
    *out = caught;
    /* max_eval_error / outoftime are left exactly as error_handler() left
     * them (it forces outoftime back on for an eval-cost error, precisely so
     * the condition cannot be swallowed by a catch). The drain loop reads
     * them to end the turn: a delivery that just consumed a whole
     * `maximum evaluation cost` is the strongest possible signal that the
     * loop wants the driver back. Re-arming a budget here would hide that,
     * and clearing the flag would too. The turn clears both on its way out. */
    too_deep_error = 0;
    pop_context(&econ);
    return false;
  }
}

/* forward decl: resumes a parked async body when its awaited promise
 * settles; frees the coroutine. */
void resume_coroutine(lpc_coroutine_t* coro, promise_t* source);

void deliver_reaction(QueuedReaction* qr) {
  promise_t* src = qr->source;

  if (qr->coro) {
    /* Every delivery gets a WHOLE eval budget of its own, so that a resumed
     * async function runs to its own natural end -- it is never cut short
     * because earlier deliveries in this turn used the loop's time. That is
     * the same deal a call_out() callback gets, and it is what makes the
     * turn budget purely a SCHEDULING bound: the turn stops handing out
     * deliveries, it never interrupts one it already started.
     *
     * Armed HERE, not inside resume_coroutine(), because its abandon paths
     * (destructed owner, recompiled/replaced program, rejection with no
     * acatch, resume stack overflow) return before reaching it while still
     * running the parked frame's defer() handlers as arbitrary LPC. Those
     * would otherwise inherit whatever the previous delivery in this drain
     * left over -- and consecutive abandons would share one cumulatively-
     * draining budget, truncating a predictable fraction of cleanups. */
    set_eval(max_eval_cost);
    /* hand the coroutine off before the call: resume_coroutine owns it from
     * here (including on its own error paths), so the drain's recovery
     * catch must not free it again */
    lpc_coroutine_t* coro = qr->coro;
    coroutine_clear_queued(coro);
    qr->coro = nullptr;
    resume_coroutine(coro, src);
    qr->source = nullptr;
    free_promise(src);
    return;
  }

  bool const rejected = (src->state == PROMISE_REJECTED);
  funptr_t* handler = rejected ? qr->on_rejected : qr->on_fulfilled;

  object_t* giver = qr->command_giver;
  if (giver && (giver->flags & O_DESTRUCTED)) {
    giver = nullptr;
  }
  save_command_giver(giver);

  if (handler) {
    /* A whole budget of its own, as above: a handler runs to completion on
     * its own terms, never truncated by what this turn already spent.
     *
     * Armed only for a delivery that runs LPC. A pass-through (no handler)
     * just propagates state to the chained promise in C++ and never enters
     * the interpreter, so arming there spent a timer_settime(2) syscall per
     * link of an adoption chain for nothing. */
    set_eval(max_eval_cost);
    svalue_t out = const0;
    if (invoke_handler(handler, &src->result, &out)) {
      if (qr->next) {
        promise_resolve_with(qr->next, &out);
      }
    } else {
      if (qr->next) {
        promise_settle(qr->next, &out, 1);
      }
    }
    free_svalue(&out, "deliver_reaction");
  } else {
    /* pass-through: propagate the source's state to the chained promise */
    if (qr->next) {
      if (rejected) {
        promise_settle(qr->next, &src->result, 1);
      } else {
        promise_resolve_with(qr->next, &src->result);
      }
    }
  }

  restore_command_giver();

  free_queued_reaction(qr);
}

void drain_promise_microtasks() {
  g_draining = true;
  DEFER { g_draining = false; };
  g_drain_scheduled = false;
  g_drain_event = nullptr; /* the event now firing is being disposed */

  /* Turn scheduling. The drain must never hold the driver: while it runs,
   * nothing else does -- no socket reads, no command scheduling, no
   * heartbeats. So a turn makes a bounded amount of progress and then
   * re-posts itself to the event loop (schedule_drain_continue), which is
   * where pending network I/O is actually serviced, picking the queue back
   * up on the next loop iteration.
   *
   * The bound governs HOW MANY deliveries a turn starts. It never truncates
   * one: each delivery is armed with a whole `maximum evaluation cost` of
   * its own (deliver_reaction), so a resumed async function or a settlement
   * handler always runs to its own natural end. Sharing one budget across a
   * turn was tried and is wrong for exactly this reason -- it made an
   * innocent handler die of "Too long evaluation" because the deliveries
   * ahead of it in the same turn had used the time. A delivery is a
   * scheduled unit of work like a call_out callback, and gets metered like
   * one; the turn budget is a SCHEDULING decision layered on top, and its
   * only effect is to stop handing out more work.
   *
   * Two independent things end a turn.
   *
   * 1. The turn has held the loop for drain_eval_budget_us(). This counts
   *    work created DURING the turn as well as work already queued, which is
   *    not an oversight but the point: `await` of an already-settled promise
   *    still parks, so an ordinary sequential loop
   *
   *        for (i = 0; i < n; i++) sum += await compute(i);
   *
   *    is a causal chain in which each delivery enqueues exactly the next
   *    one. An earlier version capped the turn at the queue length AT ENTRY
   *    so a self-feeding chain would yield on every link; that made the entry
   *    length 1 for every chain of this shape, costing a whole event-loop
   *    turn plus a re-post timer per link -- measured at ~4ms per await, i.e.
   *    two seconds for a 500-iteration loop on an idle driver, for the idiom
   *    the documentation actively recommends.
   *
   *    Measured with the monotonic clock rather than get_eval(): FluffOS
   *    meters eval cost with a POSIX CLOCK_MONOTONIC timer, so elapsed
   *    microseconds ARE the eval cost spent -- and steady_clock is a ~20ns
   *    vDSO read where timer_gettime() is a ~300ns syscall, on a path that
   *    already pays one timer_settime() per delivery.
   *
   * 2. The delivery that just ran consumed its ENTIRE budget -- `outoftime`
   *    (the volatile flag the SIGVTALRM handler sets, which
   *    eval_instruction() itself tests once per opcode) or `max_eval_error`.
   *    This test is free, and it is the one that matters under load: a
   *    handler that just burned a whole `maximum evaluation cost` has held
   *    the loop for far longer than any turn budget, and starting another
   *    full-budget delivery behind it is the last thing the driver wants.
   *    error_handler() forces `outoftime` back on for an eval-cost error
   *    precisely so it cannot be swallowed, so this single test covers both
   *    "the timer fired" and "that delivery died of the limit".
   *
   * Both are tested BEFORE a delivery, never after, so a turn always makes at
   * least one and a single expensive handler cannot starve the queue.
   * Neither is an error for the drain and neither drops anything: the tail
   * below re-arms the drain, so the remaining work is simply deferred past
   * the next I/O poll. */
  auto const budget_us = drain_eval_budget_us();
  auto const turn_started = std::chrono::steady_clock::now();
  bool first = true;
  /* The flags belong to the deliveries, not to the backend: a turn that ends
   * on an exhausted delivery must not leave `outoftime` set for whatever the
   * loop runs next (which arms its own budget, but reads the flag first). */
  DEFER {
    outoftime = 0;
    max_eval_error = 0;
  };

  while (!g_promise_microtasks.empty()) {
    if (!first) {
      if (outoftime || max_eval_error) {
        break;
      }
      if (budget_us > 0) {
        auto const spent = std::chrono::duration_cast<std::chrono::microseconds>(
                               std::chrono::steady_clock::now() - turn_started)
                               .count();
        if (spent >= budget_us) {
          break;
        }
      }
    }
    first = false;
    QueuedReaction qr = g_promise_microtasks.front();
    g_promise_microtasks.pop_front();
    /* This is a bare tick callback: an error escaping to the event loop is
     * fatal() (backend_libevent's "jumped out of event loop"). The inner
     * paths have their own handlers, but the scaffolding around them
     * (push_control_stack's "Too deep recursion", a defer running at
     * teardown) can still throw, so every delivery gets a recovery point
     * of its own -- one bad delivery must not take the driver down or
     * strand the rest of the queue. */
    error_context_t econ;
    save_context(&econ);
    g_delivering = &qr;
    try {
      deliver_reaction(&qr);
    } catch (const char*) {
      restore_context(&econ);
      free_svalue(&catch_value, "drain_promise_microtasks");
      catch_value = const0;
      too_deep_error = 0;
      /* the delivery died mid-way: release whatever it still owned
       * (deliver_reaction nulls each field as ownership is consumed, so
       * this cannot double-free what the failed path already released) */
      free_queued_reaction(&qr);
    }
    g_delivering = nullptr;
    pop_context(&econ);
  }
  if (!g_promise_microtasks.empty()) {
    g_drain_yields_total++;
    /* One yield is ordinary: the batch ended with work still queued. A long
     * RUN of them means the queue is not keeping up, which is worth saying
     * once in a while -- but never per yield, which under load would be a
     * log line per loop turn. */
    if (++g_consecutive_drain_yields % kDrainYieldLogInterval == 0) {
      debug_message(
          "promise: microtask drain has yielded %d times in a row, %d deliveries still "
          "pending.\n",
          g_consecutive_drain_yields, static_cast<int>(g_promise_microtasks.size()));
    }
    /* A mid-drain settle has usually already armed a same-tick GAMETICK
     * event (and set g_drain_scheduled). That one is serviced from inside
     * call_tick_events() without ever returning to the loop, which is
     * exactly what re-posting is trying to avoid. Cancel it and re-arm as a
     * walltime event so the loop gets to read sockets and queue commands
     * before the next batch. */
    if (g_drain_event != nullptr) {
      g_drain_event->valid = false;
      g_drain_event = nullptr;
    }
    g_drain_scheduled = false;
    schedule_drain_continue();
  } else {
    g_consecutive_drain_yields = 0;
  }
}

/* ------------------------------------------------------------------ */
/* async/await coroutine engine                                        */
/* ------------------------------------------------------------------ */

/* innermost acatch() region marker above `floor_csp`, or null */
control_stack_t* find_acatch_marker(control_stack_t* floor_csp) {
  for (control_stack_t* f = csp; f > floor_csp; f--) {
    if ((f->framekind & FRAME_MASK) == FRAME_CATCH && (f->framekind & FRAME_ASYNC)) {
      return f;
    }
  }
  return nullptr;
}

/* Unwind to an acatch() marker after a caught error and return the
 * continuation pc (the code after the region -- popping the marker restores
 * pc from it, exactly like F_END_CATCH's pop does for do_catch()).
 *
 * This must do everything restore_context() does, since no C++ handler
 * runs for an acatch: the module-global scratch reset, control stack,
 * command-giver stack, value stack, and the csp-anchored ref list. Missing
 * the command-giver unwind leaked one cgsp entry per caught error and
 * eventually overflowed that fixed array; missing the dwlib latch below
 * left reference_allowed() permanently short-circuited to "allowed" for the
 * rest of the driver's life (AGENTS.md section 4 -- module-global scratch
 * state must be reset on the error path too). When restore_context() grows
 * another such reset, it belongs here as well. */
char* unwind_to_acatch_marker(control_stack_t* marker) {
  /* Latch the caught value BEFORE anything below runs LPC. pop_control_stack()
   * invokes the frame's defer() handlers, and do_catch() assigns const1 to
   * catch_value on ENTRY -- so a handler containing any catch(...) wipes the
   * error this unwind exists to deliver, and the acatch region yields 0, which
   * reads as "no error" to `if (e = acatch {...})`. Ownership transfers here:
   * catch_value is reset now, and `caught` is handed to the stack below.
   * (AGENTS.md section 13 item 3 -- a bare global clobbered by nested LPC.) */
  svalue_t caught = catch_value;
  catch_value = const1;
#ifdef PACKAGE_DWLIB
  _in_reference_allowed = 0;
#endif
  while (csp > marker) {
    pop_control_stack();
  }
  while (cgsp != marker->save_cgsp) {
    restore_command_giver();
  }
  if (sp > marker->save_sp) {
    pop_n_elems(sp - marker->save_sp);
  } else if (sp < marker->save_sp) {
    /* restore_context()'s underflow repair, for the same reason it has one:
     * an error path with broken stack accounting can leave sp BELOW the
     * mark, and here that additionally lands the caught value at the wrong
     * depth, so the code after the region reads the wrong slots. Re-point sp
     * at the mark and neutralize the revived slots so later pops do not free
     * stale pointers. */
    debug_message("acatch unwind: value stack underflow by %" PRIdPTR " elements, repairing.\n",
                  static_cast<intptr_t>(marker->save_sp - sp));
    while (sp < marker->save_sp) {
      *++sp = const0;
    }
  }
  pop_control_stack();

  /* kill ref lvalues created at or above the frame we just left */
  ref_t* refp = global_ref_list;
  while (refp) {
    ref_t* next = refp->next;
    if (refp->csp >= csp) {
      kill_ref(refp);
    }
    refp = next;
  }

  STACK_INC;
  *sp = caught;
  return pc;
}

/* Run (or continue) an async body inside its own eval_instruction() until
 * it completes, suspends, or fails. The FRAME_ASYNC entry frame is already
 * set up and `entry_pc` points into its program. Settles `p` on completion
 * or failure. Returns true if an uncatchable eval-cost error must be
 * propagated by the caller (do_catch() parity). */
bool run_coroutine_body(char* entry_pc, promise_t* p, control_stack_t* async_frame) {
  error_context_t econ;
  save_context(&econ);
  /* The coroutine owns its whole frame (safe_apply's "callee owns the
   * args" rule, extended): if the body errors with no acatch region to
   * resume, restore_context() must reclaim the args/locals below the
   * boundary-entry sp AND pop the async control frame itself, or the
   * caller continues with a desynced stack.
   *
   * `async_frame` is passed in rather than assumed to be `csp`: on the
   * resume path acatch markers are already stacked above the async frame,
   * so `csp - 1` would leave those frames behind on every uncaught error
   * (leaking a control frame per error until "Too deep recursion", and
   * making an outer acatch stop catching). */
  econ.save_csp = async_frame - 1;
  econ.save_sp = fp - 1;
  error_context_t* prev_econ = g_coroutine_econ;
  promise_t* prev_promise = g_coroutine_promise;
  g_coroutine_econ = &econ;
  g_coroutine_promise = p;
  /* RAII, not a tail assignment: `econ` lives on THIS C++ frame, so if an
   * exception ever escapes this function the globals must not be left
   * pointing at it (error_handler() compares current_error_context against
   * g_coroutine_econ -- a dangling one is a use-after-free). */
  DEFER {
    g_coroutine_econ = prev_econ;
    g_coroutine_promise = prev_promise;
    pop_context(&econ);
  };
  bool propagate_eval_error = false;

  while (true) {
    try {
      g_coroutine_suspended = false;
      eval_instruction(entry_pc);
      if (!g_coroutine_suspended) {
        /* normal return: the body's return value is on the stack */
        promise_resolve_with(p, sp);
        free_svalue(sp--, "run_coroutine_body");
      }
      g_coroutine_suspended = false;
      break;
    } catch (const char*) {
      /* catch_value holds the error (error_handler takes the caught path
       * for g_coroutine_econ; throw_error()/await-rejection set it
       * directly). An acatch() region catches it in-place. */
      control_stack_t* marker = nullptr;
      if (!max_eval_error) {
        marker = find_acatch_marker(econ.save_csp);
      }
      if (marker) {
        /* "Too deep recursion." is catchable, exactly as in do_catch(): the
         * unwind below cuts the control stack back to the marker, so depth
         * is no longer exceeded when the region's continuation runs. Only
         * the eval-cost error stays unswallowable. */
        too_deep_error = 0;
        /* The unwind pops control frames, which runs defer() handlers --
         * arbitrary LPC that can error() again. If that happens the stacks
         * are mid-unwind, so fall through to the reject path (which cuts
         * them back to the floor) rather than letting the throw escape. */
        bool unwound = false;
        try {
          entry_pc = unwind_to_acatch_marker(marker);
          unwound = true;
        } catch (const char*) {
        }
        if (unwound) {
          continue;
        }
      }
      /* latched before restore_context() runs the frame's defers, as above */
      svalue_t err = catch_value;
      catch_value = const1;
      restore_context(&econ);
      (void)promise_settle(p, &err, 1);
      free_svalue(&err, "run_coroutine_body");
      too_deep_error = 0;
      if (max_eval_error) {
        propagate_eval_error = true;
      }
      break;
    }
  }

  return propagate_eval_error;
}

/* free defer nodes without running them */
void discard_defer_list(struct defer_list* d) {
  while (d) {
    free_svalue(&d->func, "discard_defer_list");
    free_svalue(&d->tp, "discard_defer_list");
    struct defer_list* old = d;
    d = d->next;
    FREE(old);
  }
}

/* Release a coroutine that will not (or can no longer) continue. When
 * `run_lpc` is set (safe at top level only) its pending defers run with
 * normal pop_control_stack() semantics via a scratch frame; otherwise they
 * are discarded. `reject_with`, if non-null, rejects the result promise
 * (promise_settle only queues -- no LPC runs synchronously, so this is
 * safe from deallocation paths too). */
void free_coroutine(lpc_coroutine_t* coro, svalue_t* reject_with, bool run_lpc) {
  /* Latched before free_object() below nulls coro->ob: the owner index is
   * keyed on that pointer, and losing it would strand the entry -- so the
   * object's next destruct would find no frames and skip abandoning them.
   * Only ever compared as an address, never dereferenced, so it stays valid
   * as a key even once the object is gone (ids are unique, so an unrelated
   * object later reusing the address cannot have our id in its list). */
  object_t* const owner = coro->ob;
  /* gather all pending defers, innermost region first, frame's last */
  struct defer_list* all = nullptr;
  struct defer_list** tail = &all;
  for (auto it = coro->markers.rbegin(); it != coro->markers.rend(); ++it) {
    if (it->defers) {
      *tail = it->defers;
      while (*tail) {
        tail = &(*tail)->next;
      }
      it->defers = nullptr;
    }
  }
  if (coro->defers) {
    *tail = coro->defers;
    coro->defers = nullptr;
  }
  if (all) {
    /* "Can LPC still run in this object?", not merely "did the caller ask?".
     * resume_coroutine() abandons on a destructed, recompiled or replaced
     * owner, and in every one of those cases call_function_pointer() REFUSES
     * the defer funptrs -- "Owner (...) of function pointer is destructed",
     * "Stale function pointer: owner ... was recompiled". Asking anyway
     * turns each pending defer into a driver error() through
     * mudlib_error_handler: one per defer per abandoned frame, naming a
     * cause no mudlib author can act on, and a mass destruct produces a
     * burst of them. The eager sibling route,
     * abandon_coroutines_of_object(), already discards them silently and
     * says why. Same situation, so same answer (AGENTS.md section 15).
     *
     * Decided here rather than at the four call sites so it also covers a
     * defer handler that destructs the owner partway through the list. */
    bool const can_run = run_lpc && coro->ob != nullptr &&
                         !(coro->ob->flags & O_DESTRUCTED) &&
                         coro->prog_generation == coro->ob->prog_generation &&
                         coro->ob->prog == coro->object_prog;
    if (can_run) {
      /* a scratch frame whose pop runs the defers with full semantics.
       * Those handlers are arbitrary LPC: keep this coroutine's still-held
       * refs visible to the Debug ref checker for the duration (its defer
       * lists are already detached above, so mark_coroutine() counts
       * exactly what is left). */
      lpc_coroutine_t* const prev_freeing = g_freeing_coro;
      g_freeing_coro = coro;
      /* RAII, and the list handed over before anything that can throw:
       * push_control_stack() errors on a full control stack, which would
       * otherwise leak the whole gathered list AND leave g_freeing_coro
       * dangling at a coroutine nothing frees -- biasing
       * coroutine_is_running() for the rest of the run. That is the
       * §13.13 shape: a fixed-size stack whose over-cap push throws after
       * the caller has already handed ownership over. */
      struct defer_list* pending = all;
      all = nullptr;
      DEFER {
        g_freeing_coro = prev_freeing;
        discard_defer_list(pending);
      };
      push_control_stack(FRAME_CATCH);
      csp->defers = pending;
      pending = nullptr;
      pop_control_stack();
    } else {
      discard_defer_list(all);
    }
  }
  if (coro->frame) {
    for (int i = 0; i < coro->frame_size; i++) {
      free_svalue(&coro->frame[i], "free_coroutine");
    }
    delete[] coro->frame;
    coro->frame = nullptr;
  }
  if (reject_with) {
    (void)promise_settle(coro->result_promise, reject_with, 1);
  }
  free_promise(coro->result_promise);
  free_object(&coro->ob, "free_coroutine");
  if (coro->prev_ob) {
    free_object(&coro->prev_ob, "free_coroutine");
  }
  if (coro->command_giver) {
    free_object(&coro->command_giver, "free_coroutine");
  }
  free_prog(&coro->prog);
  /* Checks `queued`, not `abandoned`: abandoned implies queued, and reaching
   * the free path still queued is the fatal case either way -- it leaves a
   * dangling QueuedReaction::coro in the deque. */
  DEBUG_CHECK(coro->queued,
              "free_coroutine: a coroutine reached the free path with its queue "
              "ownership still claimed\n");
  g_live_coroutines.erase(coro->id);
  coro->registered = false;
  {
    auto owned = g_coroutines_by_owner.find(owner);
    if (owned != g_coroutines_by_owner.end()) {
      auto& ids = owned->second;
      ids.erase(std::remove(ids.begin(), ids.end(), coro->id), ids.end());
      if (ids.empty()) {
        g_coroutines_by_owner.erase(owned);
      }
    }
  }
  delete coro;
}

void resume_coroutine(lpc_coroutine_t* coro, promise_t* source) {
  bool const rejected = (source->state == PROMISE_REJECTED);

  /* Three ways the owner can invalidate the parked frame: destruction,
   * recompile_object() (bumps prog_generation), and replace_program()
   * (swaps ob->prog to an inherited program and rewrites the variable
   * block WITHOUT bumping the generation -- the pointer comparison against
   * the OWNER's parked-time program is what catches it; note replace_program
   * can make ob->prog equal coro->prog, the defining program, which is why
   * that must not be the thing compared). Resuming past any of these would
   * run old bytecode with a stale variable_index_offset against the new
   * block: a Debug "Illegal variable access" abort, or silent wrong-global
   * reads in release. */
  if ((coro->ob->flags & O_DESTRUCTED) || coro->prog_generation != coro->ob->prog_generation ||
      coro->ob->prog != coro->object_prog) {
    svalue_t err;
    err.type = T_STRING;
    err.subtype = STRING_CONSTANT;
    if (coro->ob->flags & O_DESTRUCTED) {
      err.u.string = const_cast<char*>(kDestructedRejection);
    } else if (coro->prog_generation != coro->ob->prog_generation) {
      err.u.string = "*async function owner was recompiled while suspended";
    } else {
      err.u.string = "*async function owner's program was replaced while suspended";
    }
    free_coroutine(coro, &err, true);
    return;
  }

  if (rejected && coro->markers.empty()) {
    /* no acatch() region spans the await: the rejection propagates
     * straight to the coroutine's own promise, no need to rebuild the
     * frame at all (no catch may span an await by construction). */
    free_coroutine(coro, &source->result, true);
    return;
  }

  if (sp + coro->frame_size + 2 >= end_of_stack) {
    svalue_t err;
    err.type = T_STRING;
    err.subtype = STRING_CONSTANT;
    err.u.string = "*stack overflow while resuming async function";
    free_coroutine(coro, &err, true);
    return;
  }

  /* No set_eval() here: deliver_reaction(), the only caller, arms the budget
   * immediately before calling us precisely so that the abandon paths above
   * -- which return without reaching this point while still running the
   * parked frame's defer() handlers -- get a fresh one too. Re-arming here
   * costs a timer_settime(2) syscall and buys nothing: nothing between that
   * call and this line runs LPC. */
  /* this_player() at suspension time is restored on resume under the same
   * driver option that governs call_out() and promise_then() handlers */
  object_t* giver = nullptr;
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__)) {
    giver = (coro->command_giver && !(coro->command_giver->flags & O_DESTRUCTED))
                ? coro->command_giver
                : nullptr;
  }
  save_command_giver(giver);

  /* Rebuild the async frame on the (top-level) stacks. FRAME_EXTERNAL is
   * set here, not left to eval_instruction()'s entry: acatch markers are
   * re-pushed ABOVE this frame, so the entry would stamp the innermost
   * marker instead and the body's F_RETURN would sail past this frame
   * into stale pc. */
  push_control_stack(FRAME_FUNCTION | FRAME_ASYNC | FRAME_OB_CHANGE | FRAME_EXTERNAL);
  control_stack_t* const async_frame = csp;
  csp->fr.table_index = coro->table_index;
  csp->num_local_variables = coro->num_local_variables;
  csp->defers = coro->defers;
  coro->defers = nullptr;
  caller_type = coro->caller_type;
  previous_ob = coro->prev_ob;
  current_object = coro->ob;
  current_prog = coro->prog;
  /* (pc, current_prog) must stay a consistent pair from here on: rebuilding
   * the frame can run mudlib code before eval_instruction() sets pc itself
   * (unwind_to_acatch_marker() below pops control frames, which runs their
   * defer() handlers), and dump_trace()/find_line() would compute a bogus
   * offset from a null pc against a real program. */
  pc = coro->prog->program + coro->pc_offset;
  function_index_offset = coro->function_index_offset;
  variable_index_offset = coro->variable_index_offset;
  fp = sp + 1;
  if (coro->frame_size > 0) {
    memcpy(fp, coro->frame, coro->frame_size * sizeof(svalue_t));
  }
  sp = fp + coro->frame_size - 1;
  delete[] coro->frame;
  coro->frame = nullptr;
  coro->frame_size = 0;

  for (auto& m : coro->markers) {
    push_control_stack(FRAME_CATCH | FRAME_ASYNC);
    csp->pc = coro->prog->program + m.pc_offset;
    csp->save_sp = fp + m.sp_offset;
    csp->save_cgsp = cgsp;
    csp->num_local_variables = coro->num_local_variables;
    csp->defers = m.defers;
    m.defers = nullptr;
  }

  /* Claim the coroutine as "running" BEFORE anything below can enter LPC.
   * The rejected path's unwind_to_acatch_marker() pops control frames,
   * which runs the acatch region's defer() handlers -- arbitrary LPC that
   * can call check_memory(). By this point the coroutine is detached from
   * the microtask queue and from every reaction list, so if it is not
   * claimed here nothing marks the refs it still holds (its frame slice,
   * result promise, objects and program) and the Debug ref checker reports
   * them as bad counts. Same hole g_resuming_coro and g_freeing_coro were
   * introduced to close, in the one window neither covered. */
  lpc_coroutine_t* const prev_resuming = g_resuming_coro;
  g_resuming_coro = coro;

  /* RAII, not tail statements, for the same reason run_coroutine_body()
   * gives: run_coroutine_body() catches `const char*`, which covers every
   * error()/throw() an LPC body can raise, but NOT a C++ exception from the
   * allocations around it (std::bad_alloc out of `new svalue_t[n]`, a
   * markers/vector push_back). If one ever escaped, a tail assignment would
   * leave g_resuming_coro dangling at a coroutine nothing frees -- which also
   * biases suspended_coroutine_count() forever, since coroutine_is_running()
   * keeps matching the stale pointer -- and the command-giver stack one entry
   * deep. Unreachable outside OOM today; this just stops it being a question.
   *
   * Armed BEFORE the entry-point computation below, not after it: STACK_INC
   * expands to CHECK_STACK_OVERFLOW and can error(), and
   * unwind_to_acatch_marker() runs pop_control_stack() -- arbitrary LPC
   * defer handlers -- and ends in a STACK_INC of its own. An escape from
   * either lands in the drain's catch with qr->coro already nulled, so
   * nothing would free the coroutine: it would leak with its frame and refs,
   * stay in g_live_coroutines, and leave g_resuming_coro pointing at it. The
   * follow-on is worse than the leak -- the drain then releases qr->source,
   * and build_async_info() reads coro->awaiting (deliberately not ref-held)
   * for async_info(), i.e. a use-after-free from ordinary LPC. */
  DEFER {
    g_resuming_coro = prev_resuming;
    restore_command_giver();
  };

  char* entry;
  if (!rejected) {
    /* the await expression's value */
    STACK_INC;
    assign_svalue_no_free(sp, &source->result);
    entry = coro->prog->program + coro->pc_offset;
  } else {
    /* re-raise at the await point; the innermost acatch() catches it */
    assign_svalue(&catch_value, &source->result);
    entry = unwind_to_acatch_marker(csp);
  }
  /* The eval-cost flag is deliberately NOT consumed here, unlike the
   * synchronous entry in run_async_function(), which rethrows it into its
   * caller's evaluation. A resumption has no caller -- but the drain turn
   * that delivered it does read the flag, as a resumed frame that burned its
   * whole budget ends the turn. drain_promise_microtasks() clears it. */
  (void)run_coroutine_body(entry, coro->result_promise, async_frame);
  free_coroutine(coro, nullptr, false);
}

}  // namespace

/* Queue depth, for the registration ceiling in promise_then/promise_catch. */
size_t pending_promise_deliveries() { return g_promise_microtasks.size(); }

promise_t* promise_async_yield() {
  /* Checked BEFORE allocating, so the error() unwind has nothing to clean up.
   *
   * A pending yield is retained memory the caller can no longer see: unlike
   * promise_create(), dropping the returned value does NOT free it, because
   * the registry still holds a reference until the loop runs. So a sync loop
   * calling async_yield() and discarding the result would grow the registry
   * for as long as its eval budget lasts. That is the same runaway shape
   * promise_then()'s ceiling exists for -- a handler queueing work faster
   * than the driver retires it -- so it shares the same limit rather than
   * introducing a second knob for it. */
  LPC_INT const limit = CONFIG_INT(__RC_MAX_PENDING_DELIVERIES__);
  if (limit > 0 && static_cast<LPC_INT>(g_pending_yields.size()) >= limit) {
    error("async_yield: too many yields already waiting for the event loop (limit %d).\n",
          static_cast<int>(limit));
  }

  auto* p = promise_alloc();

  if (g_promises_shut_down) {
    /* Past promise_cleanup(): no further loop pass will happen, so there is
     * nothing to register the promise WITH -- and registering it would leak
     * it, since the registry that would free it has already been emptied.
     * The caller gets a promise that stays pending, which is what "the loop
     * will never run again" honestly is. Matches enqueue_reaction(), which
     * likewise drops work rather than queueing into a drained queue. */
    return p;
  }

  /* Registered before the ref is taken: push_back is the only step here that
   * can throw, and doing it first means a throw cannot leave an extra
   * reference on a promise nothing holds. */
  g_pending_yields.push_back(p);
  p->ref++; /* one for the caller, one for the registry */

  if (g_yield_event == nullptr) {
    g_yield_event = add_loop_yield_event(TickEvent::callback_type([] {
      g_yield_event = nullptr;
      if (g_promises_shut_down) {
        return;
      }
      /* Swapped out BEFORE settling. Settling only QUEUES deliveries, but
       * promise_settle() runs the adoption/self-resolve paths and a settle
       * can arm the drain, and a later delivery can call async_yield()
       * again -- which must land in the NEXT pass, on a fresh event, not be
       * appended to the batch being walked here. */
      std::vector<promise_t*> due;
      due.swap(g_pending_yields);
      for (auto* p : due) {
        /* Skip one whose fate LPC already committed to an adoption
         * (`promise_resolve(y, other)` with `other` still pending). The
         * efuns refuse a second settle in that state on the grounds that
         * the adoption owns the outcome; settling here anyway would
         * override it and silently discard the adopted value. */
        if (!p->resolving) {
          svalue_t zero = const0;
          (void)promise_settle(p, &zero, 0);
        }
        free_promise(p);
      }
    }));
  }
  return p;
}

promise_t* promise_alloc() {
  auto* p = reinterpret_cast<promise_t*>(
      DCALLOC(1, sizeof(promise_t), TAG_PROMISE, "promise_alloc"));
  p->ref = 1;
  p->state = PROMISE_PENDING;
  p->handled = false;
  p->resolving = false;
  p->body_owned = false;
  p->value_type = 0;
  p->result = const0;
  p->reactions = nullptr;
  p->reject_origin = nullptr;
  return p;
}

/* "/obj/name:42" for the currently executing LPC, or null if there is none
 * (a rejection made from a driver callback rather than from LPC). */
char* capture_reject_origin() {
  if (current_object == nullptr) {
    return nullptr;
  }
  const char* file = nullptr;
  int line = 0;
  if (current_prog != nullptr) {
    get_line_number_info(&file, &line);
  }
  char buf[512];
  if (file != nullptr && line > 0) {
    snprintf(buf, sizeof(buf), "/%s at %s:%d", current_object->obname, file, line);
  } else {
    snprintf(buf, sizeof(buf), "/%s", current_object->obname);
  }
  return string_copy(buf, "capture_reject_origin");
}

void free_promise(promise_t* p) {
  if (p->ref > 0) {
    p->ref--;
    if (p->ref == 0) {
      /* route through the deferred compound-free queue so dropping a long
       * then()-chain cannot recurse the C stack away (same protection as
       * arrays/mappings). */
      free_compound(p, T_PROMISE);
    }
  }
}

void dealloc_promise(promise_t* p) {
  if (p->state == PROMISE_REJECTED && !p->handled) {
    /* Where it was rejected. Without this the report is a bare reason with no
     * object, function or line -- and it is printed at DEALLOCATION, which
     * can be arbitrarily far from the rejection, so there is nothing else in
     * the log to correlate it with. */
    const char* const origin = p->reject_origin ? p->reject_origin : "unknown location";
    if (p->result.type == T_STRING) {
      debug_message("Unhandled promise rejection (rejected by %s): %s\n", origin,
                    p->result.u.string);
    } else {
      /* Render the VALUE where it is cheap to, not just its type name:
       * non-string reasons are routine here (the promise forms of
       * async_read/async_write/async_getdir reject with the callback
       * convention's negative int), and "Unhandled promise rejection:
       * (int)" tells the author nothing. Scalars only -- svalue_to_string()
       * lives in package/core, and this must stay allocation-light on a
       * deallocation path. */
      switch (p->result.type) {
        case T_NUMBER:
          debug_message("Unhandled promise rejection (rejected by %s): (int) %" LPC_INT_FMTSTR_P
                        "\n",
                        origin, p->result.u.number);
          break;
        case T_REAL:
          debug_message("Unhandled promise rejection (rejected by %s): (float) %f\n", origin,
                        static_cast<double>(p->result.u.real));
          break;
        case T_OBJECT:
          debug_message("Unhandled promise rejection (rejected by %s): (object) /%s\n", origin,
                        p->result.u.ob->obname ? p->result.u.ob->obname : "?");
          break;
        default:
          debug_message("Unhandled promise rejection (rejected by %s): (%s)\n", origin,
                        type_name(p->result.type));
          break;
      }
    }
  }
  if (p->reject_origin != nullptr) {
    FREE_MSTR(p->reject_origin);
    p->reject_origin = nullptr;
  }
  free_svalue(&p->result, "dealloc_promise");
  if (p->reactions) {
    for (auto& r : *p->reactions) {
      if (r.on_fulfilled) {
        free_funp(r.on_fulfilled);
      }
      if (r.on_rejected) {
        free_funp(r.on_rejected);
      }
      if (r.next) {
        /* An ADOPTION target must be settled, not merely released.
         * promise_resolve_with(next, p) committed next's fate to p and
         * latched next->resolving -- deliberately without taking a ref on p,
         * since that would build the cycle promise_cleanup() exists to
         * break. So when p dies unsettled, next is left PENDING with its
         * fate already spoken for, and both promise_resolve() and
         * promise_reject() refuse it on those grounds: the mudlib can never
         * take it back, and nothing is reported, because a promise stuck
         * pending is not a rejection. Anything awaiting it is parked for the
         * life of the driver, holding its object, program, frame slice and
         * one `max suspended async functions` slot -- invisible to both
         * abandon routes, since the object is alive and the promise it
         * awaits is alive; it is the SOURCE that died.
         *
         * Same safety net as the r.coro arm below, and gated on `resolving`
         * so an ordinary promise_then() chain keeps its documented
         * behaviour: dropping the last reference to a plain pending promise
         * frees its reactions and the awaiting side simply never runs, but
         * that promise is still one LPC can settle itself. Only the adopted
         * case is unrecoverable. promise_settle() only queues, so no LPC
         * runs here. */
        if (r.next->resolving && r.next->state == PROMISE_PENDING) {
          svalue_t err;
          err.type = T_STRING;
          err.subtype = STRING_CONSTANT;
          err.u.string = "*promise adoption source was collected before settling";
          (void)promise_settle(r.next, &err, 1);
        }
        free_promise(r.next);
      }
      if (r.command_giver) {
        free_object(&r.command_giver, "dealloc_promise");
      }
      if (r.coro) {
        /* the awaited promise died unsettled: the parked body can never
         * continue. Reject its own promise (queue-only, no LPC here). */
        svalue_t err;
        err.type = T_STRING;
        err.subtype = STRING_CONSTANT;
        err.u.string = "*awaited promise was collected before settling";
        free_coroutine(r.coro, &err, false);
      }
    }
    delete p->reactions;
    p->reactions = nullptr;
  }
  FREE(p);
}

int promise_settle(promise_t* p, svalue_t* value, int rejected) {
  if (p->state != PROMISE_PENDING) {
    return 0; /* first settle wins */
  }
  if (rejected && p->reject_origin == nullptr) {
    p->reject_origin = capture_reject_origin();
  }
  p->state = rejected ? PROMISE_REJECTED : PROMISE_FULFILLED;
  assign_svalue(&p->result, value);
  if (p->reactions) {
    std::vector<promise_reaction_t>* reactions = p->reactions;
    p->reactions = nullptr;
    for (auto& r : *reactions) {
      enqueue_reaction(p, &r); /* transfers the reaction's refs to the queue */
    }
    delete reactions;
  }
  return 1;
}

void promise_resolve_with(promise_t* p, svalue_t* value) {
  if (value->type == T_PROMISE) {
    promise_t* src = value->u.prom;
    if (src == p) {
      svalue_t err;
      err.type = T_STRING;
      err.subtype = STRING_CONSTANT;
      err.u.string = "*promise resolved with itself";
      promise_settle(p, &err, 1);
      return;
    }
    /* adopt: pass-through reaction chaining src's eventual state into p.
     * p stays PENDING until the delivery, but its fate is committed --
     * the efuns' double-settle guards key on `resolving` for this window. */
    p->resolving = true;
    p->ref++;
    src->handled = true; /* the rejection, if any, propagates to p */
    promise_add_reaction(src, nullptr, nullptr, p, nullptr);
    return;
  }
  promise_settle(p, value, 0);
}

void promise_add_reaction(promise_t* p, funptr_t* on_fulfilled, funptr_t* on_rejected,
                          promise_t* next, object_t* giver) {
  if (on_rejected || next) {
    p->handled = true;
  }
  promise_reaction_t r{on_fulfilled, on_rejected, next, giver, nullptr};
  if (p->state == PROMISE_PENDING) {
    if (!p->reactions) {
      p->reactions = new std::vector<promise_reaction_t>();
    }
    p->reactions->push_back(r);
  } else {
    enqueue_reaction(p, &r);
  }
}

/* Attach a parked coroutine to the promise it awaits. Ownership of `coro`
 * transfers to the promise machinery. */
static void promise_add_coroutine(promise_t* p, lpc_coroutine_t* coro) {
  p->handled = true; /* the await observes a rejection */
  promise_reaction_t r{nullptr, nullptr, nullptr, nullptr, coro};
  if (p->state == PROMISE_PENDING) {
    if (!p->reactions) {
      p->reactions = new std::vector<promise_reaction_t>();
    }
    p->reactions->push_back(r);
  } else {
    enqueue_reaction(p, &r);
  }
}

void run_async_function(char* entry_pc, const function_t* funp) {
  csp->framekind |= FRAME_ASYNC;
  /* The promise value carries its declared payload type in the svalue's
   * subtype, taken from the coroutine's declared return type -- promise<T>
   * for an `async T f()`. See promise_value_subtype(). */
  unsigned short const value_type = promise_value_subtype(promise_of_type(funp->type));
  promise_t* p = promise_alloc();
  /* also recorded on the promise itself, so paths that build a fresh
   * svalue for it (async_info()'s mapping) can restore the annotation */
  p->value_type = value_type;
  /* the body owns the settlement from here on; see promise_t::body_owned */
  p->body_owned = true;
#ifdef DEBUGMALLOC_EXTENSIONS
  g_active_body_promises.push_back(p);
  /* paired by RAII, not a tail pop: see the note in resume_coroutine(). A C++
   * exception escaping run_coroutine_body() would otherwise leave this entry
   * behind, and the ref checker would keep marking a promise that is gone. */
  DEFER { g_active_body_promises.pop_back(); };
#endif
  /* the caller set this frame up; nothing is stacked above it yet */
  bool const propagate_eval_error = run_coroutine_body(entry_pc, p, csp);
  if (propagate_eval_error) {
    /* do_catch() parity: the eval-cost limit cannot be swallowed. The
     * promise was already rejected; release our ref before unwinding.
     *
     * Mark it handled first. The caller never receives this promise -- the
     * error() below reaches it instead -- so the rejection IS delivered, by
     * the other route. Without this the drop reports "Unhandled promise
     * rejection" as well, and one eval-cost overrun is logged twice. */
    p->handled = true;
    free_promise(p);
    error("Can't catch eval cost too big error.\n");
  }
  push_refed_promise(p);
  sp->subtype = value_type;
}

void coroutine_await_pending(promise_t* awaited) {
  /* locate the async entry frame; only acatch() markers may sit between
   * it and the current frame (anything else means a C++ frame would be
   * pinned across the suspension) */
  control_stack_t* async_frame = nullptr;
  for (control_stack_t* f = csp; f >= control_stack; f--) {
    if ((f->framekind & FRAME_MASK) == FRAME_FUNCTION && (f->framekind & FRAME_ASYNC)) {
      async_frame = f;
      break;
    }
    if (!((f->framekind & FRAME_MASK) == FRAME_CATCH && (f->framekind & FRAME_ASYNC))) {
      break;
    }
  }
  if (!async_frame || !g_coroutine_promise) {
    error("await: not directly inside an async function body.\n");
  }
  /* Parking inside an object that is ALREADY destructed would create a frame
   * nothing can ever clean up: abandon_coroutines_of_object() runs once, from
   * destruct_object(), so it only sees frames that were parked by then, and
   * resume_coroutine()'s destructed check is only reached if the awaited
   * promise settles -- which for a promise nobody holds a reference to it
   * never does. The frame would then pin the destructed object, its program
   * and a suspension slot for the life of the driver. An async body can
   * easily reach here after its own object is gone: the ordinary die()/quit()
   * shape is `await something; cleanup(); destruct(this_object()); await
   * something_else;`. Refuse to suspend, which rejects the body's promise
   * with this message, exactly as any other error in an async body does. */
  if (current_object->flags & O_DESTRUCTED) {
    error("await: cannot suspend inside a destructed object.\n");
  }
  {
    auto const limit = CONFIG_INT(__RC_MAX_SUSPENDED_ASYNC__);
    /* g_live_coroutines still holds coroutines that are not suspended: the
     * one being resumed (its heap frame copy is already freed) and the one
     * being torn down in its defer handlers. Count what async_info() counts,
     * or the ceiling differs by path and an await from a resume or a
     * teardown defer fails while async_info() reports free slots. */
    LPC_INT const suspended = static_cast<LPC_INT>(suspended_coroutine_count());
    if (limit > 0 && suspended >= limit) {
      error("await: too many suspended async functions (limit %d).\n", static_cast<int>(limit));
    }
  }
  /* transient references into the stacks cannot be parked */
  for (svalue_t* v = fp; v < sp; v++) {
    if (v->type &
        (T_LVALUE | T_LVALUE_BYTE | T_LVALUE_RANGE | T_LVALUE_CODEPOINT | T_REF | T_ERROR_HANDLER)) {
      error(
          "await: cannot suspend while a reference or lvalue is pending on the stack. "
          "Inside a `foreach` loop, use an indexed `for` loop instead; for a `ref` "
          "argument, await into a plain variable first and pass that.\n");
    }
  }

  auto* coro = new lpc_coroutine_t{};
  coro->id = ++g_next_coroutine_id;
  coro->awaiting = awaited;
  coro->result_promise = g_coroutine_promise;
  g_coroutine_promise->ref++;
  coro->ob = current_object;
  add_ref(current_object, "coroutine");
  coro->prev_ob = previous_ob;
  if (previous_ob) {
    add_ref(previous_ob, "coroutine");
  }
  coro->command_giver = command_giver;
  if (command_giver) {
    add_ref(command_giver, "coroutine");
  }
  coro->prog = current_prog;
  reference_prog(current_prog, "coroutine");
  /* the owner's top-level program: for an INHERITED async function this is
   * not current_prog (see the field comment), and resume_coroutine()'s
   * replace_program() guard must compare against this one */
  coro->object_prog = current_object->prog;
  coro->prog_generation = current_object->prog_generation;
  coro->pc_offset = pc - current_prog->program;
  coro->caller_type = caller_type;
  coro->function_index_offset = function_index_offset;
  coro->variable_index_offset = variable_index_offset;
  coro->table_index = async_frame->fr.table_index;
  coro->num_local_variables = csp->num_local_variables;
  coro->defers = async_frame->defers;
  async_frame->defers = nullptr;
  for (control_stack_t* f = async_frame + 1; f <= csp; f++) {
    coro->markers.push_back({static_cast<int>(f->pc - current_prog->program),
                             static_cast<int>(f->save_sp - fp), f->defers});
    f->defers = nullptr;
  }

  /* bitwise ownership transfer of the frame slice (the awaited promise at
   * sp stays out of it) */
  int const n = sp - fp;
  coro->frame_size = n;
  if (n > 0) {
    coro->frame = new svalue_t[n];
    memcpy(coro->frame, fp, n * sizeof(svalue_t));
  }

  /* registered from the moment it exists: every free_coroutine() erases */
  g_live_coroutines[coro->id] = coro;
  coro->registered = true;
  g_coroutines_by_owner[coro->ob].push_back(coro->id);

  /* hand the coroutine to the awaited promise, then drop the stack's ref
   * on it -- if that was the last ref the promise can never settle and the
   * coroutine is abandoned right here (its own promise rejects). */
  svalue_t promise_slot = *sp;
  sp = fp - 1;
  promise_add_coroutine(awaited, coro);
  free_svalue(&promise_slot, "coroutine_await_pending");

  /* pop the markers and the async frame; defers were transferred above */
  while (csp >= async_frame) {
    pop_control_stack();
  }
  g_coroutine_suspended = true;
}

void free_coroutine_orphan(lpc_coroutine_t* coro) { free_coroutine(coro, nullptr, false); }

/* Abandon every frame parked inside `ob`, called from destruct_object().
 *
 * Without this a parked frame is only ever discovered LAZILY, by
 * resume_coroutine()'s destructed check when the awaited promise settles --
 * so a frame awaiting a promise that never settles is never cleaned up at
 * all. That is not a corner case: the parked frame's own copy of a local
 * holding the promise keeps it alive, which also disables the "promise was
 * collected while pending" safety net, so both documented abandonment paths
 * miss together. `async int run() { promise p = promise_create(); await p; }`
 * on a destructed object stayed suspended forever, pinning the object, its
 * program, its saved frame and one of the max-suspended-async slots, and
 * invisible to check_memory() because the registry kept it reachable.
 *
 * The coroutine is owned by the awaited promise's reaction list, so it has
 * to be unhooked from there before being freed. Defers are NOT run: they are
 * funptrs owned by the object being destructed, and call_function_pointer()
 * refuses those -- see the note in docs/concepts/general/async.md. */
void abandon_coroutines_of_object(object_t* ob) {
  /* O(this object's frames), not O(every parked frame in the driver). The
   * owner index answers "has this object any?" in one hash lookup, which is
   * the answer for every destruct in a mud that does not use async at all --
   * previously each of those walked the whole registry, so a 1000-clone
   * reset sweep paid ~75ms at the default ceiling for nothing. */
  auto owned = g_coroutines_by_owner.find(ob);
  if (owned == g_coroutines_by_owner.end()) {
    return;
  }
  /* Collect this object's parked frames in ONE pass, by id.
   *
   * The obvious shape -- re-scan for the next victim after each free -- is
   * correct but quadratic, and reachably so: an object with 9000 parked
   * frames (under the default 10000 ceiling) took 600ms to destruct, during
   * which nothing else in the driver runs. That is the very property the
   * drain's batching exists to guarantee, undone at destruct time. Equally,
   * building the skip-set by scanning the whole microtask deque made EVERY
   * destruct_object() in the driver pay O(pending deliveries) even when the
   * object had no parked frame at all; the coroutine's own `queued` flag
   * answers that in O(1).
   *
   * Ids, not pointers: freeing one frame settles its result promise, which
   * can hand another of this object's frames to the microtask queue or free
   * it outright through a dealloc cascade, so every candidate is re-validated
   * against the registry immediately before it is touched. */
  std::vector<uint64_t> candidates;
  /* by value: freeing a frame can add to or erase from the owner's list */
  std::vector<uint64_t> const mine = owned->second;
  for (uint64_t const cid : mine) {
    auto entry = g_live_coroutines.find(cid);
    if (entry == g_live_coroutines.end()) {
      continue;
    }
    lpc_coroutine_t* coro = entry->second;
    if (coro->queued) {
      /* Owned by the microtask queue; resume_coroutine() abandons it on
       * arrival. Freeing it here would strand that delivery -- but it is
       * dead as of now, so stop reporting it as suspended.
       *
       * The `!running` half is redundant today (ownership leaves the queue
       * before either running global is set) and kept so that the mark stays
       * disjoint from the other two discounts if that ever changes: marking a
       * running frame would subtract it twice and underflow the counter for
       * good. */
      if (!coro->abandoned && !coroutine_is_running(coro)) {
        coro->abandoned = true;
        g_abandoned_queued++;
      }
      continue;
    }
    if (!coroutine_is_running(coro)) {
      candidates.push_back(cid);
    }
  }
#ifdef DEBUG
  /* The index is only as good as its two sync points. Cross-check it against
   * the registry on debug builds: a missed entry here means a parked frame
   * survives its object's destruct forever, which is exactly the bug this
   * function exists to prevent and is otherwise silent. */
  for (auto& entry : g_live_coroutines) {
    DEBUG_CHECK(entry.second->ob == ob &&
                    std::find(mine.begin(), mine.end(), entry.first) == mine.end(),
                "abandon_coroutines_of_object: owner index missed a parked frame\n");
  }
#endif

  for (uint64_t id : candidates) {
    auto it = g_live_coroutines.find(id);
    if (it == g_live_coroutines.end()) {
      continue; /* freed by a cascade since the scan */
    }
    lpc_coroutine_t* coro = it->second;
    /* Re-check both conditions: a settle during this loop can have handed
     * this frame to the queue (ownership moves with it, and its resume is
     * already scheduled -- resume_coroutine() abandons a destructed owner on
     * arrival), and the running/teardown state can have changed. */
    if (coro->queued) {
      if (!coro->abandoned && !coroutine_is_running(coro)) {
        coro->abandoned = true;
        g_abandoned_queued++;
      }
      continue;
    }
    if (coroutine_is_running(coro)) {
      continue;
    }

    /* Unhook from the awaited promise's reaction list, which still owns it.
     * (Had that promise settled, the entry would have moved to the queue and
     * the guard above would have skipped this frame.) */
    promise_t* awaited = coro->awaiting;
    if (awaited != nullptr && awaited->reactions != nullptr) {
      for (auto rit = awaited->reactions->begin(); rit != awaited->reactions->end(); ++rit) {
        if (rit->coro == coro) {
          awaited->reactions->erase(rit);
          break;
        }
      }
    }
    coro->awaiting = nullptr;

    svalue_t reason;
    reason.type = T_STRING;
    reason.subtype = STRING_MALLOC;
    reason.u.string = string_copy(kDestructedRejection, "abandon_coroutines_of_object");
    /* No LPC runs here (promise_settle only queues, and defers are discarded
     * -- see the note in docs/concepts/general/async.md), so no new frame can
     * park and the candidate list cannot grow underneath us. */
    free_coroutine(coro, &reason, false);
    free_svalue(&reason, "abandon_coroutines_of_object");
  }
}


mapping_t* build_async_scheduler_info() {
  /* allocate_mapping can error() (OOM), and add_mapping_pair can too, so the
   * mapping under construction is guarded exactly like build_async_info's
   * (AGENTS.md section 4). */
  mapping_t* m = allocate_mapping(4);
  bool done = false;
  DEFER {
    if (!done) {
      free_mapping(m);
    }
  };

  add_mapping_pair(m, "suspended", static_cast<long>(suspended_coroutine_count()));
  add_mapping_pair(m, "pending_deliveries", static_cast<long>(g_promise_microtasks.size()));
  /* monotonic: every slice that spent its time with work still queued and
   * handed the rest back through the event loop */
  add_mapping_pair(m, "drain_yields", g_drain_yields_total);
  /* monotonic: settles that arrived from outside gametick dispatch and are
   * therefore served by an event-loop arming rather than by the tick queue */
  add_mapping_pair(m, "drain_arms_loop", g_drain_arms_loop_total);
  /* the EFFECTIVE budget, not the raw config: 0 there means "use the
   * default", and a caller watching the scheduler wants the real number */
  add_mapping_pair(m, "drain_eval_budget", drain_eval_budget_us());

  done = true;
  return m;
}

array_t* build_async_info() {
  size_t n = 0;
  for (auto& entry : g_live_coroutines) {
    if (!coroutine_is_running(entry.second) && !entry.second->abandoned) {
      n++;
    }
  }
  /* allocate_array, not allocate_empty_array: the items start as 0 so the
   * error guard below can free a partially built result. The add_mapping_*
   * calls can error() (mapping-too-large, OOM) and until the return this
   * function is the only owner of the array and the mapping under
   * construction -- without the guard both leak on the unwind (AGENTS.md
   * section 4; same shape as mudlib_error_handler's diagnostic mapping). */
  array_t* v = allocate_array(n);
  mapping_t* m = nullptr;
  bool done = false;
  DEFER {
    if (!done) {
      if (m != nullptr) {
        free_mapping(m);
      }
      free_array(v);
    }
  };
  int i = 0;

  for (auto& entry : g_live_coroutines) {
    lpc_coroutine_t* coro = entry.second;
    if (coroutine_is_running(coro) || coro->abandoned) {
      continue;  // running, being torn down, or dead awaiting its delivery
    }
    m = allocate_mapping(8);
    const char* file = nullptr;
    int line = 0;

    add_mapping_pair(m, "id", coro->id);
    add_mapping_object(m, "object", coro->ob);
    /* table_index indexes the DEFINING program's function_table directly
     * (setup_new_frame() stores findex - last_inherited), which is how every
     * other consumer reads it -- interpret.cc's trace rendering and
     * get_trace_details() both do prog->function_table[table_index].
     * function_name() takes a RUNTIME index into function_flags[] instead,
     * so feeding it this one names a different function whenever the
     * defining program inherits anything. */
    add_mapping_string(m, "function",
                       coro->table_index >= 0 &&
                               coro->table_index < static_cast<long>(coro->prog->num_functions_defined)
                           ? coro->prog->function_table[coro->table_index].funcname
                           : "<unknown>");
    get_explicit_line_number_info(coro->prog->program + coro->pc_offset, coro->prog, &file, &line);
    add_mapping_malloced_string(m, "file", add_slash(file));
    add_mapping_pair(m, "line", line);
    /* the awaited promise has settled and the resume is already queued */
    add_mapping_pair(m, "ready", coro->awaiting->state != PROMISE_PENDING);
    /* how many acatch() regions the await sits inside */
    add_mapping_pair(m, "acatch_depth", static_cast<long>(coro->markers.size()));

    add_mapping_promise(m, "promise", coro->result_promise);
    add_mapping_promise(m, "awaiting", coro->awaiting);

    v->item[i].type = T_MAPPING;
    v->item[i].u.map = m;
    m = nullptr; /* ownership now in the array */
    i++;
  }
  done = true;
  return v;
}

void push_refed_promise(promise_t* p) {
  STACK_INC;
  sp->type = T_PROMISE;
  sp->subtype = 0;
  sp->u.prom = p;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_coroutine(lpc_coroutine_t* coro) {
  coro->result_promise->extra_ref++;
  coro->ob->extra_ref++;
  if (coro->prev_ob) {
    coro->prev_ob->extra_ref++;
  }
  if (coro->command_giver) {
    coro->command_giver->extra_ref++;
  }
  coro->prog->extra_ref++;
  for (int i = 0; i < coro->frame_size; i++) {
    mark_svalue(&coro->frame[i]);
  }
  for (struct defer_list* d = coro->defers; d; d = d->next) {
    mark_svalue(&d->func);
    mark_svalue(&d->tp);
  }
  for (auto& m : coro->markers) {
    for (struct defer_list* d = m.defers; d; d = d->next) {
      mark_svalue(&d->func);
      mark_svalue(&d->tp);
    }
  }
}

void mark_promise(promise_t* p) {
  mark_svalue(&p->result);
  /* A raw malloced string held by the struct rather than by an svalue, so
   * nothing else marks it: without this the checker reports it as an
   * unaccounted reference after every file that leaves a rejected promise
   * around (AGENTS.md section 3). */
  if (p->reject_origin != nullptr) {
    MSTR_EXTRA_REF(p->reject_origin)++;
  }
  if (p->reactions) {
    for (auto& r : *p->reactions) {
      if (r.on_fulfilled) {
        r.on_fulfilled->hdr.extra_ref++;
      }
      if (r.on_rejected) {
        r.on_rejected->hdr.extra_ref++;
      }
      if (r.next) {
        r.next->extra_ref++;
      }
      if (r.command_giver) {
        r.command_giver->extra_ref++;
      }
      if (r.coro) {
        mark_coroutine(r.coro);
      }
    }
  }
}

void mark_promise_queue() {
  /* Off-graph: a pending async_yield() promise is referenced only by the
   * registry, which is a C++ global the allocation sweep never walks. */
  for (auto* p : g_pending_yields) {
    p->extra_ref++;
  }
  auto mark_one = [](QueuedReaction& qr) {
    if (qr.on_fulfilled) {
      qr.on_fulfilled->hdr.extra_ref++;
    }
    if (qr.on_rejected) {
      qr.on_rejected->hdr.extra_ref++;
    }
    if (qr.next) {
      qr.next->extra_ref++;
    }
    if (qr.command_giver) {
      qr.command_giver->extra_ref++;
    }
    if (qr.coro) {
      mark_coroutine(qr.coro);
    }
    qr.source->extra_ref++;
  };
  if (g_delivering != nullptr) {
    mark_one(*g_delivering);
  }
  /* refs held only by C++ locals while an async body runs (see the
   * declarations): first-run result promises, and the resuming coroutine
   * (deliver_reaction nulls qr->coro before resume_coroutine, so the
   * g_delivering mark above no longer covers it). mark_coroutine() is safe
   * mid-run: frame and defers are nulled/transferred during resume, so it
   * bumps exactly the outstanding result_promise/ob/prev_ob/command_giver/
   * prog refs. */
  for (auto* p : g_active_body_promises) {
    p->extra_ref++;
  }
  if (g_resuming_coro != nullptr) {
    mark_coroutine(g_resuming_coro);
  }
  if (g_freeing_coro != nullptr && g_freeing_coro != g_resuming_coro) {
    mark_coroutine(g_freeing_coro);
  }
  for (auto& qr : g_promise_microtasks) {
    if (qr.on_fulfilled) {
      qr.on_fulfilled->hdr.extra_ref++;
    }
    if (qr.on_rejected) {
      qr.on_rejected->hdr.extra_ref++;
    }
    if (qr.next) {
      qr.next->extra_ref++;
    }
    if (qr.command_giver) {
      qr.command_giver->extra_ref++;
    }
    if (qr.coro) {
      mark_coroutine(qr.coro);
    }
    qr.source->extra_ref++;
  }
}
#endif

void promise_cleanup() {
  g_promises_shut_down = true;
  /* The TickEvent g_drain_event points at is owned by the backend and is
   * deleted by clear_tick_events()/clear_walltime_events(), which run AFTER
   * this in the shutdown sequence. Drop our pointer now rather than relying
   * on g_promises_shut_down keeping every later reader away from it. */
  g_drain_event = nullptr;
  g_drain_event_is_tick = false;
  g_drain_scheduled = false;
  /* Same reasoning for the yield event, whose TickEvent the backend deletes
   * (without running it) in clear_walltime_events().
   *
   * SETTLE-REJECT first, do not merely drop the reference. A parked coroutine
   * does NOT hold a ref on the promise it awaits -- coroutine_await_pending()
   * drops the stack slot's ref once the coroutine is registered as a reaction
   * -- so for the ordinary shape
   *
   *     promise p = async_yield();   // the frame local is the surviving ref
   *     await p;
   *
   * the last reference lives INSIDE the parked coroutine's saved frame, and
   * the coroutine is reachable only from p->reactions. That is a refcount
   * cycle: free_promise() never reaches zero, dealloc_promise()'s
   * reaction-cleanup never runs, and the coroutine, its frame svalues (with
   * the object and program refs they hold) and both promises leak at exit.
   * Measured at 328 bytes in 6 allocations, all reported Indirect with no
   * Direct root -- LeakSanitizer's signature for a pure cycle.
   *
   * Settling breaks it: the reaction is handed to enqueue_reaction(), which
   * short-circuits to free_queued_reaction() because g_promises_shut_down is
   * already set above, freeing the coroutine synchronously.
   *
   * This is what the sibling teardown already does and what simulate.cc's
   * shutdown ordering comment demands of it: clear_call_outs() rejects every
   * call_out promise for exactly this reason. A driver-owned promise must be
   * SETTLED at teardown, never just released (AGENTS.md section 15 -- the
   * ownership step that only some sibling paths take). */
  g_yield_event = nullptr;
  for (auto* p : g_pending_yields) {
    svalue_t err;
    err.type = T_STRING;
    err.subtype = STRING_CONSTANT;
    err.u.string = "*async_yield never ran: the driver shut down";
    (void)promise_settle(p, &err, 1);
    free_promise(p);
  }
  g_pending_yields.clear();
  while (!g_promise_microtasks.empty()) {
    QueuedReaction qr = g_promise_microtasks.front();
    g_promise_microtasks.pop_front();
    free_queued_reaction(&qr);
  }
}
