#include "base/std.h"

#include "vm/internal/base/machine.h"

#include <deque>

#include "backend.h"
#include "vm/internal/eval_limit.h"

/*
 * Native LPC promises (issue #1319 phase 1). See promise.h for the ownership
 * model. Delivery runs from a gametick event, so a reaction queued by a
 * settle in this execution runs after the current execution finishes, within
 * the same tick (call_tick_events() re-loops until the tick's queue drains).
 */

/* The error context of the innermost running coroutine body boundary
 * (run_coroutine_body). error_handler()/throw_error() treat an error that
 * unwinds to this context like a caught one: the message lands in
 * catch_value and becomes a rejection (or resumes an acatch() region). */
error_context_t* g_coroutine_econ = nullptr;

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

/* Fairness bound: a self-feeding then() chain must not wedge the driver
 * inside one tick. Leftover work is rescheduled to the next gametick. */
constexpr int kMaxPromiseDeliveriesPerTick = 65536;

void drain_promise_microtasks();

void schedule_drain() {
  if (!g_drain_scheduled) {
    g_drain_scheduled = true;
    add_gametick_event(0, TickEvent::callback_type([] { drain_promise_microtasks(); }));
  }
}

void enqueue_reaction(promise_t* source, promise_reaction_t* r) {
  source->ref++;
  g_promise_microtasks.push_back(
      QueuedReaction{r->on_fulfilled, r->on_rejected, r->next, r->command_giver, r->coro, source});
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
    restore_context(&econ);
    /* take ownership of the caught error value, like do_catch() */
    *out = catch_value;
    catch_value = const1;
    if (max_eval_error) {
      set_eval(max_eval_cost);
      max_eval_error = 0;
    }
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
    resume_coroutine(qr->coro, src);
    free_promise(qr->source);
    return;
  }

  bool const rejected = (src->state == PROMISE_REJECTED);
  funptr_t* handler = rejected ? qr->on_rejected : qr->on_fulfilled;

  object_t* giver = qr->command_giver;
  if (giver && (giver->flags & O_DESTRUCTED)) {
    giver = nullptr;
  }
  save_command_giver(giver);
  set_eval(max_eval_cost);

  if (handler) {
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

  if (qr->on_fulfilled) {
    free_funp(qr->on_fulfilled);
  }
  if (qr->on_rejected) {
    free_funp(qr->on_rejected);
  }
  if (qr->next) {
    free_promise(qr->next);
  }
  if (qr->command_giver) {
    free_object(&qr->command_giver, "deliver_reaction");
  }
  free_promise(qr->source);
}

void drain_promise_microtasks() {
  g_drain_scheduled = false;
  int budget = kMaxPromiseDeliveriesPerTick;
  while (!g_promise_microtasks.empty() && budget-- > 0) {
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
    try {
      deliver_reaction(&qr);
    } catch (const char*) {
      restore_context(&econ);
      free_svalue(&catch_value, "drain_promise_microtasks");
      catch_value = const0;
      too_deep_error = 0;
      max_eval_error = 0;
    }
    pop_context(&econ);
  }
  if (!g_promise_microtasks.empty()) {
    debug_message("promise: %d deliveries deferred to the next gametick.\n",
                  static_cast<int>(g_promise_microtasks.size()));
    schedule_drain();
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
 * runs for an acatch: control stack, command-giver stack, value stack, and
 * the csp-anchored ref list. Missing the command-giver unwind leaked one
 * cgsp entry per caught error and eventually overflowed that fixed array. */
char* unwind_to_acatch_marker(control_stack_t* marker) {
  while (csp > marker) {
    pop_control_stack();
  }
  while (cgsp != marker->save_cgsp) {
    restore_command_giver();
  }
  while (sp > marker->save_sp) {
    pop_stack();
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
  *sp = catch_value;
  catch_value = const1;
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
      if (!max_eval_error && !too_deep_error) {
        marker = find_acatch_marker(econ.save_csp);
      }
      if (marker) {
        entry_pc = unwind_to_acatch_marker(marker);
        continue;
      }
      restore_context(&econ);
      svalue_t err = catch_value;
      catch_value = const1;
      (void)promise_settle(p, &err, 1);
      free_svalue(&err, "run_coroutine_body");
      too_deep_error = 0;
      if (max_eval_error) {
        propagate_eval_error = true;
      }
      break;
    }
  }

  g_coroutine_econ = prev_econ;
  g_coroutine_promise = prev_promise;
  pop_context(&econ);
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
    if (run_lpc) {
      /* a scratch frame whose pop runs the defers with full semantics */
      push_control_stack(FRAME_CATCH);
      csp->defers = all;
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
  g_live_coroutines.erase(coro->id);
  delete coro;
}

void resume_coroutine(lpc_coroutine_t* coro, promise_t* source) {
  bool const rejected = (source->state == PROMISE_REJECTED);

  if ((coro->ob->flags & O_DESTRUCTED) || coro->prog_generation != coro->ob->prog_generation) {
    svalue_t err;
    err.type = T_STRING;
    err.subtype = STRING_CONSTANT;
    err.u.string = (coro->ob->flags & O_DESTRUCTED)
                       ? "*async function owner was destructed while suspended"
                       : "*async function owner was recompiled while suspended";
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

  set_eval(max_eval_cost);
  object_t* giver =
      (coro->command_giver && !(coro->command_giver->flags & O_DESTRUCTED)) ? coro->command_giver
                                                                            : nullptr;
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

  (void)run_coroutine_body(entry, coro->result_promise, async_frame);
  /* top level: nothing to propagate an eval-cost error to */
  max_eval_error = 0;

  restore_command_giver();
  free_coroutine(coro, nullptr, false);
}

}  // namespace

promise_t* promise_alloc() {
  auto* p = reinterpret_cast<promise_t*>(
      DCALLOC(1, sizeof(promise_t), TAG_PROMISE, "promise_alloc"));
  p->ref = 1;
  p->state = PROMISE_PENDING;
  p->handled = false;
  p->result = const0;
  p->reactions = nullptr;
  return p;
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
    if (p->result.type == T_STRING) {
      debug_message("Unhandled promise rejection: %s\n", p->result.u.string);
    } else {
      debug_message("Unhandled promise rejection: (%s)\n", type_name(p->result.type));
    }
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
    /* adopt: pass-through reaction chaining src's eventual state into p */
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

void run_async_function(char* entry_pc) {
  csp->framekind |= FRAME_ASYNC;
  promise_t* p = promise_alloc();
  /* the caller set this frame up; nothing is stacked above it yet */
  bool const propagate_eval_error = run_coroutine_body(entry_pc, p, csp);
  if (propagate_eval_error) {
    /* do_catch() parity: the eval-cost limit cannot be swallowed. The
     * promise was already rejected; release our ref before unwinding. */
    free_promise(p);
    error("Can't catch eval cost too big error.\n");
  }
  push_refed_promise(p);
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
  {
    auto const limit = CONFIG_INT(__RC_MAX_SUSPENDED_ASYNC__);
    if (limit > 0 && static_cast<LPC_INT>(g_live_coroutines.size()) >= limit) {
      error("await: too many suspended async functions (limit %d).\n", static_cast<int>(limit));
    }
  }
  /* transient references into the stacks cannot be parked */
  for (svalue_t* v = fp; v < sp; v++) {
    if (v->type &
        (T_LVALUE | T_LVALUE_BYTE | T_LVALUE_RANGE | T_LVALUE_CODEPOINT | T_REF | T_ERROR_HANDLER)) {
      error(
          "await: cannot suspend while a reference or lvalue is pending on the stack "
          "(e.g. `arr[i] += await p` or a `foreach ref` loop); await into a plain "
          "variable first.\n");
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

array_t* build_async_info() {
  array_t* v = allocate_empty_array(g_live_coroutines.size());
  int i = 0;

  for (auto& entry : g_live_coroutines) {
    lpc_coroutine_t* coro = entry.second;
    mapping_t* m = allocate_mapping(8);
    const char* file = nullptr;
    int line = 0;

    add_mapping_pair(m, "id", coro->id);
    add_mapping_object(m, "object", coro->ob);
    add_mapping_string(m, "function", function_name(coro->prog, coro->table_index));
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
    i++;
  }
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
  while (!g_promise_microtasks.empty()) {
    QueuedReaction qr = g_promise_microtasks.front();
    g_promise_microtasks.pop_front();
    if (qr.on_fulfilled) {
      free_funp(qr.on_fulfilled);
    }
    if (qr.on_rejected) {
      free_funp(qr.on_rejected);
    }
    if (qr.next) {
      free_promise(qr.next);
    }
    if (qr.command_giver) {
      free_object(&qr.command_giver, "promise_cleanup");
    }
    if (qr.coro) {
      /* shutdown: free without running any LPC */
      free_coroutine(qr.coro, nullptr, false);
    }
    free_promise(qr.source);
  }
}
