/* efun_args_inline.h -- the per-call half of EfunArgs, inline.
 *
 * VM-only (it includes vm_stack.h): the dispatchers and the generated
 * thunks in efuns.autogen.cc, which run these once per efun call. Package
 * code never calls them, so it never includes this header.
 */

#ifndef VM_EFUN_ARGS_INLINE_H
#define VM_EFUN_ARGS_INLINE_H

#include "vm/internal/base/efun_args.h"
#include "vm/internal/base/vm_stack.h"

/* Every efun call runs these: keep them inline (out of line, the result is
 * spilled to memory and the thunk grows a stack-protector canary, which
 * measurably slows every trivial efun). */
#if defined(__GNUC__) || defined(__clang__)
#define EFUN_ARGS_HOT inline __attribute__((always_inline))
#else
#define EFUN_ARGS_HOT inline
#endif

void free_string_svalue(svalue_t*); /* interpret.cc */

/* Release one argument slot. Strings, the commonest refcounted argument,
 * take free_string_svalue()'s short path (as the hand-written efuns did),
 * and an array/mapping/object/... argument that something else still
 * references (a variable, almost always) only loses one reference, as the
 * hand-written free_array()-style calls did; only a last reference goes
 * through int_free_svalue(). DEBUG builds keep the generic path for its
 * double-free marking and reference journal. */
EFUN_ARGS_HOT void release_efun_arg(svalue_t* slot) {
#if !defined(DEBUG) && !defined(DEBUGMALLOC_EXTENSIONS)
  /* (free_svalue()'s own mask: numbers and floats own nothing) */
  if (!(slot->type &
        (T_STRING | T_REFED | T_ERROR_HANDLER | T_LVALUE_CODEPOINT | T_LVALUE_RANGE))) {
    return;
  }
  if (slot->type == T_STRING) {
    free_string_svalue(slot);
    return;
  }
  if ((slot->type & T_REFED) && !(slot->type & T_FREED) && slot->u.refed->ref > 1) {
    slot->u.refed->ref--;
    return;
  }
  int_free_svalue(slot);
#else
  free_svalue(slot, "efun argument");
#endif
}

EFUN_ARGS_HOT EfunArgs EfunArgs::over_top(int num_arg, int efun) {
  return EfunArgs(sp - num_arg + 1, num_arg, efun);
}

EFUN_ARGS_HOT void EfunArgs::finish(svalue_t result) {
  /* (the error paths take `result` by value: its address is never taken
   * here, so it stays in registers) */
  if (sp != base_ + argc_ - 1) {
    not_top_releasing(result, efun_);
  }
  if (argc_ > 0) {
    /* the trailing arguments are popped; argument 0's slot takes the
     * result in place (no push, no overflow check). Releasing a value runs
     * no LPC and never reads sp, so sp is stored once, at the end. */
    for (svalue_t* slot = sp; slot > base_; slot--) {
      release_efun_arg(slot);
    }
    release_efun_arg(base_);
    *base_ = result;
    sp = base_;
  } else {
    if (sp + 1 >= end_of_stack) {
      stack_overflow_releasing(result);
    }
    *++sp = result;
  }
  argc_ = 0;
  base_ = sp + 1;
}

EFUN_ARGS_HOT void EfunArgs::finish_void() {
  if (sp != base_ + argc_ - 1) {
    not_top(efun_, "return");
  }
  for (svalue_t* slot = sp; slot >= base_; slot--) {
    release_efun_arg(slot);
  }
  sp = base_ - 1;
  argc_ = 0;
  base_ = sp + 1;
}

#endif /* VM_EFUN_ARGS_INLINE_H */
