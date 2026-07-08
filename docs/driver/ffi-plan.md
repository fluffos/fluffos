---
layout: doc
title: driver / FFI package plan
---
# `package_ffi` — a foreign-function interface for LPC

Status: **design / plan**. This document is the proposal for a new
`ffi` package that lets LPC load native shared libraries
(`.so`/`.dll`/`.dylib`), call their functions, manage native memory,
and pass in/out parameters — plus a `tools/ffi` generator that turns a
C header into ready-to-use LPC bindings.

It follows the existing package conventions (`src/packages/db` is the
closest sibling: a `.spec`, a `.cc`, a `CMakeLists.txt`, an
`option(PACKAGE_FFI ...)` toggle, and a handle table into an opaque
native resource).

> [!CAUTION]
> An FFI is a **full sandbox escape**: any mudlib code that reaches
> these efuns can call arbitrary native code and read/write arbitrary
> memory. The security model (below) is not optional — the package is
> `OFF` by default, every call is gated through a master apply, and the
> recommendation is to wrap the raw efuns in a privileged simul-efun
> that ordinary mudlib code cannot reach.

---

## 1. Scope

In scope:

- Load/unload a shared library by path; resolve a symbol.
- Describe a C function's signature (argument + return types) and call
  it, marshalling LPC values ↔ C values.
- Allocate/free/read/write native memory blocks for buffers, structs,
  and out-parameters.
- Read/write C structs by field layout.
- A `tools/ffi` generator: parse a C header, emit an LPC binding object
  (`.lpc`) plus a struct-layout include.

Explicitly out of scope (at least v1):

- Native code calling *into* LPC (callbacks from C to LPC) — deferred to
  a v2 "closure trampoline" section; libffi supports it
  (`ffi_closure`) but it is a large surface.
- C++ name mangling / methods — C ABI only.
- Varargs C functions beyond a fixed described prototype.

## 2. Implementation library: **libffi**

Use **libffi** (the same library CPython, Ruby, LuaJIT, and Node's
ffi-napi use). Rationale:

- It is the mature, portable answer to "call a function whose signature
  is only known at runtime": it builds the platform call frame
  (`ffi_prep_cif` + `ffi_call`) for x86-64 SysV, Windows x64, ARM64,
  etc., which is exactly the hard part we must not hand-roll per
  architecture.
- It is already an apt/brew/pacman package on every CI platform
  (`libffi-dev` / `libffi`), matching how `package_db` finds MySQL etc.
- Dynamic loading itself is `dlopen`/`dlsym` on POSIX and
  `LoadLibrary`/`GetProcAddress` on Windows — a thin `#ifdef _WIN32`
  wrapper (the tree already vendors this shape in
  `thirdparty/libwebsockets/.../*-plugins.c`).

CMake (`src/packages/ffi/CMakeLists.txt`), mirroring `package_db`:

```cmake
if(${PACKAGE_FFI})
  add_library(package_ffi STATIC "ffi.cc" "ffi.h")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FFI REQUIRED libffi)
  target_include_directories(package_ffi SYSTEM PUBLIC ${FFI_INCLUDE_DIRS})
  target_link_libraries(package_ffi PRIVATE ${FFI_LIBRARIES})
  # POSIX: also link libdl where dlopen is separate.
  if(NOT WIN32)
    target_link_libraries(package_ffi PRIVATE ${CMAKE_DL_LIBS})
  endif()
endif()
```

`option(PACKAGE_FFI "foreign function interface package" OFF)` — **off
by default**, unlike the other packages.

## 3. LPC-visible types and the type-tag question

There is **no new svalue type**. The FFI surface is expressed entirely
with existing LPC types, which keeps the VM, the GC, `save_object`, and
every existing efun untouched:

- **library handle**, **function handle** → `int` (index into a
  driver-side handle table, exactly like `db_connect`'s int handle).
- **native pointer / allocation** → `buffer`. A `buffer` is already a
  ref-counted byte blob with a size; an FFI allocation is a `buffer`
  whose bytes *are* the native memory. Reading an out-param is then
  just indexing the buffer, and existing efuns
  (`string_encode`/`string_decode`, `read_buffer`/`write_buffer`)
  already marshal buffers.
- **C numeric argument/return** → LPC `int` or `float`.
- **C string argument** → LPC `string` (marshalled to a NUL-terminated
  copy for the duration of the call).
- **a described signature** → a small LPC array/mapping of type codes
  (see §4), not a native object.

This is deliberately the same decision the `decimal` proposal makes in
reverse: FFI *avoids* a new tag because it can, whereas `decimal`
*needs* one. (If a future v2 wants first-class typed native pointers
with automatic freeing, `T_BUFFER` with a subtype flag is the
extension point — but v1 does not need it.)

### Type codes

A compact integer enum shared between the C side and an LPC header
`ffi.h` (shipped in `src/include/`, like `socket.h`):

```c
#define FFI_VOID    0
#define FFI_INT8    1
#define FFI_UINT8   2
#define FFI_INT16   3
#define FFI_UINT16  4
#define FFI_INT32   5
#define FFI_UINT32  6
#define FFI_INT64   7
#define FFI_UINT64  8
#define FFI_FLOAT   9
#define FFI_DOUBLE  10
#define FFI_POINTER 11   /* buffer <-> void*        */
#define FFI_STRING  12   /* string  <-> const char* */
```

Each maps to an `ffi_type` (`ffi_type_sint32`, `ffi_type_pointer`, …).

## 4. The efun surface (`ffi.spec`)

```c
/* --- library & symbol --------------------------------------------- */
int    ffi_load(string path);           /* -> library handle, or 0 + error */
void   ffi_unload(int lib);
int    ffi_symbol(int lib, string name); /* raw code address as a handle    */

/* --- describe & call ---------------------------------------------- */
/* ret_type is a type code; arg_types is an array of type codes.
 * Returns a callable function handle. */
int    ffi_prepare(int lib, string name, int ret_type, int *arg_types);
/* Call: args are LPC values matching the prepared arg_types; the return
 * is marshalled per ret_type (int/float/buffer/string/0 for void). */
mixed  ffi_call(int func, mixed *args);

/* --- native memory ------------------------------------------------ */
buffer ffi_alloc(int nbytes);            /* zeroed native block as a buffer */
void   ffi_free(buffer mem);             /* explicit free (also GC'd)       */
int    ffi_sizeof(int type_code);        /* platform size of a scalar type  */

/* --- typed peek/poke into a buffer at an offset ------------------- */
mixed  ffi_read(buffer mem, int offset, int type_code);
void   ffi_write(buffer mem, int offset, int type_code, mixed value);

/* --- struct layout ------------------------------------------------ */
/* field_types -> ({ total_size, ({ off0, off1, ... }) }) honoring
 * platform alignment; the tools/ffi generator emits these. */
mixed *ffi_struct_layout(int *field_types);

/* --- introspection ------------------------------------------------ */
string ffi_error();                      /* last error on this thread       */
mixed *ffi_status();                      /* open libs/funcs/allocations     */
```

Notes:

- `ffi_call` is the one variadic-in-LPC efun; it validates
  `sizeof(args)` against the prepared arg-type count and each arg's LPC
  type against its code before touching libffi, erroring (not
  crashing) on a mismatch — the lesson already learned this cycle in
  `socket_create` (validate the LPC boundary *before* the native
  conversion).
- Out-parameters: pass an `ffi_alloc`'d `buffer` where the C function
  wants `T*`; the native code writes into it; read it back with
  `ffi_read`. In/out is the same buffer written before the call and
  read after.

## 5. Driver-side design (`ffi.cc`)

- **Handle tables** (three `std::unordered_map<int, ...>` or the same
  `find_db_conn`-style scheme `package_db` uses): open libraries,
  prepared functions (each owns its `ffi_cif` + `ffi_type*` array),
  live allocations.
- **Allocations as buffers**: `ffi_alloc(n)` returns a `buffer_t` of
  size `n`; the bytes are the native storage (no separate pointer). For
  `FFI_POINTER` args, the native pointer passed to C is
  `&buf->item[0]`. This means the GC already tracks the lifetime — an
  allocation lives as long as an LPC value references the buffer.
- **Marshalling** (`ffi_call`): build an `void *avalues[]` from the LPC
  args per the prepared type codes into a scratch arena
  (`scratchpad.h`), `ffi_call`, then convert the return slot back to an
  svalue. Strings are copied to NUL-terminated scratch buffers for the
  call's duration only.
- **Error handling**: every failure path is `error()`, never a crash;
  the caution in AGENTS.md §4 (longjmp leaks) means all transient native
  buffers for a call are arena/`unique_ptr` owned so an `error()`
  mid-marshal leaks nothing.
- **DEBUGMALLOC**: the handle tables and any non-buffer native blocks
  get a `TAG_FFI` and a `mark_ffi_handles()` in `check_all_blocks`
  (this cycle proved that any driver-held live state needs a mark hook —
  see the pending-DNS and replace_program fixes).

## 6. Security model (mandatory)

1. `PACKAGE_FFI` is **OFF** by default; a mud opts in at build time.
2. Every `ffi_load` and every `ffi_prepare` calls a master apply
   **`valid_ffi(string operation, mixed arg, object caller)`**
   (mirroring `valid_database`, `valid_link`, `get_include_path`): the
   mudlib decides which libraries and symbols are permitted, keyed on
   the calling object. Default master returns 0 → denied.
3. An allow-list of library paths in the config file
   (`ffi allowed libraries : /usr/lib/libm.so.6:...`) that
   `valid_ffi` can consult.
4. Documentation strongly recommends wrapping the raw efuns in a single
   privileged simul-efun (`efun::ffi_*`) so ordinary objects never call
   the raw efuns directly.

## 7. `tools/ffi` — the bindings generator

A dependency-free generator under `tools/ffi/` (same spirit as
`tools/lpc-syntax`), invoked as:

```bash
tools/ffi/generate.py <header.h> --lib libfoo.so --out mudlib/std/foo
```

It emits two files:

- `foo.lpc` — one LPC wrapper function per exported C function: it
  `ffi_prepare`s the signature once (lazily, cached in a global) and
  `ffi_call`s it, converting LPC args. For a C `double sqrt(double)` it
  emits `float sqrt(float x) { return ffi_call(_h_sqrt(), ({ x })); }`.
- `foo_structs.h` — `#define`s for each struct's `ffi_struct_layout`
  field-type array and named field offsets, so LPC code reads
  `buf[STRUCT_FOO_field_x .. ]` symbolically.

Parser strategy: reuse the C-declaration subset we already understand —
a small hand-written tokenizer (the `tools/lpc-syntax` tokenizer is the
model) over `extern` function prototypes and `struct { ... }` bodies,
mapping C types to the §3 type codes. Anything it cannot map (function
pointers, unions, bitfields in v1) is reported and skipped with a
warning, never silently mis-bound. A `--emit-json` mode dumps the
parsed signature table so the binding step and tests share one source
of truth, exactly like `lpc-grammar.json`.

## 8. Testing

- **GTest** (`src/tests/test_ffi.cc`): prepare and call functions in a
  tiny fixture `.so` built by the test CMake (e.g. `int add(int,int)`,
  `double scale(double)`, `void fill(char*, int, int)` for out-params,
  a struct round-trip) — pure C-ABI round trips with no mudlib.
- **LPC** (`testsuite/single/tests/efuns/ffi_*.lpc`, guarded by
  `#ifdef __PACKAGE_FFI__` like the other optional packages): load
  `libm`, call `sqrt`/`pow`, allocate a buffer, write/read a struct,
  and assert `valid_ffi` denial is enforced. The suite's per-file
  `check_memory()` gate (which caught seven leaks this cycle) validates
  the allocation/handle accounting.
- `tools/ffi` gets a `test.mjs`/`test.py` over a sample header with
  golden expected LPC output, run in CI like `tools/lpc-syntax/test.mjs`.

## 9. Phasing

1. **v1a**: `ffi_load`/`ffi_symbol`/`ffi_prepare`/`ffi_call` for scalar
   + pointer + string args, `ffi_alloc`/`ffi_free`/`ffi_read`/
   `ffi_write`, `valid_ffi`, the `OFF` toggle, GTest + LPC tests.
2. **v1b**: `ffi_struct_layout` + the `tools/ffi` generator.
3. **v2**: C-calls-LPC callbacks via `ffi_closure` (a separate, larger
   design — the trampoline must enter the VM safely from a native
   thread/stack).

Each phase is a self-contained, independently testable PR.
