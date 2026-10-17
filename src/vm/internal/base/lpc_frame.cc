#include "base/std.h"

#include "vm/internal/base/lpc_frame.h"

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

/* push_object()'s rule, as a value: a dead object reads as undefined. */
svalue_t object_svalue(object_t* ob) {
  if (!ob || (ob->flags & (O_DESTRUCTED | O_BEING_DESTRUCTED))) {
    return const0u;
  }
  svalue_t v;
  v.type = T_OBJECT;
  v.subtype = 0;
  v.u.ob = ob;
  add_ref(ob, "LPCFrame");
  return v;
}

svalue_t number_svalue(LPC_INT n) {
  svalue_t v;
  v.type = T_NUMBER;
  v.subtype = 0;
  v.u.number = n;
  return v;
}

svalue_t real_svalue(LPC_FLOAT n) {
  svalue_t v;
  v.type = T_REAL;
  v.subtype = 0;
  v.u.real = n;
  return v;
}

svalue_t string_svalue(const char* p, int subtype) {
  svalue_t v;
  v.type = T_STRING;
  v.subtype = subtype;
  v.u.string = p;
  return v;
}

template <typename T>
svalue_t refed_svalue(int type, T* p);
template <>
svalue_t refed_svalue(int type, array_t* p) {
  svalue_t v;
  v.type = type;
  v.subtype = 0;
  v.u.arr = p;
  return v;
}
template <>
svalue_t refed_svalue(int type, mapping_t* p) {
  svalue_t v;
  v.type = type;
  v.subtype = 0;
  v.u.map = p;
  return v;
}
template <>
svalue_t refed_svalue(int type, buffer_t* p) {
  svalue_t v;
  v.type = type;
  v.subtype = 0;
  v.u.buf = p;
  return v;
}
template <>
svalue_t refed_svalue(int type, promise_t* p) {
  svalue_t v;
  v.type = type;
  v.subtype = 0;
  v.u.prom = p;
  return v;
}

/* Push a value that already owns its references. On overflow the value is
 * released first, so nothing leaks past the error(). */
void push_owned_value(svalue_t v) {
  if (sp + 1 >= end_of_stack) {
    free_svalue(&v, "LPCFrame push");
    too_deep_error = 1;
    error("stack overflow");
  }
  *++sp = v;
}

}  // namespace

/* ---------------------------------------------------------------- LPCFrame */

LPCFrame LPCFrame::args(int num_arg) { return LPCFrame(sp - num_arg + 1, num_arg); }

void LPCFrame::bad_index(int i) const {
  error("Driver bug: efun argument %d out of range (%d argument(s)).\n", i, argc_);
}

void LPCFrame::pop_back(int n) {
  DEBUG_CHECK2(n < 0 || n > argc_, "LPCFrame::pop_back(%d) with %d argument(s).\n", n, argc_);
  DEBUG_CHECK(sp != base_ + argc_ - 1, "LPCFrame::pop_back: window is not the stack top.\n");
  pop_n_elems(n);
  argc_ -= n;
}

void LPCFrame::rebase(int num_arg) {
  DEBUG_CHECK1(num_arg < 0, "LPCFrame::rebase(%d).\n", num_arg);
  base_ = sp - num_arg + 1;
  argc_ = num_arg;
}

void LPCFrame::process_callback(int narg, function_to_call_t* ftc, int opcode) {
  /* process_efun_callback() locates the arguments from the stack top */
  DEBUG_CHECK(sp != base_ + argc_ - 1,
              "LPCFrame::process_callback: window is not the stack top.\n");
  process_efun_callback(narg, ftc, opcode, argc_);
}

/* Release the window, then push the already-built result in its place. */
void LPCFrame::finish(svalue_t v) {
  DEBUG_CHECK(sp != base_ + argc_ - 1, "LPCFrame: window is not the stack top at ret.\n");
  pop_n_elems(argc_);
  push_owned_value(v);
  base_ = sp;
  argc_ = 1;
}

void LPCFrame::ret_int(LPC_INT n) { finish(number_svalue(n)); }
void LPCFrame::ret_real(LPC_FLOAT n) { finish(real_svalue(n)); }
void LPCFrame::ret_undefined() { finish(const0u); }

void LPCFrame::ret_void() {
  DEBUG_CHECK(sp != base_ + argc_ - 1, "LPCFrame: window is not the stack top at ret_void.\n");
  pop_n_elems(argc_);
  base_ = sp + 1;
  argc_ = 0;
}

void LPCFrame::ret_owned(svalue_t v) { finish(v); }

void LPCFrame::ret_copy(const svalue_t* v) {
  svalue_t copy;
  assign_svalue_no_free(&copy, const_cast<svalue_t*>(v));
  finish(copy);
}

void LPCFrame::ret(object_t* ob) { finish(object_svalue(ob)); }

void LPCFrame::ret_refed(array_t* v) { finish(refed_svalue(T_ARRAY, v)); }
void LPCFrame::ret_addref(array_t* v) {
  v->ref++;
  finish(refed_svalue(T_ARRAY, v));
}
void LPCFrame::ret_refed(mapping_t* m) { finish(refed_svalue(T_MAPPING, m)); }
void LPCFrame::ret_addref(mapping_t* m) {
  m->ref++;
  finish(refed_svalue(T_MAPPING, m));
}
void LPCFrame::ret_refed(buffer_t* b) { finish(refed_svalue(T_BUFFER, b)); }
void LPCFrame::ret_addref(buffer_t* b) {
  b->ref++;
  finish(refed_svalue(T_BUFFER, b));
}
void LPCFrame::ret_refed(promise_t* p) { finish(refed_svalue(T_PROMISE, p)); }

void LPCFrame::ret_str_malloced(const char* p) { finish(string_svalue(p, STRING_MALLOC)); }
void LPCFrame::ret_str_shared(const char* p) {
  ref_string(p);
  finish(string_svalue(p, STRING_SHARED));
}
void LPCFrame::ret_str_constant(const char* p) { finish(string_svalue(p, STRING_CONSTANT)); }
void LPCFrame::ret_str_copy(const char* p) {
  finish(string_svalue(string_copy(p, "LPCFrame::ret_str_copy"), STRING_MALLOC));
}

void LPCFrame::ret_pushed() {
  DEBUG_CHECK(sp != base_ + argc_, "LPCFrame::ret_pushed: expected one value above the window.\n");
  svalue_t v = *sp--;
  finish(v);
}

/* ----------------------------------------------------------------- LPCCall */

LPCCall::LPCCall() : base_(sp + 1), uncaught_at_entry_(std::uncaught_exceptions()) {}

LPCCall::~LPCCall() {
  if (consumed_ || std::uncaught_exceptions() != uncaught_at_entry_) {
    return;
  }
  discard();
}

void LPCCall::note_push() {
  DEBUG_CHECK(sp != base_ + argc_, "LPCCall: something else pushed between arguments.\n");
  argc_++;
}

/* Hand the arguments to a call. They belong to the callee from here on,
 * including if it error()s, so mark them consumed first. */
int LPCCall::take() {
  DEBUG_CHECK(consumed_, "LPCCall used for a second call.\n");
  DEBUG_CHECK(sp != base_ + argc_ - 1, "LPCCall: arguments are not the stack top at call.\n");
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
  DEBUG_CHECK(sp != base_ + argc_ - 1, "LPCCall: arguments are not the stack top at discard.\n");
  pop_n_elems(argc_);
}

void LPCCall::push_int(LPC_INT n) {
  push_owned_value(number_svalue(n));
  note_push();
}
void LPCCall::push_real(LPC_FLOAT n) {
  push_owned_value(real_svalue(n));
  note_push();
}
void LPCCall::push_undefined() {
  push_owned_value(const0u);
  note_push();
}
svalue_t& LPCCall::arg(int i) {
  if (i < 0 || i >= argc_ || consumed_) {
    error("Driver bug: LPCCall argument %d out of range (%d argument(s)).\n", i, argc_);
  }
  return base_[i];
}
void LPCCall::push_owned(svalue_t v) {
  push_owned_value(v);
  note_push();
}
void LPCCall::push(const svalue_t* v) {
  svalue_t copy;
  assign_svalue_no_free(&copy, const_cast<svalue_t*>(v));
  push_owned_value(copy);
  note_push();
}
void LPCCall::push(object_t* ob) {
  push_owned_value(object_svalue(ob));
  note_push();
}
void LPCCall::push_refed(array_t* v) {
  push_owned_value(refed_svalue(T_ARRAY, v));
  note_push();
}
void LPCCall::push_addref(array_t* v) {
  v->ref++;
  push_refed(v);
}
void LPCCall::push_refed(mapping_t* m) {
  push_owned_value(refed_svalue(T_MAPPING, m));
  note_push();
}
void LPCCall::push_addref(mapping_t* m) {
  m->ref++;
  push_refed(m);
}
void LPCCall::push_refed(buffer_t* b) {
  push_owned_value(refed_svalue(T_BUFFER, b));
  note_push();
}
void LPCCall::push_addref(buffer_t* b) {
  b->ref++;
  push_refed(b);
}
void LPCCall::push_refed(promise_t* p) {
  push_owned_value(refed_svalue(T_PROMISE, p));
  note_push();
}
void LPCCall::push_str_malloced(const char* p) {
  push_owned_value(string_svalue(p, STRING_MALLOC));
  note_push();
}
void LPCCall::push_str_shared(const char* p) {
  ref_string(p);
  push_owned_value(string_svalue(p, STRING_SHARED));
  note_push();
}
void LPCCall::push_str_constant(const char* p) {
  push_owned_value(string_svalue(p, STRING_CONSTANT));
  note_push();
}
void LPCCall::push_str_copy(const char* p) {
  push_owned_value(string_svalue(string_copy(p, "LPCCall::push_str_copy"), STRING_MALLOC));
  note_push();
}

svalue_t* LPCCall::apply(const char* fun, object_t* ob, int origin) {
  return ::apply(fun, ob, take(), origin);
}
svalue_t* LPCCall::safe_apply(const char* fun, object_t* ob, int origin) {
  return ::safe_apply(fun, ob, take(), origin);
}
svalue_t* LPCCall::apply_master(int fun) { return apply_master_ob(fun, take()); }
svalue_t* LPCCall::safe_apply_master(int fun) { return safe_apply_master_ob(fun, take()); }
svalue_t* LPCCall::call(funptr_t* fp) { return call_function_pointer(fp, take()); }
svalue_t* LPCCall::safe_call(funptr_t* fp) { return safe_call_function_pointer(fp, take()); }
svalue_t* LPCCall::call(function_to_call_t* ftc) { return call_efun_callback(ftc, take()); }
svalue_t* LPCCall::safe_call(function_to_call_t* ftc) {
  return safe_call_efun_callback(ftc, take());
}

/* ------------------------------------------------------- StackErrorHandler */

StackErrorHandler::StackErrorHandler(void (*fn)())
    : uncaught_at_entry_(std::uncaught_exceptions()) {
  STACK_INC;
  sp->type = T_ERROR_HANDLER;
  sp->subtype = 0;
  sp->u.error_handler = fn;
  slot_ = sp;
}

StackErrorHandler::~StackErrorHandler() {
  if (removed_ || std::uncaught_exceptions() != uncaught_at_entry_) {
    return;
  }
  run();
}

void StackErrorHandler::run() {
  if (removed_) {
    return;
  }
  DEBUG_CHECK(sp != slot_, "StackErrorHandler: slot is not the stack top.\n");
  removed_ = true;
  pop_stack(); /* freeing a T_ERROR_HANDLER calls it */
}

void StackErrorHandler::dismiss() {
  if (removed_) {
    return;
  }
  DEBUG_CHECK(sp != slot_, "StackErrorHandler: slot is not the stack top.\n");
  removed_ = true;
  sp--;
}
