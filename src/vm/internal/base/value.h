/* value.h -- LPC values in C++: one owning type and two borrowing views.
 *
 *   Value      Owns one svalue and every reference in it. Move-only; the
 *              destructor releases what it still holds, including during an
 *              error() unwind (error() is a C++ throw), so a Value cannot
 *              leak the way a raw svalue_t local does. An efun returns one,
 *              Call::push() takes one, a CallResult holds one.
 *
 *   StringArg  A borrowed LPC string. c_str() is NUL-terminated; size() is
 *              the byte length, read from the string's header rather than
 *              recomputed with strlen().
 *
 *   Arg        A borrowed argument slot whose type is a union or mixed. Test
 *              type(), then read through the accessor for that type; the
 *              accessors check, so a wrong guess is a clean error rather
 *              than a reinterpreted union.
 *
 * Ownership is spelled out in one vocabulary, shared by every factory below
 * and by nothing else:
 *
 *   refed(p)    takes over the reference the caller already holds (a value
 *               it just allocated, or one a helper handed it);
 *   addref(p)   adds a reference (a borrowed pointer);
 *   copy(sv)    copies an svalue, adding a reference to what it points to;
 *   owned(sv)   takes over an svalue and every reference in it.
 *
 * Objects use the same pair. `object(ob)` is an alias of `addref(ob)` that
 * matches what LPC sees: null or (being) destructed becomes undefined.
 *
 * There is deliberately no conversion from a bare pointer: which of refed()
 * and addref() was meant is the whole question, and a short spelling would
 * have to guess. Numbers convert implicitly (`return 0;` in an efun).
 *
 * The borrowing views point into the stack slot of a live argument. They
 * are valid until that slot is released: the efun returns, or calls
 * EfunArgs::pop_back()/take() on it.
 *
 * All of it lives in namespace vm, with efun_args.h's EfunArgs, Call and
 * CallResult; code outside the namespace spells the names out (vm::Value).
 */

#ifndef VM_VALUE_H
#define VM_VALUE_H

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "vm/internal/base/svalue.h"

struct object_t;
struct array_t;
struct mapping_t;
struct buffer_t;
struct funptr_t;
struct promise_t;

namespace vm {

class Value {
 public:
  /* The number 0. */
  Value() noexcept {
    set_number(0);
    track();
  }
  template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
  Value(T n) noexcept {  // NOLINT(google-explicit-constructor): `return 1;`
    set_number(static_cast<LPC_INT>(n));
    track();
  }
  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
  Value(T n) noexcept {  // NOLINT(google-explicit-constructor)
    v_.type = T_REAL;
    v_.subtype = 0;
    v_.u.real = static_cast<LPC_FLOAT>(n);
    track();
  }

  Value(Value&& other) noexcept : v_(other.v_) {
    other.set_number(0);
    track();
  }
  /* free_svalue on a Value never throws: Values do not hold T_ERROR_HANDLER
   * slots (those stay on the eval stack). Coverity still sees a transitive
   * fmt::format_error from deep debug paths; the noexcept stands. */
  // coverity[uncaught_exception]
  Value& operator=(Value&& other) noexcept {
    if (this != &other) {
      svalue_t old = v_;
      v_ = other.v_;
      other.set_number(0);
      free_svalue(&old, "Value");
    }
    return *this;
  }
  Value(const Value&) = delete;
  Value& operator=(const Value&) = delete;
  // coverity[uncaught_exception]
  ~Value() {
    free_svalue(&v_, "Value");
    untrack();
  }

  /* 0 with undefinedp() true. */
  static Value undefined();
  static Value owned(svalue_t v);
  /* Moves the value out of a slot the caller controls, leaving 0 there (no
   * reference counting at all): e.g. a call's result out of the driver's
   * shared return slot. */
  static Value take(svalue_t* slot);
  static Value copy(const svalue_t* v);

  static Value refed(array_t* v);
  static Value addref(array_t* v);
  static Value refed(mapping_t* m);
  static Value addref(mapping_t* m);
  static Value refed(buffer_t* b);
  static Value addref(buffer_t* b);
  static Value refed(funptr_t* fp);
  static Value addref(funptr_t* fp);
  static Value refed(promise_t* p);
  static Value addref(promise_t* p);
  /* Take over a held object reference (no increment). Null or (being)
   * destructed becomes undefined and the held ref is dropped. */
  static Value refed(object_t* ob);
  /* Add a reference; null or (being) destructed becomes undefined. */
  static Value addref(object_t* ob);
  /* LPC-facing alias of addref(object_t*): what LPC sees for this pointer. */
  static Value object(object_t* ob);
  /* A class instance (a T_CLASS array). */
  static Value refed_class(array_t* v);

  static Value str_malloced(const char* p); /* takes over a malloced string */
  static Value str_shared(const char* p);   /* adds a reference to a shared one */
  static Value str_make_shared(const char* p); /* a shared copy (make_shared_string) */
  static Value str_constant(const char* p); /* static storage, never freed */
  static Value str_copy(const char* p);     /* copies up to the NUL */
  static Value str_copy(std::string_view s);

  uint32_t type() const { return v_.type; }
  bool is(uint32_t t) const { return v_.type == t; }
  svalue_t& operator*() { return v_; }
  const svalue_t& operator*() const { return v_; }
  svalue_t* operator->() { return &v_; }
  const svalue_t* operator->() const { return &v_; }
  /* The held value, still owned here; for helpers that take an svalue_t*. */
  svalue_t* get() { return &v_; }
  const svalue_t* get() const { return &v_; }

  /* An addref'd copy of the held object, or undefined if this is not a live
   * object — for re-pushing a Value held across LPC (e.g. event origin). */
  Value as_object() const;

  /* Make the held string uniquely owned and writable (unlink_string_svalue).
   * Returns the mutable buffer; still owned here. */
  char* unlink_string();

  /* Gives up ownership: the caller now holds the references. */
  [[nodiscard]] svalue_t release() {
    svalue_t v = v_;
    set_number(0);
    return v;
  }

 private:
  explicit Value(svalue_t v) : v_(v) { track(); }
  template <typename T>
  static svalue_t pointer_svalue(uint32_t type, T* p) {
    svalue_t v;
    v.type = type;
    v.subtype = 0;
    v.u.number = 0;
    v.u.refed = reinterpret_cast<refed_t*>(p);
    return v;
  }
  static svalue_t string_svalue(const char* p, unsigned short subtype) {
    svalue_t v;
    v.type = T_STRING;
    v.subtype = subtype;
    v.u.string = p;
    return v;
  }
  void set_number(LPC_INT n) {
    v_.type = T_NUMBER;
    v_.subtype = 0;
    v_.u.number = n;
  }

  svalue_t v_;

#ifdef DEBUGMALLOC_EXTENSIONS
  /* A Value held while LPC runs is a reference holder off the object graph
   * (AGENTS.md section 3), so check_memory() has to see it: every live
   * Value is on a list that mark_live_values() walks. */
  void track() noexcept;
  void untrack() noexcept;
  Value* prev_;
  Value* next_;
  friend void mark_live_values();
#else
  void track() noexcept {}
  void untrack() noexcept {}
#endif
};

#ifdef DEBUGMALLOC_EXTENSIONS
/* check_memory()'s mark phase: count what every live Value holds. */
void mark_live_values();
#endif

/* The factories that only build an svalue are inline: an efun returns
 * through one on every call. (The slot left by take() is written field by
 * field rather than copied from const0 so the compiler knows it holds a
 * plain 0, and the release that follows in EfunArgs::finish() folds away.) */
inline Value Value::owned(svalue_t v) { return Value(v); }
inline Value Value::take(svalue_t* slot) {
  svalue_t v = *slot;
  slot->type = T_NUMBER;
  slot->subtype = 0;
  slot->u.number = 0;
  return Value(v);
}
inline Value Value::refed(array_t* p) { return Value(pointer_svalue(T_ARRAY, p)); }
inline Value Value::refed(mapping_t* p) { return Value(pointer_svalue(T_MAPPING, p)); }
inline Value Value::refed(buffer_t* p) { return Value(pointer_svalue(T_BUFFER, p)); }
inline Value Value::refed(funptr_t* p) { return Value(pointer_svalue(T_FUNCTION, p)); }
inline Value Value::refed(promise_t* p) { return Value(pointer_svalue(T_PROMISE, p)); }
inline Value Value::refed_class(array_t* p) { return Value(pointer_svalue(T_CLASS, p)); }
inline Value Value::object(object_t* ob) { return addref(ob); }
inline Value Value::str_malloced(const char* p) {
  return Value(string_svalue(p, STRING_MALLOC));
}
inline Value Value::str_constant(const char* p) {
  return Value(string_svalue(p, STRING_CONSTANT));
}

class StringArg {
 public:
  explicit StringArg(const svalue_t* sv) : sv_(sv) {}

  const char* c_str() const { return sv_->u.string; }
  size_t size() const { return SVALUE_STRLEN(sv_); }
  bool empty() const { return size() == 0; }
  std::string_view view() const { return {c_str(), size()}; }
  operator std::string_view() const { return view(); }  // NOLINT

  /* The argument's slot, for helpers that take one. */
  const svalue_t* sv() const { return sv_; }
  /* The same string as an owned value: a reference, not a copy of the bytes. */
  Value copy() const { return Value::copy(sv_); }

 private:
  const svalue_t* sv_;
};

class Arg {
 public:
  explicit Arg(svalue_t* sv) : sv_(sv) {}

  uint32_t type() const { return sv_->type; }
  unsigned short subtype() const { return sv_->subtype; }
  bool is(uint32_t t) const { return sv_->type == t; }

  LPC_INT number() const { return expect(T_NUMBER).u.number; }
  LPC_FLOAT real() const { return expect(T_REAL).u.real; }
  /* An int-or-float as a float. */
  LPC_FLOAT as_real() const {
    return sv_->type == T_NUMBER ? static_cast<LPC_FLOAT>(sv_->u.number) : real();
  }
  StringArg string() const { return StringArg(&expect(T_STRING)); }
  array_t* array() const { return expect(T_ARRAY).u.arr; }
  /* A class instance is an array_t under T_CLASS; array() would reject it. */
  array_t* class_rep() const { return expect(T_CLASS).u.arr; }
  mapping_t* mapping() const { return expect(T_MAPPING).u.map; }
  buffer_t* buffer() const { return expect(T_BUFFER).u.buf; }
  object_t* object() const { return expect(T_OBJECT).u.ob; }
  funptr_t* funptr() const { return expect(T_FUNCTION).u.fp; }
  promise_t* promise() const { return expect(T_PROMISE).u.prom; }

  /* The slot itself, for helpers that take an svalue_t*. */
  svalue_t* sv() const { return sv_; }
  Value copy() const { return Value::copy(sv_); }

 private:
  const svalue_t& expect(uint32_t t) const {
    if (sv_->type != t) {
      wrong_type(t);
    }
    return *sv_;
  }
  [[noreturn]] void wrong_type(uint32_t expected) const;

  svalue_t* sv_;
};

}  // namespace vm

#endif /* VM_VALUE_H */
