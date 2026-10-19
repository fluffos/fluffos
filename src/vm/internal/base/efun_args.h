/* efun_args.h -- the evaluation-stack API for efuns and driver call sites.
 *
 * Package code cannot see `sp` or the push_*()/pop_*() helpers (they live in
 * vm_stack.h, for VM internals and packages/ops only). These types are its
 * whole view of the stack:
 *
 *   EfunArgs    The window over one efun call's arguments. make_func
 *               generates a thunk per efun from its .spec line: the thunk
 *               checks every declared argument against its spec type,
 *               passes it to the efun as a typed parameter, and pushes the
 *               efun's return value (or nothing, for a void efun) in place
 *               of the arguments. An efun body sees the window only for what
 *               the spec cannot type: varargs, and moving arguments out.
 *
 *   LPCCall     The arguments for one call from C++ into LPC (an apply, a
 *               master apply, a function pointer or an efun callback). The
 *               call consumes what was pushed, so the count is never written
 *               a second time by hand.
 *
 *   CallResult  What such a call produced: a status and, when it succeeded,
 *               the result as an owned Value.
 *
 * Values themselves are value.h's Value, StringArg and Arg.
 */

#ifndef VM_EFUN_ARGS_H
#define VM_EFUN_ARGS_H

#include <exception>
#include <optional>
#include <utility>

#include "vm/internal/base/value.h"

struct function_to_call_t;

/* The window over an efun's arguments, argument 0 first.
 *
 * Generated thunks build typed parameters from it and own its release. An
 * efun body uses it for the arguments its spec leaves untyped (a `...`
 * tail: argc() and operator[]) and for the few things a value-returning
 * function cannot express on its own:
 *
 *   take(i)        move argument i out as an owned Value (its slot becomes 0);
 *   forget(i)      a helper already moved argument i's reference elsewhere:
 *                  zero the slot without releasing it;
 *   pop_back(n)    release the last n arguments now;
 *   resync()       a helper consumed trailing argument slots, or pushed
 *                  values above them: re-measure the window. The window's
 *                  base never moves, so no count is involved;
 *   take_pushed()  move out the one value a helper pushed above the window.
 *
 * Typed parameters borrow their slots: after pop_back() or take() of a
 * slot, a parameter that pointed into it is dangling.
 *
 * Every operation, including the thunk's return through finish(), checks
 * that the window is still the stack top, in every build: a desync is
 * reported at the efun that caused it, not wherever it surfaces. (DEBUG
 * builds also re-check the depth in both dispatchers, a backstop for the
 * generated thunks themselves.)
 * On an error() unwind nothing here touches the stack; restore_context()
 * owns it.
 */
class EfunArgs {
 public:
  /* The window over the top num_arg slots, for efun instruction `efun`
   * (used to name it in errors). The dispatchers build one per call; VM
   * code that runs an efun body directly may build one too. Defined in the
   * VM-only efun_args_inline.h. */
  static inline EfunArgs over_top(int num_arg, int efun);

  EfunArgs(const EfunArgs&) = delete;
  EfunArgs& operator=(const EfunArgs&) = delete;

  int argc() const { return argc_; }
  Arg operator[](int i) const { return Arg(base_ + checked(i)); }

  [[nodiscard]] Value take(int i) { return Value::take(base_ + checked(i)); }
  void forget(int i);
  void pop_back(int n = 1);
  void resync();
  [[nodiscard]] Value take_pushed();

  /* Builds the callback descriptor for the efun's trailing
   * "fun, [ob,] extra args..." arguments, starting at argument narg. */
  void process_callback(int narg, function_to_call_t* ftc);

  /* ---- for generated thunks (efuns.autogen.cc) only ----
   * Argument i, checked against its spec type: a mismatch is the usual
   * "Bad argument" error. The dispatchers have already checked compiled
   * calls; this is what makes a call through an efun pointer just as safe.
   * No index check: the thunk asks only for positions the dispatchers
   * guarantee (below min_arg), and goes through optional() for the rest. */
  LPC_INT int_arg(int i) const { return expect(i, T_NUMBER).u.number; }
  LPC_FLOAT real_arg(int i) const { return expect(i, T_REAL).u.real; }
  StringArg string_arg(int i) const { return StringArg(&expect(i, T_STRING)); }
  array_t* array_arg(int i) const { return expect(i, T_ARRAY).u.arr; }
  mapping_t* mapping_arg(int i) const { return expect(i, T_MAPPING).u.map; }
  buffer_t* buffer_arg(int i) const { return expect(i, T_BUFFER).u.buf; }
  object_t* object_arg(int i) const { return expect(i, T_OBJECT).u.ob; }
  funptr_t* funptr_arg(int i) const { return expect(i, T_FUNCTION).u.fp; }
  promise_t* promise_arg(int i) const { return expect(i, T_PROMISE).u.prom; }
  /* A union-typed argument; types is the mask of T_* it may hold. */
  Arg any_arg(int i, uint32_t types) const {
    if (!(base_[i].type & types)) {
      bad_type(base_ + i, i, types, efun_);
    }
    return Arg(base_ + i);
  }
  /* A `mixed` argument: every value an argument can hold is allowed. */
  Arg mixed_arg(int i) const { return Arg(base_ + i); }
  /* (the getter is a template argument, not a run-time member pointer, so
   * it inlines and the EfunArgs stays in registers) */
  template <auto Get>
  auto optional(int i) const -> std::optional<decltype((this->*Get)(i))> {
    using T = decltype((this->*Get)(i));
    return i < argc_ ? std::optional<T>((this->*Get)(i)) : std::nullopt;
  }
  std::optional<Arg> optional_any(int i, uint32_t types) const {
    return i < argc_ ? std::optional<Arg>(any_arg(i, types)) : std::nullopt;
  }
  std::optional<Arg> optional_mixed(int i) const {
    return i < argc_ ? std::optional<Arg>(mixed_arg(i)) : std::nullopt;
  }
  /* Replace the window with the efun's result (taking over its
   * references), or with nothing. Inline (in efun_args_inline.h): they run
   * once per efun call. A thunk releases the efun's Value in a statement of
   * its own before calling finish(), so it never needs memory. */
  inline void finish(svalue_t result);
  inline void finish_void();

 private:
  EfunArgs(svalue_t* base, int argc, int efun) : base_(base), argc_(argc), efun_(efun) {}

  int checked(int i) const {
    if (i < 0 || i >= argc_) {
      bad_index(i, argc_, efun_);
    }
    return i;
  }
  const svalue_t& expect(int i, uint32_t t) const {
    if (base_[i].type != t) {
      bad_type(base_ + i, i, t, efun_);
    }
    return base_[i];
  }
  void check_top(const char* op) const;
  /* The error reporters take plain values, never `this`: an EfunArgs whose
   * address is never taken stays in registers in the generated thunks. */
  [[noreturn]] static void bad_index(int i, int argc, int efun);
  [[noreturn]] static void bad_type(svalue_t* arg, int i, uint32_t types, int efun);
  [[noreturn]] static void not_top(int efun, const char* op);
  [[noreturn]] static void not_top_releasing(svalue_t result, int efun);
  [[noreturn]] static void stack_overflow_releasing(svalue_t result);

  svalue_t* base_;
  int argc_;
  int efun_;
};

/* The outcome of one call into LPC.
 *
 * ok() means the function ran and returned; its result is moved out of
 * the driver's shared return slot into an owned Value here, so it outlives
 * the next call. Otherwise status() says why there is no result, and the
 * value reads as 0:
 *
 *   kNotFound  no such function, it is not callable from here, or the
 *              object is destructed;
 *   kError     a safe_*() call caught an error() in the callee;
 *   kNoMaster  a master apply was made before any master object loaded.
 *
 * operator-> and operator* reach the value without moving it, so
 * `if (r && r->type == T_STRING)` reads naturally; get() is the value, or
 * null unless ok(), for code that hands it to a function taking an
 * svalue_t*.
 */
class CallResult {
 public:
  enum Status { kOk, kNotFound, kError, kNoMaster };

  explicit CallResult(Status status) : status_(status) {}
  explicit CallResult(Value v) : status_(kOk), value_(std::move(v)) {}

  Status status() const { return status_; }
  bool ok() const { return status_ == kOk; }
  explicit operator bool() const { return ok(); }

  svalue_t& operator*() { return *value_; }
  const svalue_t& operator*() const { return *value_; }
  svalue_t* operator->() { return value_.get(); }
  const svalue_t* operator->() const { return value_.get(); }
  svalue_t* get() { return ok() ? value_.get() : nullptr; }
  const svalue_t* get() const { return ok() ? value_.get() : nullptr; }
  /* The result, owned by the caller from here on (0 unless ok()). */
  [[nodiscard]] Value take() { return std::move(value_); }

 private:
  Status status_;
  Value value_;
};

/* Arguments for exactly one call from C++ into LPC.
 *
 * Push the arguments, then make one of the calls below. The call consumes
 * them, including when the callee error()s. The safe_*() forms catch an
 * error() in the callee and report it as CallResult::kError.
 *
 * If the code leaves the scope without calling, the destructor pops what
 * was pushed -- also during an error() unwind, as long as the pushes are
 * still the stack top; otherwise restore_context() pops them with the rest
 * of the frame. Pushes must be the stack top when they are made and when
 * the call consumes them; that is checked in every build.
 */
class LPCCall {
 public:
  LPCCall();
  ~LPCCall();
  LPCCall(const LPCCall&) = delete;
  LPCCall& operator=(const LPCCall&) = delete;

  int argc() const { return argc_; }
  /* A pushed argument, 0 first; for code that inspects or hands one over
   * (promise_settle()) before deciding which call to make. */
  Arg arg(int i) const;

  void push(Value v);

  CallResult apply(const char* fun, object_t* ob, int origin);
  CallResult safe_apply(const char* fun, object_t* ob, int origin);
  CallResult apply_master(int fun);
  CallResult safe_apply_master(int fun);
  CallResult call(funptr_t* fp);
  CallResult safe_call(funptr_t* fp);
  CallResult call(function_to_call_t* ftc);
  CallResult safe_call(function_to_call_t* ftc);
  /* Function `runtime_index` of ob's program, called directly (no lookup by
   * name, no shadows): the driver's own applies such as heart_beat. */
  CallResult call_direct(object_t* ob, int runtime_index, int origin);
  CallResult safe_call_direct(object_t* ob, int runtime_index, int origin);

  /* Pop the pushed values now: the call is not going to happen, or the
   * values were only scratch space kept on the stack for error cleanup. */
  void discard();

 private:
  int consume();
  template <typename Call>
  CallResult guarded(Call&& call);

  svalue_t* base_;
  int argc_ = 0;
  bool consumed_ = false;
};

#endif /* VM_EFUN_ARGS_H */
