/* vm_stack.h -- evaluation-stack registers for VM-internal code only.
 *
 * Packages must NOT include this header. Package efuns use LPCFrame
 * (lpc_frame.h) exclusively -- not bare `sp`, `vm_sp()`, or put_/pop_
 * stack macros.
 *
 * Allowed consumers: interpret.cc, ops (bytecode operators), apply,
 * promise, hooks, simulate, lpc_frame.cc, and other vm/internal units.
 */

#ifndef VM_STACK_H
#define VM_STACK_H

#include "vm/internal/base/svalue.h"

struct control_stack_t;

/* Canonical stack-top pointer. */
extern svalue_t* lpc_eval_sp;
#ifndef sp
#define sp lpc_eval_sp
#endif

/* Transitional alias for VM-internal call sites still written as vm_sp().
 * Do not expose this from package-visible headers; do not add new uses in
 * package efuns (use LPCFrame). */
#ifndef vm_sp
#define vm_sp() lpc_eval_sp
#endif

extern svalue_t* fp;
extern char* pc;
extern svalue_t* start_of_stack;
extern svalue_t* end_of_stack;
extern control_stack_t* control_stack;
extern control_stack_t* control_stack_last;
extern control_stack_t* csp;

#define CHECK_STACK_OVERFLOW(x) \
  if (lpc_eval_sp + (x) >= end_of_stack) SAFE(too_deep_error = 1; error("stack overflow");)
#define STACK_INC SAFE(CHECK_STACK_OVERFLOW(1); lpc_eval_sp++;)

#define push_svalue(x) SAFE(STACK_INC; assign_svalue_no_free(lpc_eval_sp, x);)
#define put_number(x) \
  SAFE(lpc_eval_sp->type = T_NUMBER; lpc_eval_sp->subtype = 0; lpc_eval_sp->u.number = (x);)
#define put_buffer(x) SAFE(lpc_eval_sp->type = T_BUFFER; lpc_eval_sp->u.buf = (x);)
#define put_undested_object(x) SAFE(lpc_eval_sp->type = T_OBJECT; lpc_eval_sp->u.ob = (x);)
#define put_object(x)                                                                   \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED)* lpc_eval_sp = const0u; else put_undested_object(x);)
#define put_unrefed_undested_object(x, y) \
  SAFE(lpc_eval_sp->type = T_OBJECT; lpc_eval_sp->u.ob = (x); add_ref((x), y);)
#define put_unrefed_object(x, y)                                              \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED)* lpc_eval_sp = const0u; \
       else put_unrefed_undested_object(x, y);)
/* see comments on push_constant_string */
#define put_constant_string(x)                                                      \
  SAFE(lpc_eval_sp->type = T_STRING; lpc_eval_sp->subtype = STRING_SHARED; \
       lpc_eval_sp->u.string = make_shared_string(x);)
#define put_malloced_string(x) \
  SAFE(lpc_eval_sp->type = T_STRING; lpc_eval_sp->subtype = STRING_MALLOC; \
       lpc_eval_sp->u.string = (x);)
#define put_array(x) SAFE(lpc_eval_sp->type = T_ARRAY; lpc_eval_sp->u.arr = (x);)
#define put_shared_string(x) \
  SAFE(lpc_eval_sp->type = T_STRING; lpc_eval_sp->subtype = STRING_SHARED; \
       lpc_eval_sp->u.string = (x);)

/* Owns a popped (or stolen) stack lvalue. error() unwinds via a C++
 * exception (do_catch / safe_apply), so the destructor releases the box
 * on that path too -- a bare `svalue_t lvslot = *sp--` leaked every
 * T_LVALUE_CODEPOINT / T_LVALUE_RANGE that hit error() before the
 * matching free_svalue (issue #1358). VM/ops only; package efuns use LPCFrame.
 */
class PoppedLvalue {
 public:
  enum Mode { Pop, Steal };

  explicit PoppedLvalue(Mode mode = Pop) {
    slot_ = *lpc_eval_sp;
    if (mode == Pop) {
      lpc_eval_sp--;
    } else {
      /* Leave the stack slot but drop its claim on the box so unwind
       * and this destructor cannot both delete it. */
      lpc_eval_sp->type = T_NUMBER;
      lpc_eval_sp->subtype = 0;
      lpc_eval_sp->u.number = 0;
    }
  }
  ~PoppedLvalue() { free_svalue(&slot_, "PoppedLvalue"); }
  PoppedLvalue(const PoppedLvalue&) = delete;
  PoppedLvalue& operator=(const PoppedLvalue&) = delete;
  svalue_t* target() { return lvalue_target(&slot_); }

 private:
  svalue_t slot_;
};

#endif /* VM_STACK_H */
