#include "base/std.h"

#include "vm/internal/base/efun_args.h"

#include "applies_table.autogen.h"
#include "compiler/internal/lexer.h"
#include "vm/internal/apply.h"
#include "vm/internal/base/array.h"
#include "vm/internal/base/buffer.h"
#include "vm/internal/base/interpret.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/base/mapping.h"
#include "vm/internal/base/object.h"
#include "vm/internal/base/promise.h"
#include "vm/internal/base/vm_stack.h"
#include "vm/internal/master.h"

namespace {

#if defined(__GNUC__) || defined(__clang__)
#define EFUN_ARGS_COLD __attribute__((noinline, cold))
#else
#define EFUN_ARGS_COLD
#endif

/* Out of line: freeing through the value's address here keeps that address
 * (and a stack-protector canary) out of every push. */
[[noreturn]] EFUN_ARGS_COLD void push_overflow_releasing(svalue_t v) {
  free_svalue(&v, "push_owned_value");
  too_deep_error = 1;
  error("stack overflow");
}

/* Push a value that already owns its references. On overflow the value is
 * released first, so nothing leaks past the error(). */
void push_owned_value(svalue_t v) {
  if (sp + 1 >= end_of_stack) {
    push_overflow_releasing(v);
  }
  *++sp = v;
}

/* The evaluation stack no longer matches what the API recorded: a driver
 * bug. DEBUG builds stop on the spot. Elsewhere, error() is a sound
 * recovery: its unwind resets the stack to the catching frame's mark. */
[[noreturn]] void stack_desync(const char* what) {
#ifdef DEBUG
  fatal("Driver bug: %s\n", what);
#else
  error("Driver bug: %s\n", what);
#endif
}

}  // namespace

void efun_stack_mismatch(const char* efun, int num_arg, ptrdiff_t off) {
  char what[200];
  snprintf(what, sizeof what, "the stack is %td slot(s) off after efun %s() (%d argument(s)).",
           off, efun, num_arg);
  stack_desync(what);
}

/* ------------------------------------------------------------------- Value */

#ifdef DEBUGMALLOC_EXTENSIONS
namespace {
Value* g_live_values = nullptr; /* main thread only, like every Value */
}  // namespace

void Value::track() noexcept {
  prev_ = nullptr;
  next_ = g_live_values;
  if (next_) {
    next_->prev_ = this;
  }
  g_live_values = this;
}

void Value::untrack() noexcept {
  if (prev_) {
    prev_->next_ = next_;
  } else {
    g_live_values = next_;
  }
  if (next_) {
    next_->prev_ = prev_;
  }
}

void mark_live_values() {
  for (Value* v = g_live_values; v; v = v->next_) {
    mark_svalue(&v->v_);
  }
}
#endif

Value Value::undefined() { return Value(const0u); }

Value Value::copy(const svalue_t* v) {
  svalue_t copy;
  assign_svalue_no_free(&copy, const_cast<svalue_t*>(v));
  return Value(copy);
}

Value Value::object(object_t* ob) {
  if (!ob || (ob->flags & (O_DESTRUCTED | O_BEING_DESTRUCTED))) {
    return undefined();
  }
  add_ref(ob, "Value::object");
  return Value(pointer_svalue(T_OBJECT, ob));
}

Value Value::addref(array_t* p) {
  p->ref++;
  return refed(p);
}
Value Value::addref(mapping_t* p) {
  p->ref++;
  return refed(p);
}
Value Value::addref(buffer_t* p) {
  p->ref++;
  return refed(p);
}
Value Value::addref(funptr_t* p) {
  p->hdr.ref++;
  return refed(p);
}
Value Value::addref(promise_t* p) {
  p->ref++;
  return refed(p);
}

Value Value::str_shared(const char* p) {
  ref_string(p);
  return Value(string_svalue(p, STRING_SHARED));
}
Value Value::str_make_shared(const char* p) {
  return Value(string_svalue(make_shared_string(p), STRING_SHARED));
}
Value Value::str_copy(const char* p) {
  return Value(string_svalue(string_copy(p, "Value::str_copy"), STRING_MALLOC));
}
Value Value::str_copy(std::string_view s) {
  char* p = new_string(s.size(), "Value::str_copy");
  memcpy(p, s.data(), s.size());
  p[s.size()] = '\0';
  return Value(string_svalue(p, STRING_MALLOC));
}

/* --------------------------------------------------------------------- Arg */

void Arg::wrong_type(uint32_t expected) const {
  error("Driver bug: argument read as %s but holds %s.\n", type_name(expected),
        type_name(sv_->type));
}

/* ---------------------------------------------------------------- EfunArgs */

void EfunArgs::bad_index(int i, int argc, int efun) {
  error("Driver bug: %s() read argument %d of %d.\n", query_instr_name(efun), i, argc);
}

void EfunArgs::bad_type(svalue_t* arg, int i, uint32_t types, int efun) {
  bad_argument(arg, static_cast<int>(types), i + 1, efun);
}

void EfunArgs::check_top(const char* op) const {
  if (sp != base_ + argc_ - 1) {
    not_top(efun_, op);
  }
}

void EfunArgs::stack_overflow_releasing(svalue_t result) {
  free_svalue(&result, "efun result");
  too_deep_error = 1;
  error("stack overflow");
}

void EfunArgs::not_top_releasing(svalue_t result, int efun) {
  free_svalue(&result, "efun result");
  not_top(efun, "return");
}

void EfunArgs::not_top(int efun, const char* op) {
  char what[160];
  snprintf(what, sizeof what, "%s() %s: its arguments are not the stack top.",
           query_instr_name(efun), op);
  stack_desync(what);
}

void EfunArgs::forget(int i) {
  svalue_t* slot = base_ + checked(i);
  slot->type = T_NUMBER;
  slot->subtype = 0;
  slot->u.number = 0;
}

void EfunArgs::pop_back(int n) {
  if (n < 0 || n > argc_) {
    bad_index(argc_ - n, argc_, efun_);
  }
  check_top("pop_back");
  pop_n_elems(n);
  argc_ -= n;
}

void EfunArgs::resync() {
  if (sp < base_ - 1) {
    char what[160];
    snprintf(what, sizeof what, "%s(): a helper popped below its arguments.",
             query_instr_name(efun_));
    stack_desync(what);
  }
  argc_ = static_cast<int>(sp - base_ + 1);
}

Value EfunArgs::take_pushed() {
  if (sp != base_ + argc_) {
    char what[160];
    snprintf(what, sizeof what, "%s(): expected one value pushed above its arguments.",
             query_instr_name(efun_));
    stack_desync(what);
  }
  svalue_t v = *sp--;
  return Value::owned(v);
}

void EfunArgs::process_callback(int narg, function_to_call_t* ftc) {
  /* process_efun_callback() locates the arguments from the stack top */
  check_top("process_callback");
  process_efun_callback(narg, ftc, efun_, argc_);
}

/* ----------------------------------------------------------------- LPCCall */

LPCCall::LPCCall() : base_(sp + 1) {}

LPCCall::~LPCCall() {
  if (consumed_ || argc_ == 0) {
    return;
  }
  consumed_ = true;
  if (sp != base_ + argc_ - 1) {
    /* Pushes never handed to a call, no longer on top: an error() is
     * unwinding past them, and restore_context() pops them with the rest of
     * the frame. (Popping them while they ARE on top is right either way:
     * the catching context's mark is below them.) */
    if (std::uncaught_exceptions() > 0) {
      return;
    }
    /* A destructor must not throw. Leave the slots alone: the efun's or the
     * dispatcher's own stack check reports the desync where it can error(). */
#ifdef DEBUG
    fatal("Driver bug: LPCCall: the call's arguments are not the stack top at scope exit.\n");
#else
    debug_message("Driver bug: LPCCall: the call's arguments are not the stack top at scope exit.\n");
    return;
#endif
  }
  pop_n_elems(argc_);
}

Arg LPCCall::arg(int i) const {
  if (i < 0 || i >= argc_ || consumed_) {
    error("Driver bug: LPCCall argument %d of %d.\n", i, argc_);
  }
  return Arg(base_ + i);
}

void LPCCall::push(Value v) {
  if (consumed_ || sp != base_ + argc_ - 1) {
    stack_desync("LPCCall::push: the call's arguments are not the stack top.");
  }
  push_owned_value(v.release());
  argc_++;
}

/* Hand the arguments to a call. They belong to the callee from here on,
 * including if it error()s, so mark them consumed first. */
int LPCCall::consume() {
  if (consumed_) {
    stack_desync("LPCCall used for a second call.");
  }
  if (sp != base_ + argc_ - 1) {
    stack_desync("LPCCall: the call's arguments are not the stack top.");
  }
  consumed_ = true;
  return argc_;
}

void LPCCall::discard() {
  if (consumed_) {
    return;
  }
  consumed_ = true;
  if (argc_ == 0) {
    return; /* nothing pushed: the stack may legitimately have moved */
  }
  if (sp != base_ + argc_ - 1) {
    stack_desync("LPCCall::discard: the call's arguments are not the stack top.");
  }
  pop_n_elems(argc_);
}

namespace {
/* Every call returns through the driver's shared slot (apply_ret_value).
 * Move the result out of it: the caller gets exactly what the callee
 * returned (a copy would read a destructed object as 0), and it survives
 * the next call. */
CallResult result_of(svalue_t* r) {
  if (!r) {
    return CallResult(CallResult::kNotFound);
  }
  return CallResult(Value::take(r));
}

/* ::call_direct() leaves the result on the stack top. It is moved into a
 * slot of its own for result_of(), which empties it again at once (so the
 * slot never holds a reference between calls). */
svalue_t direct_result_slot;
svalue_t* call_direct_result(object_t* ob, int runtime_index, int origin, int n) {
  ::call_direct(ob, runtime_index, origin, n);
  direct_result_slot = *sp--;
  return &direct_result_slot;
}
}  // namespace

/* Run call(argc) so that an error() in it ends here as kError: the same
 * context dance as safe_apply(), with the unwind mark below the arguments
 * because the callee owns them once the call starts (issue #1014). call
 * returns what ::apply() does, so the one CallResult is built at the end. */
template <typename Call>
CallResult LPCCall::guarded(Call&& call) {
  int n = consume();
  error_context_t econ;
  save_context(&econ);
  econ.save_sp = base_ - 1;
  svalue_t* r;
  try {
    r = call(n);
  } catch (const char*) {
    restore_context(&econ);
    pop_context(&econ);
    return CallResult(CallResult::kError);
  }
  pop_context(&econ);
  return result_of(r);
}

CallResult LPCCall::apply(const char* fun, object_t* ob, int origin) {
  return result_of(::apply(fun, ob, consume(), origin));
}

CallResult LPCCall::safe_apply(const char* fun, object_t* ob, int origin) {
  /* As ::safe_apply(): a destructed target is not called at all -- no lazy
   * reset, no apply-miss hooks (apply_low() would run both first). */
  if (ob->flags & O_DESTRUCTED) {
    discard();
    return CallResult(CallResult::kNotFound);
  }
  return guarded([&](int n) { return ::apply(fun, ob, n, origin); });
}

CallResult LPCCall::apply_master(int fun) {
  svalue_t* r = apply_master_ob(fun, consume());
  if (r == reinterpret_cast<svalue_t*>(-1)) {
    return CallResult(CallResult::kNoMaster);
  }
  return result_of(r);
}

CallResult LPCCall::safe_apply_master(int fun) {
  if (!master_ob) {
    discard();
    return CallResult(CallResult::kNoMaster);
  }
  return safe_apply(applies_table[fun], master_ob, ORIGIN_DRIVER);
}

CallResult LPCCall::call(funptr_t* fp) { return result_of(call_function_pointer(fp, consume())); }

CallResult LPCCall::safe_call(funptr_t* fp) {
  return guarded([&](int n) { return call_function_pointer(fp, n); });
}

CallResult LPCCall::call(function_to_call_t* ftc) {
  return result_of(call_efun_callback(ftc, consume()));
}

CallResult LPCCall::safe_call(function_to_call_t* ftc) {
  return guarded([&](int n) { return call_efun_callback(ftc, n); });
}

CallResult LPCCall::call_direct(object_t* ob, int runtime_index, int origin) {
  return result_of(call_direct_result(ob, runtime_index, origin, consume()));
}

CallResult LPCCall::safe_call_direct(object_t* ob, int runtime_index, int origin) {
  return guarded([&](int n) { return call_direct_result(ob, runtime_index, origin, n); });
}
