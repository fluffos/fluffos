/* lpc_frame.h -- the evaluation-stack API for efuns and driver call sites.
 *
 * Two small types, one job each:
 *
 *   LPCFrame  The window over one efun call's arguments. The dispatchers
 *             (eval_instruction's F_EFUN* and call_function_pointer's
 *             FP_EFUN) build it over the slots they just type-checked and
 *             pass it in, so an efun never reads its arity from a global
 *             that nested LPC can overwrite (AGENTS.md section 13.16).
 *
 *   LPCCall   The arguments for one call from C++ into LPC: an apply, a
 *             master apply, a function pointer or an efun callback. The
 *             call consumes what was pushed, so the argument count is never
 *             written a second time by hand.
 *
 * Package code cannot see `sp` (it is declared only in vm_stack.h). These
 * two types, plus the push_*() helpers in interpret.h for code that builds
 * values directly on the stack, are its whole view of the stack.
 *
 * Reference ownership is spelled out for every refcounted pointer:
 * *_refed() takes over the reference the caller already holds (a freshly
 * allocated value), *_addref() adds one (a borrowed pointer). There is
 * deliberately no bare ret(array_t*) or push(array_t*): which one was meant
 * is the whole question, and the short spelling used to be the wrong one
 * for fresh results.
 */

#ifndef LPC_FRAME_H
#define LPC_FRAME_H

#include <exception>
#include <type_traits>

#include "vm/internal/base/svalue.h"

struct function_to_call_t;
struct object_t;
struct array_t;
struct mapping_t;
struct buffer_t;
struct funptr_t;
struct promise_t;

/* The window over an efun's arguments, argument 0 first.
 *
 * Contract: when the efun returns normally it has replaced its window with
 * exactly one value, or with nothing if the efun is void. The usual way is a
 * single ret*() call. An efun that edits its one remaining argument in place
 * (pop_back() the others, then write through top() or argp()) needs no call:
 * that argument already is its result. DEBUG builds check the stack depth
 * after every efun in both dispatchers.
 *
 * Every ret*() builds its result before it releases the window, so the
 * result may come from, or point into, an argument: f.ret_copy(f.argp(1))
 * and f.ret_str_copy(f.string(0)) are safe. After ret*() the window is the
 * single result slot (empty after ret_void()).
 *
 * On an error() unwind nothing here touches the stack; restore_context()
 * owns it.
 */
class LPCFrame {
 public:
  /* Window over the top num_arg slots. The dispatchers build one per efun
   * call; VM code that calls an efun body directly may build one too. */
  static LPCFrame args(int num_arg);

  LPCFrame(const LPCFrame&) = delete;
  LPCFrame& operator=(const LPCFrame&) = delete;

  int argc() const { return argc_; }

  svalue_t& operator[](int i) { return base_[checked(i)]; }
  const svalue_t& operator[](int i) const { return base_[checked(i)]; }
  svalue_t* argp(int i) { return base_ + checked(i); }
  svalue_t& top() { return base_[checked(argc_ - 1)]; }

  int type(int i) const { return base_[checked(i)].type; }
  int subtype(int i) const { return base_[checked(i)].subtype; }
  LPC_INT number(int i) const { return base_[checked(i)].u.number; }
  LPC_FLOAT real(int i) const { return base_[checked(i)].u.real; }
  /* An int-or-float argument as a float. */
  LPC_FLOAT as_real(int i) const {
    const svalue_t& v = base_[checked(i)];
    return v.type == T_NUMBER ? static_cast<LPC_FLOAT>(v.u.number) : v.u.real;
  }
  const char* string(int i) const { return base_[checked(i)].u.string; }
  int string_len(int i) const { return SVALUE_STRLEN(&base_[checked(i)]); }
  array_t* array(int i) const { return base_[checked(i)].u.arr; }
  mapping_t* mapping(int i) const { return base_[checked(i)].u.map; }
  buffer_t* buffer(int i) const { return base_[checked(i)].u.buf; }
  object_t* object(int i) const { return base_[checked(i)].u.ob; }
  funptr_t* funptr(int i) const { return base_[checked(i)].u.fp; }
  promise_t* promise(int i) const { return base_[checked(i)].u.prom; }

  /* Move argument i out, leaving 0 in its slot; the caller owns the result.
   * For values a helper takes over (call_out arguments, stored callbacks). */
  svalue_t steal(int i) {
    svalue_t v = base_[checked(i)];
    base_[i] = const0;
    return v;
  }

  /* Free the last n arguments; the window shrinks from the top. */
  void pop_back(int n = 1);

  /* A helper (apply(), merge_arg_lists(), clone_object(), ...) already
   * consumed or added trailing slots: the window is now the top num_arg. */
  void rebase(int num_arg);

  /* Builds the callback descriptor for the efun's trailing
   * "fun, [ob,] extra args..." arguments, starting at argument narg. */
  void process_callback(int narg, function_to_call_t* ftc, int opcode);

  /* ---- results: each replaces the whole window ---- */

  template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
  void ret(T n) {
    ret_int(static_cast<LPC_INT>(n));
  }
  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
  void ret(T n) {
    ret_real(static_cast<LPC_FLOAT>(n));
  }
  void ret_int(LPC_INT n);
  void ret_real(LPC_FLOAT n);
  void ret_undefined();
  void ret_void();

  /* Takes over v's references. */
  void ret_owned(svalue_t v);
  /* Adds references to *v, which may be one of the arguments. */
  void ret_copy(const svalue_t* v);
  /* Adds a reference; returns 0 for a null or (being) destructed object. */
  void ret(object_t* ob);

  void ret_refed(array_t* v);
  void ret_addref(array_t* v);
  void ret_refed(mapping_t* m);
  void ret_addref(mapping_t* m);
  void ret_refed(buffer_t* b);
  void ret_addref(buffer_t* b);
  void ret_refed(promise_t* p);

  void ret_str_malloced(const char* p); /* takes over a malloced string */
  void ret_str_shared(const char* p);   /* adds a reference to a shared string */
  void ret_str_constant(const char* p); /* static storage; never freed */
  void ret_str_copy(const char* p);     /* copies p, which may be an argument */

  /* A helper pushed the result just above the window (outbuf_push(),
   * implode_array(), ...): keep that value, release the window under it. */
  void ret_pushed();

 private:
  LPCFrame(svalue_t* base, int argc) : base_(base), argc_(argc) {}

  int checked(int i) const {
    if (i < 0 || i >= argc_) {
      bad_index(i);
    }
    return i;
  }
  [[noreturn]] void bad_index(int i) const;
  void finish(svalue_t v);

  svalue_t* base_;
  int argc_;
};

/* Arguments for exactly one call from C++ into LPC.
 *
 * Push the arguments, then make one of the calls below. The call consumes
 * them and returns the callee's result, which lives in the driver's shared
 * return slot: copy it out before making another call. The safe_*() forms
 * catch an error() in the callee and return null instead.
 *
 * If the code returns early without calling, the destructor pops what was
 * pushed. During an error() unwind it does nothing: restore_context() owns
 * the stack then.
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
  svalue_t& arg(int i);

  template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
  void push(T n) {
    push_int(static_cast<LPC_INT>(n));
  }
  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
  void push(T n) {
    push_real(static_cast<LPC_FLOAT>(n));
  }
  void push_int(LPC_INT n);
  void push_real(LPC_FLOAT n);
  void push_undefined();
  /* Takes over v's references. */
  void push_owned(svalue_t v);
  /* Adds references to *v. */
  void push(const svalue_t* v);
  /* Adds a reference; pushes 0 for a null or (being) destructed object. */
  void push(object_t* ob);
  void push_refed(array_t* v);
  void push_addref(array_t* v);
  void push_refed(mapping_t* m);
  void push_addref(mapping_t* m);
  void push_refed(buffer_t* b);
  void push_addref(buffer_t* b);
  void push_refed(promise_t* p);
  void push_str_malloced(const char* p);
  void push_str_shared(const char* p);
  void push_str_constant(const char* p);
  void push_str_copy(const char* p);

  svalue_t* apply(const char* fun, object_t* ob, int origin);
  svalue_t* safe_apply(const char* fun, object_t* ob, int origin);
  svalue_t* apply_master(int fun);
  svalue_t* safe_apply_master(int fun);
  svalue_t* call(funptr_t* fp);
  svalue_t* safe_call(funptr_t* fp);
  svalue_t* call(function_to_call_t* ftc);
  svalue_t* safe_call(function_to_call_t* ftc);

  /* Pop the pushed values now: the call is not going to happen, or the
   * values were only scratch space kept on the stack for error cleanup. */
  void discard();

 private:
  int take();
  void note_push();

  svalue_t* base_;
  int argc_ = 0;
  bool consumed_ = false;
  int uncaught_at_entry_;
};

/* An error-handler slot pushed on the stack: if an error() unwinds through
 * it, the unwind's stack sweep calls fn (the classic T_ERROR_HANDLER
 * cleanup for C state that LPC can interrupt). On the normal path it must
 * be removed while it is still the stack top -- run() calls fn, dismiss()
 * does not; the destructor runs it if neither was called. Keep its scope
 * tight: a ret*() below an unremoved slot would release the wrong slots.
 */
class StackErrorHandler {
 public:
  explicit StackErrorHandler(void (*fn)());
  ~StackErrorHandler();
  StackErrorHandler(const StackErrorHandler&) = delete;
  StackErrorHandler& operator=(const StackErrorHandler&) = delete;

  void run();
  void dismiss();

 private:
  svalue_t* slot_;
  bool removed_ = false;
  int uncaught_at_entry_;
};

#endif /* LPC_FRAME_H */
