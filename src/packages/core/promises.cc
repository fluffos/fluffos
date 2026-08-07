#include "base/package_api.h"

#include "packages/core/call_out.h"

/*
 * Promise efuns (issue #1319 phase 1). The T_PROMISE machinery itself lives
 * in vm/internal/base/promise.cc; these are the LPC-visible entry points.
 *
 * Promise arguments are spec-typed `promise`, so the VM's efun dispatch has
 * already validated the top-level tag before these run (AGENTS.md section 2).
 */

#ifdef F_PROMISE_CREATE
void f_promise_create() { push_refed_promise(promise_alloc()); }
#endif

#ifdef F_PROMISE_RESOLVE
void f_promise_resolve() {
  int const num_arg = st_num_arg;
  svalue_t* parg = sp - num_arg + 1;
  promise_t* p = parg->u.prom;

  if (p->state != PROMISE_PENDING) {
    error("promise_resolve: promise is already settled.\n");
  }
  svalue_t* value = (num_arg > 1) ? sp : &const0;
  if (value->type == T_PROMISE && value->u.prom == p) {
    error("promise_resolve: cannot resolve a promise with itself.\n");
  }
  promise_resolve_with(p, value);
  pop_n_elems(num_arg);
}
#endif

#ifdef F_PROMISE_REJECT
void f_promise_reject() {
  int const num_arg = st_num_arg;
  svalue_t* parg = sp - num_arg + 1;
  promise_t* p = parg->u.prom;

  if (p->state != PROMISE_PENDING) {
    error("promise_reject: promise is already settled.\n");
  }
  /* mirror promise_resolve's guard: a promise holding itself is an
   * unbreakable cycle and makes %O printing recurse forever */
  if (num_arg > 1 && sp->type == T_PROMISE && sp->u.prom == p) {
    error("promise_reject: cannot reject a promise with itself.\n");
  }
  promise_settle(p, (num_arg > 1) ? sp : &const0, 1);
  pop_n_elems(num_arg);
}
#endif

/* shared body of promise_then() / promise_catch() */
static void promise_then_common(int num_arg, funptr_t* on_fulfilled, funptr_t* on_rejected) {
  svalue_t* parg = sp - num_arg + 1;
  promise_t* p = parg->u.prom;

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

  pop_n_elems(num_arg);
  push_refed_promise(chained);
}

#ifdef F_PROMISE_THEN
void f_promise_then() {
  int const num_arg = st_num_arg;
  svalue_t* parg = sp - num_arg + 1;
  funptr_t* on_fulfilled = (num_arg > 1) ? (parg + 1)->u.fp : nullptr;
  funptr_t* on_rejected = (num_arg > 2) ? (parg + 2)->u.fp : nullptr;
  promise_then_common(num_arg, on_fulfilled, on_rejected);
}
#endif

#ifdef F_PROMISE_CATCH
void f_promise_catch() {
  promise_then_common(2, nullptr, sp->u.fp);
}
#endif

#ifdef F_PROMISE_STATUS
void f_promise_status() {
  promise_t* p = sp->u.prom;
  int const state = p->state;
  free_svalue(sp, "f_promise_status");
  put_number(state);
}
#endif

#ifdef F_PROMISE_RESULT
void f_promise_result() {
  promise_t* p = sp->u.prom;
  if (p->state == PROMISE_PENDING) {
    error("promise_result: promise is still pending.\n");
  }
  if (p->state == PROMISE_REJECTED) {
    /* reading a rejection counts as observing it */
    p->handled = true;
  }
  svalue_t result;
  assign_svalue_no_free(&result, &p->result);
  free_svalue(sp, "f_promise_result");
  *sp = result;
}
#endif

#ifdef F_ASYNC_INFO
void f_async_info() { push_refed_array(build_async_info()); }
#endif

#ifdef F_AWAIT_CALLOUT
void f_await_callout() {
  promise_t* p = promise_for_call_out(sp->u.number);

  if (!p) {
    error("await_callout: no pending call_out with handle %" LPC_INT_FMTSTR_P ".\n", sp->u.number);
  }
  /* sp is a T_NUMBER: overwrite it directly, nothing to free */
  sp->type = T_PROMISE;
  sp->subtype = 0;
  sp->u.prom = p;
}
#endif
