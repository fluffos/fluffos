#include "base/package_api.h"

#include "packages/core/call_out.h"
#include "include/promise.h"  // LPC-visible rejection reasons

/*
 * Promise efuns (issue #1319 phase 1). The T_PROMISE machinery itself lives
 * in vm/internal/base/promise.cc; these are the LPC-visible entry points.
 *
 * Promise arguments are spec-typed `promise`, so the VM's efun dispatch has
 * already validated the top-level tag before these run (AGENTS.md section 2).
 */

#ifdef F_PROMISE_CREATE
vm::Value f_promise_create(vm::EfunArgs&) { return vm::Value::refed(promise_alloc()); }
#endif

#ifdef F_PROMISE_RESOLVE
void f_promise_resolve(vm::EfunArgs&, promise_t* p, std::optional<vm::Arg> value_arg) {
  if (p->state != PROMISE_PENDING || p->resolving) {
    /* `resolving`: a prior resolve with a still-pending promise committed
     * this promise's fate to the adoption; without the check, a second
     * resolve/reject would race the adoption's delivery, silently
     * discarding one of them depending on tick timing. */
    error("promise_resolve: promise is already settled.\n");
  }
  if (p->body_owned) {
    /* checked after the settled guard: a body that already returned is more
     * usefully reported as "already settled" */
    error(
        "promise_resolve: promise belongs to an async function; only its body "
        "can settle it.\n");
  }
  svalue_t* value = value_arg ? value_arg->sv() : &const0;
  if (value->type == T_PROMISE && value->u.prom == p) {
    error("promise_resolve: cannot resolve a promise with itself.\n");
  }
  promise_resolve_with(p, value);
}
#endif

#ifdef F_PROMISE_REJECT
void f_promise_reject(vm::EfunArgs&, promise_t* p, std::optional<vm::Arg> reason) {
  if (p->state != PROMISE_PENDING || p->resolving) {
    /* see promise_resolve: an in-flight adoption already commits the fate */
    error("promise_reject: promise is already settled.\n");
  }
  if (p->body_owned) {
    /* see promise_resolve: settling a running body's promise would silently
     * discard whatever it returns, and would not stop it running */
    error(
        "promise_reject: promise belongs to an async function; only its body "
        "can settle it.\n");
  }
  /* mirror promise_resolve's guard: a promise holding itself is a cycle no
   * collector here can break (%O itself is depth-bounded, so it prints
   * fine -- the problem is the leak, not the printing) */
  if (reason && reason->is(T_PROMISE) && reason->promise() == p) {
    error("promise_reject: cannot reject a promise with itself.\n");
  }
  /* A rejection reason must be TRUTHY by default. acatch(), like catch(),
   * signals failure by yielding the reason and success by yielding 0, so a
   * falsy reason is indistinguishable from success -- `mixed err =
   * acatch(await p); if (err) ...` would take the success branch on a real
   * rejection, which is the idiom async.md itself documents. catch() never
   * has this problem because the driver always produces a "*..." string, so
   * a bare promise_reject() now does the same. An explicitly falsy reason
   * (promise_reject(p, 0)) is still the caller's choice, and still
   * ambiguous -- promise_status() is the unambiguous test. */
  svalue_t default_reason;
  default_reason.type = T_STRING;
  default_reason.subtype = STRING_CONSTANT;
  default_reason.u.string = PROMISE_REASON_NO_REASON;
  promise_settle(p, reason ? reason->sv() : &default_reason, 1);
}
#endif

/* shared body of promise_then() / promise_catch() */
static vm::Value promise_then_common(promise_t* p, funptr_t* on_fulfilled, funptr_t* on_rejected) {
  /* Refuse to register more work when the delivery queue is already
   * saturated. Without this a handler that attaches more reactions than the
   * drain retires grows g_promise_microtasks without bound -- one call to a
   * two-reaction self-feeding function reached 2.3 million queued deliveries
   * in a couple of seconds here, and aborts the driver with std::bad_alloc
   * once memory runs out. There is no LPC frame to blame and no eval budget
   * to exceed: every delivery is armed with a fresh one.
   *
   * `call_out(0) nest level` refuses exactly this shape for same-tick
   * call_outs (call_out.cc); promises reached it again with no guard. Checked
   * HERE, at the top of the efun before anything is allocated or ref'd, so
   * the error unwinds with nothing half-built (AGENTS.md section 4). */
  {
    LPC_INT const limit = CONFIG_INT(__RC_MAX_PENDING_DELIVERIES__);
    if (limit > 0 && (LPC_INT)pending_promise_deliveries() >= limit) {
      error("promise_then: too many promise deliveries already pending (limit %d).\n",
            (int)limit);
    }
  }

  object_t* giver = nullptr;
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__) && command_giver) {
    giver = command_giver;
    add_ref(giver, "promise_then");
  }

  promise_t* chained = promise_alloc();
  chained->ref++; /* one ref for the reaction, one for the return value */
  if (on_fulfilled) {
    on_fulfilled->hdr.ref++;
  }
  if (on_rejected) {
    on_rejected->hdr.ref++;
  }
  promise_add_reaction(p, on_fulfilled, on_rejected, chained, giver);

  return vm::Value::refed(chained);
}

#ifdef F_PROMISE_THEN
vm::Value f_promise_then(vm::EfunArgs&, promise_t* p, std::optional<funptr_t*> on_fulfilled,
                         std::optional<funptr_t*> on_rejected) {
  return promise_then_common(p, on_fulfilled.value_or(nullptr), on_rejected.value_or(nullptr));
}
#endif

#ifdef F_PROMISE_CATCH
vm::Value f_promise_catch(vm::EfunArgs&, promise_t* p, funptr_t* on_rejected) {
  return promise_then_common(p, nullptr, on_rejected);
}
#endif

#ifdef F_PROMISE_STATUS
vm::Value f_promise_status(vm::EfunArgs&, promise_t* p) {
  int const state = p->state;
  return state;
}
#endif

#ifdef F_PROMISE_RESULT
vm::Value f_promise_result(vm::EfunArgs&, promise_t* p) {
  if (p->state == PROMISE_PENDING) {
    error("promise_result: promise is still pending.\n");
  }
  if (p->state == PROMISE_REJECTED || p->state == PROMISE_CANCELLED) {
    /* reading a rejection or cancellation counts as observing it */
    p->handled = true;
  }
  return vm::Value::copy(&p->result);
}
#endif

/* shared body of the four combinators; the array arg is spec-typed, so only
 * its ELEMENTS need checking (AGENTS.md section 2) -- and they need none: a
 * non-promise element is defined to count as already fulfilled with itself,
 * so map() output drops straight in. */
static vm::Value promise_combinator_efun(array_t* inputs, uint8_t kind) {
  if (kind == PROMISE_COMB_RACE && inputs->size == 0) {
    /* JS lets this hang forever. Here a hung await is a parked frame holding
     * its object, its program and one `max suspended async functions` slot
     * for the life of the driver, so refuse it where the mistake is. */
    error("promise_race: needs at least one promise; an empty array would never settle.\n");
  }
  promise_t* result = promise_combinator_start(kind, inputs);
  return vm::Value::refed(result);
}

#ifdef F_PROMISE_ALL
vm::Value f_promise_all(vm::EfunArgs&, array_t* inputs) {
  return promise_combinator_efun(inputs, PROMISE_COMB_ALL);
}
#endif

#ifdef F_PROMISE_ANY
vm::Value f_promise_any(vm::EfunArgs&, array_t* inputs) {
  return promise_combinator_efun(inputs, PROMISE_COMB_ANY);
}
#endif

#ifdef F_PROMISE_RACE
vm::Value f_promise_race(vm::EfunArgs&, array_t* inputs) {
  return promise_combinator_efun(inputs, PROMISE_COMB_RACE);
}
#endif

#ifdef F_PROMISE_ALL_SETTLED
vm::Value f_promise_all_settled(vm::EfunArgs&, array_t* inputs) {
  return promise_combinator_efun(inputs, PROMISE_COMB_ALL_SETTLED);
}
#endif

#ifdef F_PROMISE_CANCEL
vm::Value f_promise_cancel(vm::EfunArgs&, promise_t* p) {
  /* validated (and possibly error()ed) before anything is returned */
  int const armed = promise_request_cancel(p);
  return armed;
}
#endif

#ifdef F_PROMISEP
vm::Value f_promisep(vm::EfunArgs&, vm::Arg value) {
  /* the argument is `mixed`, so unlike the rest of this file the tag has
   * NOT been validated for us -- that is the question being asked */
  return value.is(T_PROMISE) ? 1 : 0;
}
#endif

#ifdef F_ASYNC_YIELD
vm::Value f_async_yield(vm::EfunArgs&) {
  /* No arguments: return the yield promise. */
  return vm::Value::refed(promise_async_yield());
}
#endif

#ifdef F_ASYNC_INFO
vm::Value f_async_info(vm::EfunArgs&, LPC_INT stats) {
  /* async_info()  -> one mapping per suspended coroutine (the original form)
   * async_info(1) -> scheduler counters, including the drain's yield count,
   *                  which is the only portable way to observe that the
   *                  drain re-posted itself rather than running on. */
  int const want_stats = stats != 0;
  if (want_stats) {
    return vm::Value::refed(build_async_scheduler_info());
  }
  return vm::Value::refed(build_async_info());
}
#endif
