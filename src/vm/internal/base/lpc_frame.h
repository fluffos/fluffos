/* lpc_frame.h -- value-stack window protocol for driver / efun code.
 *
 * Owns a contiguous range of evaluation-stack slots. Does not replace
 * control_stack_t (call/control) or error_context_t (unwind marks); it
 * is the missing API for "these slots are my args / temps / call args".
 *
 * Modes:
 *   Args    -- efun entry: window over already-pushed arguments
 *   Builder -- push args then apply / safe_apply
 *   Mark    -- record a stack watermark (assert / discard_to)
 *
 * Packages / efuns should use LPCFrame only -- not vm_sp(), bare sp, or
 * direct put_/pop_ stack macros. Opcodes and VM internals may still use
 * vm_stack.h.
 *
 * On error() unwind the destructor does NOT touch the stack (restore_context
 * owns recovery). On the normal path Args/Builder must be finalized
 * (ret_*, done, apply, discard, ...).
 *
 * Method names intentionally avoid the push_* / put_* macros in interpret.h.
 */

#ifndef LPC_FRAME_H
#define LPC_FRAME_H

#include <exception>

#include "vm/internal/base/svalue.h"

struct function_to_call_t;
struct object_t;
struct array_t;
struct mapping_t;
struct buffer_t;
struct funptr_t;
struct promise_t;

class LPCFrame {
 public:
  enum class Mode { Args, Builder, Mark };

  /* Efun entry: [sp - num_arg + 1 .. sp]. */
  static LPCFrame args(int num_arg);

  /* Start empty; push grows the window. */
  static LPCFrame builder();

  /* Record current top; discard_to() pops back here. */
  static LPCFrame mark();

  ~LPCFrame();

  LPCFrame(LPCFrame&& other) noexcept;
  LPCFrame& operator=(LPCFrame&&) = delete;
  LPCFrame(const LPCFrame&) = delete;
  LPCFrame& operator=(const LPCFrame&) = delete;

  Mode mode() const { return mode_; }
  int argc() const { return argc_; }
  /* Slots currently in the window (inclusive). */
  int size() const;
  svalue_t* base() const { return base_; }
  bool finalized() const { return finalized_; }

  svalue_t& operator[](int i);
  const svalue_t& operator[](int i) const;
  /* 0 = top (last arg), 1 = one below, ... */
  svalue_t& from_top(int n);
  const svalue_t& from_top(int n) const;
  svalue_t& top();
  const svalue_t& top() const;
  svalue_t* argp(int i);

  int type(int i) const;
  int subtype(int i) const;
  LPC_INT number(int i) const;
  LPC_FLOAT real(int i) const;
  /* Number-or-real as float (math efuns). */
  LPC_FLOAT as_real(int i) const;
  const char* string(int i) const;
  /* String length via SVALUE_STRLEN on the slot. */
  int string_len(int i) const;
  array_t* array(int i) const;
  mapping_t* mapping(int i) const;
  buffer_t* buffer(int i) const;
  object_t* object(int i) const;
  funptr_t* funptr(int i) const;
  promise_t* promise(int i) const;

  /* Move slot i out (leave const0); caller owns the returned svalue. */
  svalue_t steal(int i);

  /* Builder push (also usable to grow a Mark). Updates argc_.
   * Named push() to avoid interpret.h push_* macros. */
  void push(LPC_INT n);
  void push(LPC_FLOAT n);
  void push(svalue_t* v);
  void push_undefined();
  void push_str_malloced(const char* p);
  void push_str_shared(const char* p);
  void push_str_constant(const char* p);
  void push_str_copy(const char* p); /* string_copy + malloced */
  void push(object_t* ob);
  void push(array_t* v);
  void push_refed(array_t* v);
  void push(mapping_t* m);
  void push_refed(mapping_t* m);
  void push(buffer_t* b);
  void push_refed(buffer_t* b);

  /* Args finishers: replace the window with a single result (or nothing). */
  void ret(LPC_INT n);
  void ret(LPC_FLOAT n);
  void ret_void();
  void ret(svalue_t v); /* moves; frees previous window */
  void ret_copy(svalue_t* v);
  void ret(array_t* v);
  void ret_refed(array_t* v);
  void ret(mapping_t* m);
  void ret_refed(mapping_t* m);
  void ret_str_malloced(const char* p);
  void ret_str_shared(const char* p);
  void ret_str_constant(const char* p);
  void ret_str_copy(const char* p);
  void ret(object_t* ob);
  void ret(buffer_t* b);
  void ret_refed(buffer_t* b);
  void ret_undefined();

  /* Pop argc-1 from the back; leave/set the front slot as an int result.
   * Common sockets/db pattern: pop_n_elems(argc-1); sp->u.number = n. */
  void ret_front_number(LPC_INT n);

  /* Overwrite slot i in place (frees previous contents). */
  void set_number(int i, LPC_INT n);
  void set_real(int i, LPC_FLOAT n);
  void set_undefined(int i);

  /* Free and overwrite the top (put_* family when window size is unchanged). */
  void set_top_number(LPC_INT n);
  void set_top_real(LPC_FLOAT n);
  void set_top_array(array_t* v);
  void set_top_refed_array(array_t* v);
  void set_top_str_malloced(const char* p);
  void set_top_str_constant(const char* p);
  void set_top_undefined();

  /* Stack already holds the correct return (e.g. unary in-place mutate). */
  void done();

  /* Drop the first n args (free them); window shrinks from the front. */
  void drop_front(int n);

  /* Pop n slots from the back (free them); window shrinks. */
  void pop_back(int n = 1);

  /* Pop everything back to (and including) the mark; Mark/Builder mode. */
  void discard_to();

  /* Callback scan over trailing fun/obj/... args (Args mode). */
  void process_callback(int narg, function_to_call_t* ftc, int opcode);

  /* Consume window as apply args. Marks finalized. */
  svalue_t* apply(const char* fun, object_t* ob, int origin);
  svalue_t* safe_apply(const char* fun, object_t* ob, int origin);

 private:
  LPCFrame() = default;

  void check_index(int i) const;
  void note_push();
  void finalize_ret_slot();

  Mode mode_ = Mode::Args;
  svalue_t* base_ = nullptr;
  int argc_ = 0;
  bool finalized_ = false;
  bool moved_from_ = false;
};

/* ---- Tier-C glue macros (fixed scalar efuns; no raw stack in the body) ---- */

#define LPC_EFUN_UNARY_REAL(name, expr)      \
  void f_##name(int num_arg) {               \
    LPCFrame f = LPCFrame::args(num_arg);    \
    f.top().u.real = (expr)(f.top().u.real); \
    f.done();                                \
  }

#endif /* LPC_FRAME_H */
