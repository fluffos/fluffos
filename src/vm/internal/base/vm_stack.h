/* vm_stack.h -- evaluation-stack registers for VM-internal code only.
 *
 * Packages must NOT include this header. Package efuns use LPCFrame
 * (lpc_frame.h) -- not `sp`, which they cannot see, nor the put_/pop_
 * stack macros.
 *
 * Allowed consumers: interpret.cc, ops (bytecode operators), apply,
 * promise, hooks, simulate, lpc_frame.cc, and other vm/internal units.
 */

#ifndef VM_STACK_H
#define VM_STACK_H

#include "vm/internal/base/svalue.h"

struct control_stack_t;

/* Evaluation-stack top. Deliberately declared only here: package code
 * cannot name it, so it reaches the stack only through LPCFrame/LPCCall
 * (lpc_frame.h) and the push_*() helpers in interpret.h. */
extern svalue_t* sp;
extern svalue_t* fp;
extern char* pc;
extern svalue_t* start_of_stack;
extern svalue_t* end_of_stack;
extern control_stack_t* control_stack;
extern control_stack_t* control_stack_last;
extern control_stack_t* csp;

#define CHECK_STACK_OVERFLOW(x) \
  if (sp + (x) >= end_of_stack) SAFE(too_deep_error = 1; error("stack overflow");)
#define STACK_INC SAFE(CHECK_STACK_OVERFLOW(1); sp++;)

#define push_svalue(x) SAFE(STACK_INC; assign_svalue_no_free(sp, x);)
#define put_number(x) SAFE(sp->type = T_NUMBER; sp->subtype = 0; sp->u.number = (x);)
#define put_buffer(x) SAFE(sp->type = T_BUFFER; sp->u.buf = (x);)
#define put_undested_object(x) SAFE(sp->type = T_OBJECT; sp->u.ob = (x);)
#define put_object(x) \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED)* sp = const0u; else put_undested_object(x);)
#define put_unrefed_undested_object(x, y) \
  SAFE(sp->type = T_OBJECT; sp->u.ob = (x); add_ref((x), y);)
#define put_unrefed_object(x, y)                             \
  SAFE(if (!(x) || (x)->flags & O_DESTRUCTED)* sp = const0u; \
       else put_unrefed_undested_object(x, y);)
/* see comments on push_constant_string */
#define put_constant_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_SHARED; sp->u.string = make_shared_string(x);)
#define put_malloced_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_MALLOC; sp->u.string = (x);)
#define put_array(x) SAFE(sp->type = T_ARRAY; sp->u.arr = (x);)
#define put_shared_string(x) \
  SAFE(sp->type = T_STRING; sp->subtype = STRING_SHARED; sp->u.string = (x);)

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
    slot_ = *sp;
    if (mode == Pop) {
      sp--;
    } else {
      /* Leave the stack slot but drop its claim on the box so unwind
       * and this destructor cannot both delete it. */
      sp->type = T_NUMBER;
      sp->subtype = 0;
      sp->u.number = 0;
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
