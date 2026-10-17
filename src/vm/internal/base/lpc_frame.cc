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

/* Stack macros / sp come from interpret.h + vm_stack.h. Prefer `sp` here. */

LPCFrame LPCFrame::args(int num_arg) {
  LPCFrame f;
  f.mode_ = Mode::Args;
  f.argc_ = num_arg;
  f.base_ = num_arg > 0 ? sp - num_arg + 1 : sp + 1;
  f.finalized_ = false;
  return f;
}

LPCFrame LPCFrame::builder() {
  LPCFrame f;
  f.mode_ = Mode::Builder;
  f.argc_ = 0;
  f.base_ = sp + 1;
  f.finalized_ = false;
  return f;
}

LPCFrame LPCFrame::mark() {
  LPCFrame f;
  f.mode_ = Mode::Mark;
  f.argc_ = 0;
  f.base_ = sp + 1;
  f.finalized_ = false;
  return f;
}

LPCFrame::LPCFrame(LPCFrame&& other) noexcept
    : mode_(other.mode_),
      base_(other.base_),
      argc_(other.argc_),
      finalized_(other.finalized_),
      moved_from_(false) {
  other.moved_from_ = true;
  other.finalized_ = true;
}

LPCFrame::~LPCFrame() {
  if (moved_from_ || finalized_) {
    return;
  }
  /* error() unwind: restore_context owns the stack. */
  if (std::uncaught_exceptions() > 0) {
    return;
  }
#ifdef DEBUG
  debug_message("LPCFrame destroyed without finalize (mode=%d argc=%d)\n", static_cast<int>(mode_),
                argc_);
#endif
  if (mode_ == Mode::Builder || mode_ == Mode::Mark) {
    while (sp >= base_) {
      pop_stack();
    }
  }
}

int LPCFrame::size() const {
  if (argc_ == 0 && (mode_ == Mode::Builder || mode_ == Mode::Mark) && sp < base_) {
    return 0;
  }
  if (sp < base_) {
    return 0;
  }
  return static_cast<int>(sp - base_) + 1;
}

void LPCFrame::check_index(int i) const {
  if (i < 0 || i >= argc_) {
    error("LPCFrame: argument index %d out of range (argc=%d)\n", i, argc_);
  }
}

void LPCFrame::note_push() {
  if (argc_ == 0) {
    base_ = sp;
  }
  argc_++;
}

void LPCFrame::finalize_ret_slot() {
  argc_ = 1;
  base_ = sp;
  finalized_ = true;
}

svalue_t& LPCFrame::operator[](int i) {
  check_index(i);
  return base_[i];
}

const svalue_t& LPCFrame::operator[](int i) const {
  check_index(i);
  return base_[i];
}

svalue_t& LPCFrame::from_top(int n) {
  if (n < 0 || n >= argc_) {
    error("LPCFrame: from_top(%d) out of range (argc=%d)\n", n, argc_);
  }
  return base_[argc_ - 1 - n];
}

const svalue_t& LPCFrame::from_top(int n) const {
  if (n < 0 || n >= argc_) {
    error("LPCFrame: from_top(%d) out of range (argc=%d)\n", n, argc_);
  }
  return base_[argc_ - 1 - n];
}

svalue_t& LPCFrame::top() {
  if (argc_ <= 0) {
    error("LPCFrame: top() with empty window\n");
  }
  return base_[argc_ - 1];
}

const svalue_t& LPCFrame::top() const {
  if (argc_ <= 0) {
    error("LPCFrame: top() with empty window\n");
  }
  return base_[argc_ - 1];
}

svalue_t* LPCFrame::argp(int i) {
  check_index(i);
  return base_ + i;
}

int LPCFrame::type(int i) const {
  check_index(i);
  return base_[i].type;
}

int LPCFrame::subtype(int i) const {
  check_index(i);
  return base_[i].subtype;
}

LPC_INT LPCFrame::number(int i) const {
  check_index(i);
  return base_[i].u.number;
}

LPC_FLOAT LPCFrame::real(int i) const {
  check_index(i);
  return base_[i].u.real;
}

LPC_FLOAT LPCFrame::as_real(int i) const {
  check_index(i);
  if (base_[i].type == T_NUMBER) {
    return static_cast<LPC_FLOAT>(base_[i].u.number);
  }
  return base_[i].u.real;
}

const char* LPCFrame::string(int i) const {
  check_index(i);
  return base_[i].u.string;
}

int LPCFrame::string_len(int i) const {
  check_index(i);
  return SVALUE_STRLEN(&base_[i]);
}

array_t* LPCFrame::array(int i) const {
  check_index(i);
  return base_[i].u.arr;
}

mapping_t* LPCFrame::mapping(int i) const {
  check_index(i);
  return base_[i].u.map;
}

buffer_t* LPCFrame::buffer(int i) const {
  check_index(i);
  return base_[i].u.buf;
}

object_t* LPCFrame::object(int i) const {
  check_index(i);
  return base_[i].u.ob;
}

funptr_t* LPCFrame::funptr(int i) const {
  check_index(i);
  return base_[i].u.fp;
}

promise_t* LPCFrame::promise(int i) const {
  check_index(i);
  return base_[i].u.prom;
}

svalue_t LPCFrame::steal(int i) {
  check_index(i);
  svalue_t v = base_[i];
  base_[i] = const0;
  return v;
}

void LPCFrame::push(LPC_INT n) {
  push_number(n);
  note_push();
}

void LPCFrame::push(LPC_FLOAT n) {
  push_real(n);
  note_push();
}

void LPCFrame::push(svalue_t* v) {
  STACK_INC;
  assign_svalue_no_free(sp, v);
  note_push();
}

void LPCFrame::push_undefined() {
  ::push_undefined();
  note_push();
}

void LPCFrame::push_str_malloced(const char* p) {
  push_malloced_string(p);
  note_push();
}

void LPCFrame::push_str_shared(const char* p) {
  push_shared_string(p);
  note_push();
}

void LPCFrame::push_str_constant(const char* p) {
  push_constant_string(p);
  note_push();
}

void LPCFrame::push_str_copy(const char* p) {
  copy_and_push_string(p);
  note_push();
}

void LPCFrame::push(object_t* ob) {
  push_object(ob);
  note_push();
}

void LPCFrame::push(array_t* v) {
  push_array(v);
  note_push();
}

void LPCFrame::push_refed(array_t* v) {
  push_refed_array(v);
  note_push();
}

void LPCFrame::push(mapping_t* m) {
  push_mapping(m);
  note_push();
}

void LPCFrame::push_refed(mapping_t* m) {
  push_refed_mapping(m);
  note_push();
}

void LPCFrame::push(buffer_t* b) {
  push_buffer(b);
  note_push();
}

void LPCFrame::push_refed(buffer_t* b) {
  push_refed_buffer(b);
  note_push();
}

void LPCFrame::ret(LPC_INT n) {
  pop_n_elems(argc_);
  push_number(n);
  finalize_ret_slot();
}

void LPCFrame::ret(LPC_FLOAT n) {
  pop_n_elems(argc_);
  push_real(n);
  finalize_ret_slot();
}

void LPCFrame::ret_void() {
  pop_n_elems(argc_);
  argc_ = 0;
  base_ = sp + 1;
  finalized_ = true;
}

void LPCFrame::ret(svalue_t v) {
  pop_n_elems(argc_);
  STACK_INC;
  *sp = v;
  finalize_ret_slot();
}

void LPCFrame::ret_copy(svalue_t* v) {
  pop_n_elems(argc_);
  push_svalue(v);
  finalize_ret_slot();
}

void LPCFrame::ret(array_t* v) {
  pop_n_elems(argc_);
  push_array(v);
  finalize_ret_slot();
}

void LPCFrame::ret_refed(array_t* v) {
  pop_n_elems(argc_);
  push_refed_array(v);
  finalize_ret_slot();
}

void LPCFrame::ret(mapping_t* m) {
  pop_n_elems(argc_);
  push_mapping(m);
  finalize_ret_slot();
}

void LPCFrame::ret_refed(mapping_t* m) {
  pop_n_elems(argc_);
  push_refed_mapping(m);
  finalize_ret_slot();
}

void LPCFrame::ret_str_malloced(const char* p) {
  pop_n_elems(argc_);
  push_malloced_string(p);
  finalize_ret_slot();
}

void LPCFrame::ret_str_shared(const char* p) {
  pop_n_elems(argc_);
  push_shared_string(p);
  finalize_ret_slot();
}

void LPCFrame::ret_str_constant(const char* p) {
  pop_n_elems(argc_);
  push_constant_string(p);
  finalize_ret_slot();
}

void LPCFrame::ret_str_copy(const char* p) {
  pop_n_elems(argc_);
  copy_and_push_string(p);
  finalize_ret_slot();
}

void LPCFrame::ret(object_t* ob) {
  pop_n_elems(argc_);
  push_object(ob);
  finalize_ret_slot();
}

void LPCFrame::ret(buffer_t* b) {
  pop_n_elems(argc_);
  push_buffer(b);
  finalize_ret_slot();
}

void LPCFrame::ret_refed(buffer_t* b) {
  pop_n_elems(argc_);
  push_refed_buffer(b);
  finalize_ret_slot();
}

void LPCFrame::ret_undefined() {
  pop_n_elems(argc_);
  ::push_undefined();
  finalize_ret_slot();
}

void LPCFrame::ret_front_number(LPC_INT n) {
  if (argc_ < 1) {
    error("LPCFrame::ret_front_number with empty window\n");
  }
  pop_n_elems(argc_ - 1);
  free_svalue(sp, "LPCFrame::ret_front_number");
  put_number(n);
  argc_ = 1;
  base_ = sp;
  finalized_ = true;
}

void LPCFrame::set_number(int i, LPC_INT n) {
  check_index(i);
  free_svalue(base_ + i, "LPCFrame::set_number");
  base_[i].type = T_NUMBER;
  base_[i].subtype = 0;
  base_[i].u.number = n;
}

void LPCFrame::set_real(int i, LPC_FLOAT n) {
  check_index(i);
  free_svalue(base_ + i, "LPCFrame::set_real");
  base_[i].type = T_REAL;
  base_[i].u.real = n;
}

void LPCFrame::set_undefined(int i) {
  check_index(i);
  free_svalue(base_ + i, "LPCFrame::set_undefined");
  base_[i] = const0u;
}

void LPCFrame::set_top_number(LPC_INT n) {
  free_svalue(&top(), "LPCFrame::set_top_number");
  top().type = T_NUMBER;
  top().subtype = 0;
  top().u.number = n;
}

void LPCFrame::set_top_real(LPC_FLOAT n) {
  free_svalue(&top(), "LPCFrame::set_top_real");
  top().type = T_REAL;
  top().u.real = n;
}

void LPCFrame::set_top_array(array_t* v) {
  free_svalue(&top(), "LPCFrame::set_top_array");
  top().type = T_ARRAY;
  top().u.arr = v;
}

void LPCFrame::set_top_refed_array(array_t* v) {
  free_svalue(&top(), "LPCFrame::set_top_refed_array");
  top().type = T_ARRAY;
  top().u.arr = v;
  v->ref++;
}

void LPCFrame::set_top_str_malloced(const char* p) {
  free_svalue(&top(), "LPCFrame::set_top_str_malloced");
  top().type = T_STRING;
  top().subtype = STRING_MALLOC;
  top().u.string = p;
}

void LPCFrame::set_top_str_constant(const char* p) {
  free_svalue(&top(), "LPCFrame::set_top_str_constant");
  top().type = T_STRING;
  top().subtype = STRING_CONSTANT;
  top().u.string = p;
}

void LPCFrame::set_top_undefined() {
  free_svalue(&top(), "LPCFrame::set_top_undefined");
  top() = const0u;
}

void LPCFrame::done() { finalized_ = true; }

void LPCFrame::drop_front(int n) {
  if (n < 0 || n > argc_) {
    error("LPCFrame::drop_front(%d) with argc=%d\n", n, argc_);
  }
  for (int i = 0; i < n; i++) {
    free_svalue(base_ + i, "LPCFrame::drop_front");
    base_[i] = const0;
  }
  int remain = argc_ - n;
  for (int i = 0; i < remain; i++) {
    base_[i] = base_[n + i];
    base_[n + i] = const0;
  }
  for (int i = 0; i < n; i++) {
    pop_stack();
  }
  argc_ = remain;
  if (argc_ == 0) {
    base_ = sp + 1;
  }
}

void LPCFrame::pop_back(int n) {
  if (n < 0 || n > argc_) {
    error("LPCFrame::pop_back(%d) with argc=%d\n", n, argc_);
  }
  for (int i = 0; i < n; i++) {
    pop_stack();
  }
  argc_ -= n;
  if (argc_ == 0) {
    base_ = sp + 1;
  }
}

void LPCFrame::discard_to() {
  if (mode_ != Mode::Mark && mode_ != Mode::Builder) {
    error("LPCFrame::discard_to requires Mark or Builder mode\n");
  }
  while (sp >= base_) {
    pop_stack();
  }
  argc_ = 0;
  finalized_ = true;
}

void LPCFrame::process_callback(int narg, function_to_call_t* ftc, int opcode) {
  process_efun_callback(narg, ftc, opcode, argc_);
}

svalue_t* LPCFrame::apply(const char* fun, object_t* ob, int origin) {
  int const n = argc_;
  finalized_ = true;
  return ::apply(fun, ob, n, origin);
}

svalue_t* LPCFrame::safe_apply(const char* fun, object_t* ob, int origin) {
  if (ob->flags & O_DESTRUCTED) {
    pop_n_elems(argc_);
    argc_ = 0;
    base_ = sp + 1;
    finalized_ = true;
    return nullptr;
  }

  error_context_t econ;
  save_context(&econ);
  /* Callee owns the arguments once the call starts (see safe_apply in
   * apply.cc): unwind mark sits below the first arg. */
  econ.save_sp = base_ - 1;

  svalue_t* ret = nullptr;
  try {
    ret = ::apply(fun, ob, argc_, origin);
  } catch (const char*) {
    restore_context(&econ);
    ret = nullptr;
  }
  pop_context(&econ);
  finalized_ = true;
  return ret;
}
