/*
 * package_ffi -- foreign function interface for LPC.
 *
 * dlopen/dlsym + libffi to load native shared libraries and call C
 * functions whose signatures are described at runtime, plus LPC function
 * pointers exposed to C as callbacks (libffi closures).
 *
 * Design & security model: docs/driver/ffi.md. The short version:
 *  - All pointer/byte data crosses as `buffer`; raw pointer VALUES
 *    (returned pointers, buffer addresses, callback code addresses) are
 *    ints. LPC strings are UTF-8-native and NEVER implicitly marshalled.
 *  - Every ffi_load / ffi_symbol / ffi_prepare / ffi_callback /
 *    ffi_peek is gated by the master apply valid_ffi(op, arg, caller);
 *    a missing apply denies (secure default). An optional config
 *    allow-list ("ffi allowed libraries") is enforced first.
 *  - Native memory is a `buffer` whose bytes ARE the storage, so the GC
 *    tracks the lifetime.
 */

#include "base/package_api.h"

#include "packages/ffi/ffi.h"
#include "thirdparty/scope_guard/scope_guard.hpp"

#ifdef PACKAGE_FFI

#include "include/ffi.h"  // LPC-visible FFI_* type codes

#include <ffi.h>  // libffi

#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// ---------------------------------------------------------------------------
// Handle tables. Ints index into these (the db_connect pattern); each
// map owns its resources and is torn down at ffi_cleanup().
// ---------------------------------------------------------------------------

namespace {

struct FfiLibrary {
  void* handle;
  std::string path;
  // IDs (into g_funcs) of every FfiFunc whose ->addr was dlsym()'d out of
  // this library. ffi_call() has no way to tell a stale address from a live
  // one after dlclose(), so ffi_unload() walks this list and invalidates
  // each one (FfiFunc::valid) instead of leaving them dangling -- refusing
  // to unload at all would break the ordinary prepare-call-then-unload
  // lifecycle (ffi_prepare() never has a matching "un-prepare").
  std::vector<int> prepared_func_ids;
};

struct FfiFunc {
  void* addr;
  ffi_cif cif;
  std::vector<ffi_type*> arg_types;
  std::vector<int> arg_codes;
  int ret_code;
  ffi_type* ret_type;
  bool valid = true;  // false once this func's owning library is unloaded
};

struct FfiCallback {
  ffi_closure* closure;
  void* code;  // executable trampoline address handed to C
  ffi_cif cif;
  std::vector<ffi_type*> arg_types;
  std::vector<int> arg_codes;
  int ret_code;
  ffi_type* ret_type;
  funptr_t* fun;  // LPC function pointer, ref-held
  // True while closure_dispatch() is on the call stack for this callback.
  // free_callback() would ffi_closure_free(closure) -- the executable
  // trampoline libffi is CURRENTLY EXECUTING to have reached this call at
  // all -- so freeing it mid-dispatch (e.g. a callback that calls
  // ffi_callback_free() on its own id) doesn't just free a struct out from
  // under a later read (that part's fixed below); it unmaps code the CPU
  // is still going to return into, crashing inside libffi itself. There's
  // no legitimate reason a callback needs to free itself, so refuse it.
  bool dispatching = false;
};

std::unordered_map<int, FfiLibrary*> g_libs;
std::unordered_map<int, FfiFunc*> g_funcs;
std::unordered_map<int, FfiCallback*> g_callbacks;
int g_next_handle = 1;

std::string g_last_error;

// One value slot big enough for any scalar libffi argument.
union FfiArgSlot {
  int8_t i8;
  uint8_t u8;
  int16_t i16;
  uint16_t u16;
  int32_t i32;
  uint32_t u32;
  int64_t i64;
  uint64_t u64;
  float f32;
  double f64;
  void* ptr;
};

// FFI_* code -> libffi ffi_type. error() on an unknown code.
ffi_type* code_to_type(int code) {
  switch (code) {
    case FFI_VOID:
      return &ffi_type_void;
    case FFI_INT8:
      return &ffi_type_sint8;
    case FFI_UINT8:
      return &ffi_type_uint8;
    case FFI_INT16:
      return &ffi_type_sint16;
    case FFI_UINT16:
      return &ffi_type_uint16;
    case FFI_INT32:
      return &ffi_type_sint32;
    case FFI_UINT32:
      return &ffi_type_uint32;
    case FFI_INT64:
      return &ffi_type_sint64;
    case FFI_UINT64:
      return &ffi_type_uint64;
    case FFI_FLOAT:
      return &ffi_type_float;
    case FFI_DOUBLE:
      return &ffi_type_double;
    case FFI_POINTER:
      return &ffi_type_pointer;
    default:
      error("ffi: unknown type code %d\n", code);
  }
}

int type_size(int code) { return static_cast<int>(code_to_type(code)->size); }

bool is_int_code(int code) { return code >= FFI_INT8 && code <= FFI_UINT64; }

// ---------------------------------------------------------------------------
// dlopen/dlsym portability wrapper.
// ---------------------------------------------------------------------------

void* plat_dlopen(const char* path) {
  // An empty path means "the driver's own global symbols" (libc/libm
  // linked into the process) -- a portable handle for standard C
  // functions without naming a platform-specific library file.
  bool self = (path == nullptr || path[0] == 0);
#ifdef _WIN32
  return reinterpret_cast<void*>(self ? GetModuleHandle(nullptr) : LoadLibraryA(path));
#else
  return dlopen(self ? nullptr : path, RTLD_NOW | RTLD_LOCAL | RTLD_GLOBAL);
#endif
}

void plat_dlclose(void* handle) {
#ifdef _WIN32
  FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
  dlclose(handle);
#endif
}

void* plat_dlsym(void* handle, const char* name) {
#ifdef _WIN32
  return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
#else
  return dlsym(handle, name);
#endif
}

const char* plat_dlerror() {
#ifdef _WIN32
  return "library/symbol not found";
#else
  const char* e = dlerror();
  return e ? e : "unknown dynamic-loader error";
#endif
}

// ---------------------------------------------------------------------------
// Security gate: master::valid_ffi(op, arg, caller).
// ---------------------------------------------------------------------------

void check_valid_ffi(const char* op, svalue_t* arg) {
  push_constant_string(op);
  if (arg == nullptr) {
    push_number(0);
  } else {
    push_svalue(arg);
  }
  push_object(current_object);
  svalue_t* ret = safe_apply_master_ob(APPLY_VALID_FFI, 3);
  if (ret && (ret == reinterpret_cast<svalue_t*>(-1) || (ret->type == T_NUMBER && ret->u.number))) {
    return;
  }
  error("FFI security violation: '%s' denied by master::valid_ffi.\n", op);
}

// Optional driver-level allow-list ("ffi allowed libraries"): a
// colon-separated list of permitted paths. Empty => rely on the master.
bool library_allowed(const char* path) {
  const char* allowed = CONFIG_STR(__FFI_ALLOWED_LIBRARIES__);
  if (allowed == nullptr || allowed[0] == 0) {
    return true;
  }
  std::string list(allowed);
  std::string want(path);
  size_t pos = 0;
  while (pos <= list.size()) {
    size_t sep = list.find(':', pos);
    std::string entry = list.substr(pos, sep == std::string::npos ? std::string::npos : sep - pos);
    if (entry == want) {
      return true;
    }
    if (sep == std::string::npos) {
      break;
    }
    pos = sep + 1;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Marshalling: LPC svalue -> C value slot (for a call ARGUMENT).
// ---------------------------------------------------------------------------

void lpc_to_slot(int code, svalue_t* arg, FfiArgSlot* slot) {
  if (is_int_code(code)) {
    if (arg->type != T_NUMBER) {
      error("ffi: integer argument expected an int.\n");
    }
    LPC_INT v = arg->u.number;
    switch (code) {
      case FFI_INT8:
        slot->i8 = static_cast<int8_t>(v);
        break;
      case FFI_UINT8:
        slot->u8 = static_cast<uint8_t>(v);
        break;
      case FFI_INT16:
        slot->i16 = static_cast<int16_t>(v);
        break;
      case FFI_UINT16:
        slot->u16 = static_cast<uint16_t>(v);
        break;
      case FFI_INT32:
        slot->i32 = static_cast<int32_t>(v);
        break;
      case FFI_UINT32:
        slot->u32 = static_cast<uint32_t>(v);
        break;
      case FFI_INT64:
        slot->i64 = static_cast<int64_t>(v);
        break;
      case FFI_UINT64:
        slot->u64 = static_cast<uint64_t>(v);
        break;
    }
    return;
  }
  switch (code) {
    case FFI_FLOAT:
      slot->f32 = static_cast<float>(arg->type == T_REAL ? arg->u.real : arg->u.number);
      break;
    case FFI_DOUBLE:
      slot->f64 = static_cast<double>(arg->type == T_REAL ? arg->u.real : arg->u.number);
      break;
    case FFI_POINTER:
      // A buffer -> its bytes; an int -> a literal address (0 = NULL).
      if (arg->type == T_BUFFER) {
        slot->ptr = arg->u.buf->size ? static_cast<void*>(&arg->u.buf->item[0]) : nullptr;
      } else if (arg->type == T_NUMBER) {
        slot->ptr = reinterpret_cast<void*>(static_cast<intptr_t>(arg->u.number));
      } else {
        error("ffi: pointer argument expected a buffer or an int address.\n");
      }
      break;
    default:
      error("ffi: cannot marshal type code %d\n", code);
  }
}

// C return storage -> LPC value pushed onto the stack. Integer returns
// are widened to ffi_arg by libffi; narrow per the declared code.
void return_to_lpc(int code, void* storage) {
  ffi_arg raw = 0;
  if (is_int_code(code)) {
    raw = *reinterpret_cast<ffi_arg*>(storage);
  }
  switch (code) {
    case FFI_VOID:
      push_number(0);
      break;
    case FFI_INT8:
      push_number(static_cast<int8_t>(raw));
      break;
    case FFI_UINT8:
      push_number(static_cast<uint8_t>(raw));
      break;
    case FFI_INT16:
      push_number(static_cast<int16_t>(raw));
      break;
    case FFI_UINT16:
      push_number(static_cast<uint16_t>(raw));
      break;
    case FFI_INT32:
      push_number(static_cast<int32_t>(raw));
      break;
    case FFI_UINT32:
      push_number(static_cast<uint32_t>(raw));
      break;
    case FFI_INT64:
      push_number(static_cast<int64_t>(raw));
      break;
    case FFI_UINT64:
      push_number(static_cast<LPC_INT>(static_cast<uint64_t>(raw)));
      break;
    case FFI_FLOAT:
      push_real(*reinterpret_cast<float*>(storage));
      break;
    case FFI_DOUBLE:
      push_real(*reinterpret_cast<double*>(storage));
      break;
    case FFI_POINTER:
      push_number(reinterpret_cast<LPC_INT>(*reinterpret_cast<void**>(storage)));
      break;
    default:
      error("ffi: cannot marshal return type code %d\n", code);
  }
}

// ---------------------------------------------------------------------------
// Callback dispatch: a C call into a libffi closure re-enters the VM and
// calls the held LPC function pointer. safe_call_function_pointer catches
// LPC errors (they must NOT unwind across the native stack), so a failing
// callback yields a zero/NULL C return.
// ---------------------------------------------------------------------------

void c_arg_to_lpc(int code, void* carg) {
  switch (code) {
    case FFI_INT8:
      push_number(*reinterpret_cast<int8_t*>(carg));
      break;
    case FFI_UINT8:
      push_number(*reinterpret_cast<uint8_t*>(carg));
      break;
    case FFI_INT16:
      push_number(*reinterpret_cast<int16_t*>(carg));
      break;
    case FFI_UINT16:
      push_number(*reinterpret_cast<uint16_t*>(carg));
      break;
    case FFI_INT32:
      push_number(*reinterpret_cast<int32_t*>(carg));
      break;
    case FFI_UINT32:
      push_number(*reinterpret_cast<uint32_t*>(carg));
      break;
    case FFI_INT64:
      push_number(*reinterpret_cast<int64_t*>(carg));
      break;
    case FFI_UINT64:
      push_number(static_cast<LPC_INT>(*reinterpret_cast<uint64_t*>(carg)));
      break;
    case FFI_FLOAT:
      push_real(*reinterpret_cast<float*>(carg));
      break;
    case FFI_DOUBLE:
      push_real(*reinterpret_cast<double*>(carg));
      break;
    case FFI_POINTER:
      push_number(reinterpret_cast<LPC_INT>(*reinterpret_cast<void**>(carg)));
      break;
    default:
      push_number(0);
  }
}

void lpc_return_to_c(int code, svalue_t* r, void* ret) {
  if (code == FFI_VOID) {
    return;
  }
  if (is_int_code(code) || code == FFI_POINTER) {
    LPC_INT v = (r && r->type == T_NUMBER) ? r->u.number : 0;
    if (code == FFI_POINTER) {
      *reinterpret_cast<void**>(ret) = reinterpret_cast<void*>(static_cast<intptr_t>(v));
    } else {
      *reinterpret_cast<ffi_arg*>(ret) = static_cast<ffi_arg>(v);
    }
    return;
  }
  double d = 0.0;
  if (r && r->type == T_REAL) {
    d = r->u.real;
  } else if (r && r->type == T_NUMBER) {
    d = static_cast<double>(r->u.number);
  }
  if (code == FFI_FLOAT) {
    *reinterpret_cast<float*>(ret) = static_cast<float>(d);
  } else {
    *reinterpret_cast<double*>(ret) = d;
  }
}

void closure_dispatch(ffi_cif* /*cif*/, void* ret, void** args, void* user) {
  auto* cb = static_cast<FfiCallback*>(user);
  int nargs = static_cast<int>(cb->arg_codes.size());
  for (int i = 0; i < nargs; i++) {
    c_arg_to_lpc(cb->arg_codes[i], args[i]);
  }
  // safe_call_function_pointer() runs arbitrary LPC, which can call
  // ffi_callback_free() on this very callback's id -- free_callback() then
  // deletes `cb`. Capture everything still needed from `cb` before the call
  // so nothing reads through it afterward.
  int ret_code = cb->ret_code;
  // f_ffi_callback_free() checks this flag and refuses to free a
  // dispatching callback (see its own comment) -- freeing cb->closure here
  // would unmap the executable trampoline this very call is running from.
  cb->dispatching = true;
  DEFER { cb->dispatching = false; };
  set_eval(max_eval_cost);
  svalue_t* r = safe_call_function_pointer(cb->fun, nargs);
  lpc_return_to_c(ret_code, r, ret);
}

// ---------------------------------------------------------------------------
// Cleanup helpers.
// ---------------------------------------------------------------------------

void free_callback(FfiCallback* cb) {
  if (cb->closure) {
    ffi_closure_free(cb->closure);
  }
  if (cb->fun) {
    free_funp(cb->fun);
  }
  delete cb;
}

}  // namespace

// ===========================================================================
// efuns
// ===========================================================================

#ifdef F_FFI_LOAD
void f_ffi_load() {
  const char* path = sp->u.string;
  if (!library_allowed(path)) {
    error("FFI: library '%s' is not in the configured allow-list.\n", path);
  }
  check_valid_ffi("load", sp);

  void* handle = plat_dlopen(path);
  if (handle == nullptr) {
    g_last_error = plat_dlerror();
    free_string_svalue(sp);
    put_number(0);
    return;
  }
  auto* lib = new FfiLibrary{handle, path};
  int id = g_next_handle++;
  g_libs[id] = lib;
  free_string_svalue(sp);
  put_number(id);
}
#endif

#ifdef F_FFI_UNLOAD
void f_ffi_unload() {
  int id = sp->u.number;
  auto it = g_libs.find(id);
  if (it != g_libs.end()) {
    // Invalidate every FfiFunc prepared from this library before closing
    // it: their ->addr was dlsym()'d out of the handle we're about to
    // dlclose(), so calling one afterward would jump into unmapped memory.
    for (int func_id : it->second->prepared_func_ids) {
      auto fit = g_funcs.find(func_id);
      if (fit != g_funcs.end()) {
        fit->second->valid = false;
      }
    }
    plat_dlclose(it->second->handle);
    delete it->second;
    g_libs.erase(it);
  }
  pop_stack();
}
#endif

#ifdef F_FFI_SYMBOL
void f_ffi_symbol() {
  const char* name = sp->u.string;
  int id = (sp - 1)->u.number;
  check_valid_ffi("symbol", sp);

  auto it = g_libs.find(id);
  if (it == g_libs.end()) {
    error("ffi_symbol: invalid library handle %d\n", id);
  }
  void* addr = plat_dlsym(it->second->handle, name);
  LPC_INT result = reinterpret_cast<LPC_INT>(addr);
  pop_2_elems();
  push_number(result);
}
#endif

#ifdef F_FFI_PREPARE
void f_ffi_prepare() {
  array_t* arg_arr = sp->u.arr;
  int ret_code = (sp - 1)->u.number;
  const char* name = (sp - 2)->u.string;
  int lib_id = (sp - 3)->u.number;

  check_valid_ffi("prepare", sp - 2);

  auto it = g_libs.find(lib_id);
  if (it == g_libs.end()) {
    error("ffi_prepare: invalid library handle %d\n", lib_id);
  }
  void* addr = plat_dlsym(it->second->handle, name);
  if (addr == nullptr) {
    error("ffi_prepare: symbol '%s' not found: %s\n", name, plat_dlerror());
  }

  // unique_ptr so an error() longjmp (e.g. an invalid type code passed to
  // code_to_type below) frees it during unwinding -- AGENTS.md section 4.
  auto fn = std::make_unique<FfiFunc>();
  fn->addr = addr;
  fn->ret_code = ret_code;
  fn->ret_type = code_to_type(ret_code);
  for (int i = 0; i < arg_arr->size; i++) {
    if (arg_arr->item[i].type != T_NUMBER) {
      error("ffi_prepare: arg-type array must hold ints.\n");
    }
    int code = arg_arr->item[i].u.number;
    fn->arg_codes.push_back(code);
    fn->arg_types.push_back(code_to_type(code));
  }
  if (ffi_prep_cif(&fn->cif, FFI_DEFAULT_ABI, fn->arg_types.size(), fn->ret_type,
                   fn->arg_types.data()) != FFI_OK) {
    error("ffi_prepare: ffi_prep_cif failed for '%s'.\n", name);
  }
  int id = g_next_handle++;
  g_funcs[id] = fn.release();
  // Record that this FfiFunc's ->addr came from this library, so
  // ffi_unload() can invalidate it later (see FfiLibrary::prepared_func_ids).
  it->second->prepared_func_ids.push_back(id);
  pop_n_elems(4);
  push_number(id);
}
#endif

#ifdef F_FFI_CALL
void f_ffi_call() {
  array_t* args = sp->u.arr;
  int func_id = (sp - 1)->u.number;

  auto it = g_funcs.find(func_id);
  if (it == g_funcs.end()) {
    error("ffi_call: invalid function handle %d\n", func_id);
  }
  FfiFunc* fn = it->second;
  if (!fn->valid) {
    error("ffi_call: function %d's library was unloaded with ffi_unload().\n", func_id);
  }
  int nargs = static_cast<int>(fn->arg_codes.size());
  if (args->size != nargs) {
    error("ffi_call: expected %d args, got %d\n", nargs, args->size);
  }

  std::vector<FfiArgSlot> slots(nargs);
  std::vector<void*> avalues(nargs);
  for (int i = 0; i < nargs; i++) {
    lpc_to_slot(fn->arg_codes[i], &args->item[i], &slots[i]);
    // coverity[wrapper_escape] - slots outlives every use of avalues (both die
    // together at function exit; ffi_call below is the only consumer).
    avalues[i] = &slots[i];
  }

  // Return storage: large enough for a widened integer, a pointer, or a
  // double.
  union {
    ffi_arg i;
    void* p;
    float f;
    double d;
  } rvalue;
  ffi_call(&fn->cif, FFI_FN(fn->addr), &rvalue, avalues.data());

  pop_2_elems();
  return_to_lpc(fn->ret_code, &rvalue);
}
#endif

#ifdef F_FFI_ALLOC
void f_ffi_alloc() {
  int nbytes = sp->u.number;
  if (nbytes < 0) {
    error("ffi_alloc: negative size.\n");
  }
  buffer_t* buf = allocate_buffer(nbytes);  // zeroed
  pop_stack();
  push_refed_buffer(buf);
}
#endif

#ifdef F_FFI_FREE
void f_ffi_free() {
  // The buffer is GC-managed; "freeing" zeroes it so a dangling native
  // reader sees no stale data. The block is reclaimed when no LPC value
  // references it.
  buffer_t* buf = sp->u.buf;
  if (buf->size) {
    memset(buf->item, 0, buf->size);
  }
  pop_stack();
}
#endif

#ifdef F_FFI_SIZEOF
void f_ffi_sizeof() {
  int code = sp->u.number;
  int sz = type_size(code);
  pop_stack();
  push_number(sz);
}
#endif

#ifdef F_FFI_PEEK
void f_ffi_peek() {
  // Gated like load/prepare: reading an arbitrary native address is a
  // process-memory disclosure primitive, so the master must approve it
  // even though no foreign code runs.
  check_valid_ffi("peek", sp - 1);
  int nbytes = sp->u.number;
  auto addr = static_cast<intptr_t>((sp - 1)->u.number);
  if (addr == 0) {
    error("ffi_peek: NULL address.\n");
  }
  const char* src = reinterpret_cast<const char*>(addr);
  if (nbytes < 0) {
    // -1: treat as a NUL-terminated C string, capped at max buffer size.
    auto cap = CONFIG_INT(__MAX_BUFFER_SIZE__);
    nbytes = 0;
    while (nbytes < cap && src[nbytes] != 0) {
      nbytes++;
    }
  }
  buffer_t* buf = allocate_buffer(nbytes);
  if (nbytes) {
    memcpy(buf->item, src, nbytes);
  }
  pop_2_elems();
  push_refed_buffer(buf);
}
#endif

#ifdef F_FFI_ADDRESS
void f_ffi_address() {
  buffer_t* buf = sp->u.buf;
  LPC_INT addr = buf->size ? reinterpret_cast<LPC_INT>(&buf->item[0]) : 0;
  pop_stack();
  push_number(addr);
}
#endif

#ifdef F_FFI_READ
void f_ffi_read() {
  int code = sp->u.number;
  LPC_INT offset = (sp - 1)->u.number;
  buffer_t* buf = (sp - 2)->u.buf;
  int sz = type_size(code);
  // Range-check in a non-truncating, non-overflowing form: a huge offset must
  // not truncate to int or wrap past the check into an out-of-bounds memcpy.
  if (offset < 0 || static_cast<size_t>(offset) + static_cast<size_t>(sz) > buf->size) {
    error("ffi_read: offset %" LPC_INT_FMTSTR_P " (size %d) out of range for buffer of %u.\n",
          offset, sz, buf->size);
  }
  void* at = &buf->item[offset];
  FfiArgSlot tmp;
  memcpy(&tmp, at, sz);
  pop_n_elems(3);
  switch (code) {
    case FFI_INT8:
      push_number(tmp.i8);
      break;
    case FFI_UINT8:
      push_number(tmp.u8);
      break;
    case FFI_INT16:
      push_number(tmp.i16);
      break;
    case FFI_UINT16:
      push_number(tmp.u16);
      break;
    case FFI_INT32:
      push_number(tmp.i32);
      break;
    case FFI_UINT32:
      push_number(tmp.u32);
      break;
    case FFI_INT64:
      push_number(tmp.i64);
      break;
    case FFI_UINT64:
      push_number(static_cast<LPC_INT>(tmp.u64));
      break;
    case FFI_FLOAT:
      push_real(tmp.f32);
      break;
    case FFI_DOUBLE:
      push_real(tmp.f64);
      break;
    case FFI_POINTER:
      push_number(reinterpret_cast<LPC_INT>(tmp.ptr));
      break;
    default:
      error("ffi_read: bad type code %d\n", code);
  }
}
#endif

#ifdef F_FFI_WRITE
void f_ffi_write() {
  svalue_t* val = sp;
  int code = (sp - 1)->u.number;
  LPC_INT offset = (sp - 2)->u.number;
  buffer_t* buf = (sp - 3)->u.buf;
  int sz = type_size(code);
  if (offset < 0 || static_cast<size_t>(offset) + static_cast<size_t>(sz) > buf->size) {
    error("ffi_write: offset %" LPC_INT_FMTSTR_P " (size %d) out of range for buffer of %u.\n",
          offset, sz, buf->size);
  }
  FfiArgSlot tmp;
  lpc_to_slot(code, val, &tmp);
  memcpy(&buf->item[offset], &tmp, sz);
  pop_n_elems(4);
}
#endif

#ifdef F_FFI_STRUCT_LAYOUT
void f_ffi_struct_layout() {
  array_t* fields = sp->u.arr;
  int n = fields->size;

  // Compute each field's aligned offset and the struct's total size,
  // following the natural C alignment (align == size for scalars here).
  // unique_ptr with free_array as the deleter so an error() unwind (an
  // unrecognized FFI type code via type_size()/code_to_type(), or a bad
  // field type) frees `offs` instead of leaking it -- same pattern as
  // f_ffi_callback() above.
  std::unique_ptr<array_t, void (*)(array_t*)> offs(allocate_array(n), free_array);
  int cur = 0;
  int max_align = 1;
  for (int i = 0; i < n; i++) {
    if (fields->item[i].type != T_NUMBER) {
      error("ffi_struct_layout: field-type array must hold ints.\n");
    }
    int code = fields->item[i].u.number;
    int sz = type_size(code);
    int align = sz > 0 ? sz : 1;
    if (align > max_align) {
      max_align = align;
    }
    if (cur % align != 0) {
      cur += align - (cur % align);
    }
    offs->item[i].type = T_NUMBER;
    offs->item[i].subtype = 0;
    offs->item[i].u.number = cur;
    cur += sz;
  }
  if (cur % max_align != 0) {
    cur += max_align - (cur % max_align);
  }

  array_t* result = allocate_empty_array(2);
  result->item[0].type = T_NUMBER;
  result->item[0].subtype = 0;
  result->item[0].u.number = cur;
  result->item[1].type = T_ARRAY;
  result->item[1].u.arr = offs.release();  // ref transferred
  pop_stack();
  push_refed_array(result);
}
#endif

#ifdef F_FFI_CALLBACK
void f_ffi_callback() {
  array_t* arg_arr = sp->u.arr;
  int ret_code = (sp - 1)->u.number;
  svalue_t* fun_sv = sp - 2;  // T_FUNCTION

  check_valid_ffi("callback", nullptr);

  // unique_ptr with free_callback as the deleter so an error() longjmp
  // (an invalid type code, or a libffi failure) frees the struct AND any
  // allocated closure during unwinding -- AGENTS.md section 4.
  // free_callback tolerates a null closure/fun.
  std::unique_ptr<FfiCallback, void (*)(FfiCallback*)> cb(new FfiCallback, free_callback);
  cb->closure = nullptr;
  cb->code = nullptr;
  cb->fun = nullptr;
  cb->ret_code = ret_code;
  cb->ret_type = code_to_type(ret_code);
  for (int i = 0; i < arg_arr->size; i++) {
    if (arg_arr->item[i].type != T_NUMBER) {
      error("ffi_callback: arg-type array must hold ints.\n");
    }
    int code = arg_arr->item[i].u.number;
    cb->arg_codes.push_back(code);
    cb->arg_types.push_back(code_to_type(code));
  }

  cb->closure = static_cast<ffi_closure*>(ffi_closure_alloc(sizeof(ffi_closure), &cb->code));
  if (cb->closure == nullptr) {
    error("ffi_callback: ffi_closure_alloc failed.\n");
  }
  if (ffi_prep_cif(&cb->cif, FFI_DEFAULT_ABI, cb->arg_types.size(), cb->ret_type,
                   cb->arg_types.data()) != FFI_OK) {
    error("ffi_callback: ffi_prep_cif failed.\n");
  }
  if (ffi_prep_closure_loc(cb->closure, &cb->cif, closure_dispatch, cb.get(), cb->code) != FFI_OK) {
    error("ffi_callback: ffi_prep_closure_loc failed.\n");
  }

  // Hold a ref on the LPC function pointer for the callback's lifetime.
  cb->fun = fun_sv->u.fp;
  cb->fun->hdr.ref++;

  int id = g_next_handle++;
  g_callbacks[id] = cb.release();
  pop_n_elems(3);
  push_number(id);
}
#endif

#ifdef F_FFI_CALLBACK_ADDR
void f_ffi_callback_addr() {
  int id = sp->u.number;
  auto it = g_callbacks.find(id);
  if (it == g_callbacks.end()) {
    error("ffi_callback_addr: invalid callback handle %d\n", id);
  }
  LPC_INT addr = reinterpret_cast<LPC_INT>(it->second->code);
  pop_stack();
  push_number(addr);
}
#endif

#ifdef F_FFI_CALLBACK_FREE
void f_ffi_callback_free() {
  int id = sp->u.number;
  auto it = g_callbacks.find(id);
  if (it != g_callbacks.end()) {
    if (it->second->dispatching) {
      // free_callback() would ffi_closure_free(closure) -- unmapping the
      // executable trampoline libffi is CURRENTLY EXECUTING to have
      // reached this call at all (a callback freeing its own id from
      // inside its own dispatch). That crashes inside libffi itself, not
      // just on the next read of the (also freed) FfiCallback struct.
      error("ffi_callback_free: callback %d cannot free itself while dispatching.\n", id);
    }
    free_callback(it->second);
    g_callbacks.erase(it);
  }
  pop_stack();
}
#endif

#ifdef F_FFI_ERROR
void f_ffi_error() {
  const char* msg = g_last_error.c_str();
  push_malloced_string(string_copy(msg, "f_ffi_error"));
}
#endif

#ifdef F_FFI_STATUS
void f_ffi_status() {
  mapping_t* m = allocate_mapping(4);
  add_mapping_pair(m, "libraries", static_cast<long>(g_libs.size()));
  add_mapping_pair(m, "functions", static_cast<long>(g_funcs.size()));
  add_mapping_pair(m, "callbacks", static_cast<long>(g_callbacks.size()));
  push_refed_mapping(m);
}
#endif

// ===========================================================================
// lifecycle + DEBUGMALLOC
// ===========================================================================

void ffi_cleanup() {
  for (auto& kv : g_callbacks) {
    free_callback(kv.second);
  }
  g_callbacks.clear();
  for (auto& kv : g_funcs) {
    delete kv.second;
  }
  g_funcs.clear();
  for (auto& kv : g_libs) {
    plat_dlclose(kv.second->handle);
    delete kv.second;
  }
  g_libs.clear();
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_ffi() {
  // Allocations are buffers (GC-tracked already). Mark the callback
  // funptrs the tables hold a ref on.
  for (auto& kv : g_callbacks) {
    if (kv.second->fun) {
      kv.second->fun->hdr.extra_ref++;
    }
  }
}
#endif

#endif /* PACKAGE_FFI */
