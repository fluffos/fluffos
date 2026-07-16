---
title: driver / FFI package
---
# `package_ffi` — a foreign-function interface for LPC

Status: **implemented**. This document is both the design and the
reference for the `ffi` package, which lets LPC load native shared
libraries (`.so`/`.dll`/`.dylib`), call their functions, manage native
memory, pass in/out parameters, and expose LPC function pointers to C as
callbacks — plus a `tools/ffi` generator that turns a C header into
ready-to-use LPC bindings. A [worked-examples section](#6-worked-examples--calling-libc)
walks through real calls into libc; every example is pinned by the
driver testsuite (`testsuite/single/tests/efuns/ffi_doc_examples.lpc`).

**What shipped** (all phases): `src/packages/ffi/` (spec/`.cc`/`.h`/
CMake, `option(PACKAGE_FFI ... ON)`, libffi), the LPC header
`src/include/ffi.h`, the `valid_ffi` master apply + the `ffi allowed
libraries` config allow-list, callbacks via libffi closures, the
`__PACKAGE_FFI__` predefine, DEBUGMALLOC marking + shutdown cleanup, the
`tools/ffi/generate.py` bindings generator with tests, and the driver
testsuite (`testsuite/single/tests/efuns/ffi_*.lpc`, 21 files) which is
the primary test surface — the efuns are VM-stack-based and libffi's
`ffi_call`/closure paths run there end-to-end under ASan/UBSan and the
per-file `check_memory()` leak gate. The notes below double as the
reference; a few names differ slightly from the original sketch (see
`ffi.spec`).

It follows the existing package conventions (`src/packages/db` is the
closest sibling: a `.spec`, a `.cc`, a `CMakeLists.txt`, an
`option(PACKAGE_FFI ...)` toggle, and a handle table into an opaque
native resource).

:::danger[Full sandbox escape]
An FFI is a **full sandbox escape**: any mudlib code that reaches
these efuns can call arbitrary native code and read/write arbitrary
memory. The security model (below) is not optional — every load,
prepare, and callback is gated through the `valid_ffi` master apply
(**a master that does not implement it denies everything**), and the
recommendation is to wrap the raw efuns in a privileged simul-efun
that ordinary mudlib code cannot reach. A mud that wants no native
code at all builds with `-DPACKAGE_FFI=OFF`.
:::

---

## 1. Scope

In scope:

- Load/unload a shared library by path; resolve a symbol.
- Describe a C function's signature (argument + return types) and call
  it, marshalling LPC values ↔ C values.
- Allocate/free/read/write native memory blocks for buffers, structs,
  and out-parameters.
- Read/write C structs by field layout.
- Native code calling *into* LPC: an LPC function pointer is wrapped in a
  libffi closure (`ffi_closure`) and handed to C as a raw callable
  address, so C libraries with callback parameters (comparators, event
  handlers) can invoke LPC.
- A `tools/ffi` generator: parse a C header, emit an LPC binding object
  (`.lpc`) plus a struct-layout include.

Explicitly out of scope (at least v1):

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

## 3. LPC-visible types and the type-tag question

There is **no new svalue type**. The FFI surface is expressed entirely
with existing LPC types, which keeps the VM, the GC, `save_object`, and
every existing efun untouched:

- **library handle**, **function handle** → `int` (index into a
  driver-side handle table, exactly like `db_connect`'s int handle).
- **native pointer / allocation** → [`buffer`](../lpc/types/buffer). A
  `buffer` is already a ref-counted byte blob with a size; an FFI
  allocation is a `buffer` whose bytes *are* the native memory. Reading
  an out-param is then just indexing the buffer.
- **C numeric argument/return** → LPC `int` or `float`.
- **all pointer/data across the call boundary** → LPC `buffer`, never
  `string`. See "Strings are UTF-8; the boundary is bytes" below. A
  *raw foreign* pointer (an address returned by C that LPC does not
  own) is an `int`; `0` passed where C wants a pointer is `NULL`.
- **a described signature** → a small LPC array of type codes (see
  §4), not a native object.

### The buffer type does the heavy lifting

The [`buffer`](../lpc/types/buffer) language features are exactly the
toolkit an FFI needs, and the API leans on all of them:

- **Byte lvalues** — `buf[i]` reads a byte as an int 0..255, and `=`,
  `++`, `--`, `+=`, `-=` write it back with a range check. Patching a
  single byte of native memory (NUL-terminating a C string in place,
  poking a flag field) is one assignment; no `ffi_write` needed for
  byte-sized fields.
- **Range assignment** — `buf[0..4] = "hello"` writes the string's raw
  UTF-8 bytes into an allocation; an int-array right-hand side writes
  one byte per element. The buffer resizes when the lengths differ, so
  building a wire payload in place is natural.
- **Promotion** — strings and arrays of ints 0..255 convert to buffers
  in declarations, assignments, `+` and `+=` (`buffer b = "abc"`,
  `b += ({ 0 })`). This is what makes the explicit
  string-to-C-string idiom below a two-liner. The explicit spelling is
  the [to_buffer](../efun/buffers/to_buffer) efun.
- **Concatenation** — `buf1 + buf2` / `buf += part` assemble larger
  native payloads from pieces.
- **foreach** — buffers iterate like arrays (each byte an int 0..255),
  so scanning bytes C wrote — checksums, delimiter splits — is a plain
  loop over the result of `ffi_peek()`.

### Strings are UTF-8; the boundary is bytes

**LPC strings are UTF-8-native**, so the FFI layer must **never**
implicitly marshal a `string` to a C `char*`. Doing so would silently
impose UTF-8 on a C function that may expect Latin-1, UTF-16, a
specific code page, or raw bytes — and LPC strings cannot faithfully
carry an arbitrary byte a C API might return (e.g. a lone `0x80`, or an
embedded NUL). The encoding boundary has to be **explicit and visible
in the LPC source**, exactly as the rest of the driver already does it
with `string_encode`/`string_decode`.

Therefore **`buffer` is the currency for every pointer and every byte
payload** at the FFI boundary. To call a C function that wants
`const char*`, the LPC caller converts to bytes and NUL-terminates
first — with buffer promotion that is:

```c
buffer cstr(string s) {
    buffer b = s;        // promotes: the string's raw UTF-8 bytes
    b += ({ 0 });        // NUL terminator
    return b;
}

ffi_call(_h_puts, ({ cstr("café") }));   // 5 bytes + NUL cross the boundary
```

(Promotion always yields UTF-8 bytes because that is the string's
in-memory encoding. For any *other* target encoding, spell it with
`string_encode(s, "latin-1")` etc. — the promotion shortcut and the
explicit call are interchangeable for UTF-8.)

To read a `char*` result, the caller decodes the bytes it copied out:
`string_decode(ffi_peek(addr, -1), "utf-8")`. No efun on the call path
takes or returns a `string` for foreign data — and note that promotion
does **not** reach inside `ffi_call`'s argument array: the args are
`mixed *`, so a bare string there is an error, never an implicit
conversion. The boundary stays visible at every call site.
(Driver-mediated *identifiers* — the library path in `ffi_load`, the
symbol name in `ffi_symbol`/`ffi_prepare` — stay `string`, consistent
with every file efun: the driver makes its own NUL-terminated copy for
`dlopen`/`dlsym`; they are never handed to a foreign function.)

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
#define FFI_FLOAT   9    /* C float  <-> LPC float */
#define FFI_DOUBLE  10   /* C double <-> LPC float */
#define FFI_POINTER 11   /* arg: buffer (its bytes) or int address; return: int address */

/* Convenience aliases for the native C int/long widths. */
#define FFI_INT   FFI_INT32
#define FFI_UINT  FFI_UINT32
#define FFI_LONG  FFI_INT64
#define FFI_ULONG FFI_UINT64
```

Each maps to an `ffi_type` (`ffi_type_sint32`, `ffi_type_pointer`, …).
There is **no** `FFI_STRING` code: a C `char*` is just an `FFI_POINTER`
whose buffer the caller filled with encoded, NUL-terminated bytes.

## 4. The efun surface (`ffi.spec`)

```c
/* --- library & symbol --------------------------------------------- */
int    ffi_load(string path);            /* -> library handle; "" = the
                                            driver process itself (libc/libm) */
void   ffi_unload(int lib);
int    ffi_symbol(int lib, string name); /* raw code address as an int      */

/* --- describe & call ---------------------------------------------- */
/* ret_type is a type code; arg_types is an array of type codes.
 * Returns a callable function handle. */
int    ffi_prepare(int lib, string name, int ret_type, int *arg_types);
/* Call: each arg is int | float | buffer (NEVER string), matching the
 * prepared arg_types. An FFI_POINTER arg takes a buffer (its bytes are
 * the pointee) or an int address (0 = NULL). Return per ret_type:
 * int/float for scalars, an int address for FFI_POINTER, 0 for void. */
mixed  ffi_call(int func, mixed *args);

/* --- native memory ------------------------------------------------ */
buffer ffi_alloc(int nbytes);            /* zeroed native block as a buffer */
void   ffi_free(buffer mem);             /* zero the bytes now; the block
                                            itself is GC'd like any buffer  */
int    ffi_sizeof(int type_code);        /* platform size of a scalar type  */
int    ffi_address(buffer mem);          /* address of the buffer's bytes
                                            (0 for an empty buffer)         */
/* Copy nbytes from a raw foreign address (e.g. a char* returned by a C
 * function) into an owned buffer -- the only way foreign bytes become an
 * LPC value. Pass -1 to read a NUL-terminated C string (capped at the
 * "maximum buffer size" config). Then string_decode() it if it is text.
 * Gated by valid_ffi("peek", address, caller): an arbitrary-address
 * read is a disclosure primitive even though no foreign code runs. */
buffer ffi_peek(int address, int nbytes);

/* --- typed peek/poke into a buffer at an offset ------------------- */
mixed  ffi_read(buffer mem, int offset, int type_code);
void   ffi_write(buffer mem, int offset, int type_code, mixed value);

/* --- struct layout ------------------------------------------------ */
/* field_types -> ({ total_size, ({ off0, off1, ... }) }) honoring
 * platform alignment; the tools/ffi generator emits these. */
mixed *ffi_struct_layout(int *field_types);

/* --- callbacks: expose an LPC function to C ----------------------- */
/* Wrap an LPC function pointer in a libffi closure with the given
 * return + argument type codes. Returns a callback handle. */
int    ffi_callback(function fn, int ret_type, int *arg_types);
/* Raw code address of the closure, to pass to C as an FFI_POINTER. */
int    ffi_callback_addr(int cb);
/* Release the closure (also GC'd). */
void   ffi_callback_free(int cb);

/* --- introspection ------------------------------------------------ */
string ffi_error();                      /* last error on this thread       */
mapping ffi_status();                    /* counts: libraries/functions/callbacks */
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
  `ffi_read` (or plain `buf[i]` indexing for single bytes). In/out is
  the same buffer written before the call and read after. A C `char*`
  result is a foreign address (`int`); copy it into a buffer with
  `ffi_peek` and `string_decode` if it is text — strings never cross
  implicitly.
- `ffi_read`/`ffi_write` exist for the *multi-byte* scalars, where
  width, alignment, and endianness matter. For single bytes they
  overlap with the buffer's own `buf[i]` lvalues — use whichever reads
  better; `buf[i]` range-checks 0..255 while
  `ffi_write(buf, i, FFI_INT8, v)` truncates to the C type like a C
  assignment would.

## 5. Driver-side design (`ffi.cc`)

- **Handle tables** (three `std::unordered_map<int, ...>` or the same
  `find_db_conn`-style scheme `package_db` uses): open libraries,
  prepared functions (each owns its `ffi_cif` + `ffi_type*` array),
  live callbacks.
- **Allocations as buffers**: `ffi_alloc(n)` returns a `buffer_t` of
  size `n`; the bytes are the native storage (no separate pointer). For
  `FFI_POINTER` args, the native pointer passed to C is
  `&buf->item[0]`. This means the GC already tracks the lifetime — an
  allocation lives as long as an LPC value references the buffer.
  `ffi_free` just zeroes the bytes immediately (so a dangling native
  reader sees no stale data); the block itself is reclaimed by
  ref-counting like any other buffer.
- **Marshalling** (`ffi_call`): build a `void *avalues[]` from the LPC
  args per the prepared type codes, `ffi_call`, then convert the return
  slot back to an svalue. Only `int`/`float`/`buffer` args are
  accepted; a `buffer` argument passes `&buf->item[0]` as the pointer
  (its own bytes, whatever encoding the caller put there), an `int`
  argument to an `FFI_POINTER` slot passes the literal address — the
  driver does no encoding conversion, so no NUL-terminated string
  copies exist on this path.
- **Error handling**: every failure path is `error()`, never a crash;
  the caution in AGENTS.md §4 (error-unwind leaks) means all transient
  native buffers for a call are arena/`unique_ptr` owned so an
  `error()` mid-marshal leaks nothing.
- **DEBUGMALLOC**: no dedicated FFI tag or `check_all_blocks` mark hook
  is needed. The handle tables are `std::unordered_map`s of plain C++
  heap objects (`FfiLibrary`/`FfiFunc`/`FfiCallback`), invisible to the
  driver's tagged-allocation accounting; the only LPC-heap storage the
  package hands out is `ffi_alloc()` buffers, which the GC already
  tracks as ordinary `TAG_BUFFER` blocks.

## 6. Worked examples — calling libc

Everything below runs against the C library that is already linked into
the driver process: `ffi_load("")` is the portable spelling for "the
process's own global symbols" (`dlopen(NULL)` on POSIX,
`GetModuleHandle(NULL)` on Windows), so no platform-specific library
file name is needed to reach `sqrt`, `strlen`, `qsort`, and friends.
Each snippet is pinned by
`testsuite/single/tests/efuns/ffi_doc_examples.lpc`, which runs in CI
on every platform.

All the examples assume:

```c
#include <ffi.h>

int lib = ffi_load("");     // libc/libm via the driver's own symbols
```

and the two-line helper from §3 for C strings:

```c
buffer cstr(string s) {
    buffer b = s;        // promotes: the string's raw UTF-8 bytes
    b += ({ 0 });        // NUL terminator
    return b;
}
```

(`cstr()` and the rest of the boundary boilerplate — reading returned
`char*`s, out-parameter blocks, argv arrays, struct-field access — ship
as the pure-LPC library [`/std/ffi_util`](../stdlib/ffi_util) in the
testsuite mudlib, portable to any mudlib.)

### Scalars: `sqrt`, `pow`, `abs`

Numeric arguments and returns marshal directly between LPC `int`/`float`
and the described C type:

```c
int f = ffi_prepare(lib, "sqrt", FFI_DOUBLE, ({ FFI_DOUBLE }));
ffi_call(f, ({ 9.0 }));                       // -> 3.0

int p = ffi_prepare(lib, "pow", FFI_DOUBLE, ({ FFI_DOUBLE, FFI_DOUBLE }));
ffi_call(p, ({ 2.0, 3.0 }));                  // -> 8.0

int a = ffi_prepare(lib, "abs", FFI_INT, ({ FFI_INT }));
ffi_call(a, ({ -42 }));                       // -> 42
```

### Strings into C: `strlen`

A C `char*` parameter is an `FFI_POINTER`; the caller passes a buffer
of encoded, NUL-terminated bytes:

```c
int sl = ffi_prepare(lib, "strlen", FFI_UINT64, ({ FFI_POINTER }));

ffi_call(sl, ({ cstr("hello") }));            // -> 5
ffi_call(sl, ({ cstr("café") }));             // -> 5: é is two UTF-8 bytes

ffi_call(sl, ({ "hello" }));                  // ERROR -- a string never
                                              // crosses implicitly
```

Buffer range assignment is the other way to lay text into native
memory — here filling a zeroed allocation, whose trailing byte is the
NUL:

```c
buffer buf = ffi_alloc(6);    // 6 zero bytes
buf[0..4] = "hello";          // the string's 5 raw UTF-8 bytes
ffi_call(sl, ({ buf }));      // -> 5 (byte 5 is still 0)
```

### Strings out of C: `getenv`

A returned `char*` arrives as a raw address (an `int`). Check it for
`NULL`, copy the bytes out with `ffi_peek(addr, -1)` (reads up to the
NUL), and decode:

```c
int ge = ffi_prepare(lib, "getenv", FFI_POINTER, ({ FFI_POINTER }));

int addr = ffi_call(ge, ({ cstr("PATH") }));
if (addr) {
    string path = string_decode(ffi_peek(addr, -1), "utf-8");
    write(path + "\n");
}
// An unset variable returns 0 (NULL) -- never ffi_peek() address 0.
```

### Out-parameters: `frexp`

Where C wants `T *out`, pass an `ffi_alloc`'d buffer sized for `T` and
read it back after the call:

```c
// double frexp(double value, int *exp);
int fx = ffi_prepare(lib, "frexp", FFI_DOUBLE, ({ FFI_DOUBLE, FFI_POINTER }));

buffer exp = ffi_alloc(ffi_sizeof(FFI_INT));
float mant = ffi_call(fx, ({ 8.0, exp }));    // -> 0.5
int e = ffi_read(exp, 0, FFI_INT);            // -> 4 (8.0 == 0.5 * 2^4)
```

### Structs: `time` + `localtime`

`ffi_struct_layout` turns a field-type array into
`({ total_size, ({ offsets... }) })` honoring platform alignment, so
LPC never hard-codes offsets. `struct tm` leads with nine `int` fields
(`tm_sec`, `tm_min`, `tm_hour`, `tm_mday`, `tm_mon`, `tm_year`, ...):

```c
// time_t time(time_t *tloc);           -- 0 as the arg is NULL
int tf = ffi_prepare(lib, "time", FFI_INT64, ({ FFI_POINTER }));
int now = ffi_call(tf, ({ 0 }));

// struct tm *localtime(const time_t *timep);
int lt = ffi_prepare(lib, "localtime", FFI_POINTER, ({ FFI_POINTER }));
buffer timep = ffi_alloc(ffi_sizeof(FFI_LONG));
ffi_write(timep, 0, FFI_INT64, now);
int addr = ffi_call(lt, ({ timep }));

mixed *lay = ffi_struct_layout(({ FFI_INT, FFI_INT, FFI_INT, FFI_INT,
                                  FFI_INT, FFI_INT, FFI_INT, FFI_INT,
                                  FFI_INT }));
buffer tm = ffi_peek(addr, lay[0]);           // copy the struct's bytes
int year = 1900 + ffi_read(tm, lay[1][5], FFI_INT);   // tm_year
int mon  = 1 + ffi_read(tm, lay[1][4], FFI_INT);      // tm_mon
```

(The `tools/ffi` generator emits these field-type arrays and symbolic
per-field indexes from the C header, so real bindings say
`lay[1][STRUCT_TM_tm_year]` instead of a bare `5` — see §8.)

### Callbacks: `qsort` with an LPC comparator

An LPC function pointer becomes a C function pointer via a libffi
closure; C calls back into the VM on every comparison:

```c
// The comparator receives two const void* -- here int* addresses.
int cmp_ints(int a_addr, int b_addr) {
    return ffi_read(ffi_peek(a_addr, 4), 0, FFI_INT) -
           ffi_read(ffi_peek(b_addr, 4), 0, FFI_INT);
}

// void qsort(void *base, size_t nmemb, size_t size,
//            int (*compar)(const void *, const void *));
int qs = ffi_prepare(lib, "qsort", FFI_VOID,
                     ({ FFI_POINTER, FFI_UINT64, FFI_UINT64, FFI_POINTER }));

int *vals = ({ 30, 10, 40, 20, 50 });
buffer data = ffi_alloc(sizeof(vals) * ffi_sizeof(FFI_INT));
for (int i = 0; i < sizeof(vals); i++)
    ffi_write(data, i * 4, FFI_INT, vals[i]);

int cb = ffi_callback((: cmp_ints :), FFI_INT, ({ FFI_POINTER, FFI_POINTER }));
ffi_call(qs, ({ data, sizeof(vals), ffi_sizeof(FFI_INT),
                ffi_callback_addr(cb) }));
// data now holds 10, 20, 30, 40, 50
ffi_callback_free(cb);
```

An LPC error inside the callback is caught at the closure boundary
(`safe_call_function_pointer`) and yields a zero/NULL C return — it
never unwinds across the native stack.

### Working with the returned bytes

Buffers iterate like arrays, so post-processing what C wrote is plain
LPC — no `ffi_read` loop needed for byte-wise work:

```c
int sum = 0;
foreach (int byte in ffi_peek(addr, -1))      // checksum a C string
    sum += byte;

buffer name = ffi_peek(addr, -1);
name[0] = 'x';                // patch a single byte (range-checked 0..255)
```

Remember to `ffi_unload(lib)` when a binding object is done with the
library (the testsuite does it per test; a real binding keeps the
handle for the object's lifetime).

## 7. Security model (mandatory)

1. The build ships with `PACKAGE_FFI` on (like the other packages), but
   the runtime default is **deny**: without a `valid_ffi` apply in the
   master object, every load/prepare/callback errors. A mud that wants
   the efuns gone entirely builds with `-DPACKAGE_FFI=OFF` (the WASM
   target forces it off — there is no `dlopen` in the browser).
2. `ffi_load`, `ffi_symbol`, `ffi_prepare`, `ffi_callback`, and
   `ffi_peek` each call a master apply
   **`valid_ffi(string operation, mixed arg, object caller)`**
   (mirroring `valid_database`, `valid_link`, `get_include_path`): the
   mudlib decides which libraries and symbols are permitted, keyed on
   the calling object. Default master (no apply) returns 0 → denied.
   The operation is `"load"` (arg: the library path), `"symbol"` /
   `"prepare"` (arg: the symbol name), `"peek"` (arg: the address —
   gated because reading arbitrary process memory is a disclosure
   primitive on its own, no foreign code required), or `"callback"`
   (arg: 0). The other efuns need no gate: they operate on LPC-owned
   buffers and handles only (`ffi_address` reveals a buffer's own
   address, which is inert without a peek/call grant). A restrictive
   implementation:

   ```c
   // master.c -- only one privileged daemon may reach native code.
   int valid_ffi(string op, mixed arg, object caller) {
       if (base_name(caller) != "/daemon/ffi") return 0;
       if (op == "load")
           return member_array(arg, ({ "" })) != -1;   // driver's own libc only
       return 1;   // symbol/prepare/callback/peek by the daemon are fine
   }
   ```

3. An allow-list of library paths in the config file
   (`ffi allowed libraries : /usr/lib/libm.so.6:...`, colon-separated)
   is enforced by the driver *before* `valid_ffi` runs; when the option
   is unset, the master apply alone decides.
4. Documentation strongly recommends wrapping the raw efuns in a single
   privileged simul-efun (`efun::ffi_*`) so ordinary objects never call
   the raw efuns directly.

## 8. `tools/ffi` — the bindings generator

A dependency-free generator under `tools/ffi/` (same spirit as
`tools/lpc-syntax`), invoked as:

```bash
tools/ffi/generate.py <header.h> --lib libfoo.so --out mudlib/std/foo
```

It emits two files:

- `foo.lpc` — one LPC wrapper function per exported C function: it
  `ffi_prepare`s the signature once (lazily, cached in a mapping) and
  `ffi_call`s it. For a C `double sqrt(double)` it emits
  `float sqrt(float a0) { return ffi_call(_h_sqrt(), ({ a0 })); }`. A C
  `char*`/`const char*` parameter is emitted as a **`buffer`** (never a
  `string`), honoring the UTF-8 boundary rule of §3 — the caller passes
  encoded, NUL-terminated bytes (the `cstr()` idiom).
  `--string-convenience` additionally emits a clearly-named overload
  (e.g. `puts_s(string)`) that converts to a NUL-terminated UTF-8
  buffer for the common ASCII/UTF-8 case, but the raw binding is always
  the buffer form so the encoding is never hidden by default.
- `foo_structs.h` — `#define`s for each struct's `ffi_struct_layout`
  field-type array and named field indexes, so LPC code reads
  `lay[1][STRUCT_FOO_field_x]` symbolically.

Parser strategy: reuse the C-declaration subset we already understand —
a small hand-written tokenizer (the `tools/lpc-syntax` tokenizer is the
model) over `extern` function prototypes and `struct { ... }` bodies,
mapping C types to the §3 type codes. Anything it cannot map (function
pointers, unions, bitfields in v1) is reported and skipped with a
warning, never silently mis-bound. A `--emit-json` mode dumps the
parsed signature table so the binding step and tests share one source
of truth, exactly like `lpc-grammar.json`.

## 9. Testing

- **LPC** (`testsuite/single/tests/efuns/ffi_*.lpc`, 21 files, guarded
  by `#ifdef __PACKAGE_FFI__` like the other optional packages): the FFI
  surface is exercised entirely through the LPC testsuite rather than a
  GTest fixture, since every path needs a live VM (for `valid_ffi` and,
  for callbacks, VM re-entry). The tests `dlopen` the process itself
  (`ffi_load("")`) to reach libc symbols portably, call scalar/pointer
  functions, allocate a buffer, write/read a struct, round-trip a
  `char*` (pinning that strings only cross as buffers), drive an LPC
  callback back from C, and assert `valid_ffi` denial is enforced (for
  both a "load" path sentinel and a "peek" address sentinel);
  `ffi_doc_examples.lpc` pins every snippet in §6 verbatim, and
  `tests/std/ffi_util.lpc` covers the `/std/ffi_util` helper library.
  The suite's per-file `check_memory()` gate (which caught seven leaks
  this cycle) validates the allocation/handle accounting.
- `tools/ffi/test.py` runs the generator over `test_sample.h` and checks
  the emitted LPC/struct output, run in CI like `tools/lpc-syntax/test.mjs`.

## 10. Phasing

1. **v1a**: `ffi_load`/`ffi_symbol`/`ffi_prepare`/`ffi_call` for scalar
   + pointer + string args, `ffi_alloc`/`ffi_free`/`ffi_read`/
   `ffi_write`, `valid_ffi`, the `OFF` toggle, LPC tests.
2. **v1b**: `ffi_struct_layout` + the `tools/ffi` generator.
3. **v2**: C-calls-LPC callbacks via `ffi_closure`
   (`ffi_callback`/`ffi_callback_addr`/`ffi_callback_free`) — the
   closure trampoline (`closure_dispatch`) re-enters the VM through
   `safe_call_function_pointer`.

All three phases are implemented.
