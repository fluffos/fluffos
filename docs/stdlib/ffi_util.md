---
title: stdlib / ffi_util
---
# FFI utilities (/std/ffi_util)

A pure-LPC helper library for code that calls native functions through
[package_ffi](../driver/ffi). It packages the recurring idioms at the C
boundary — NUL-terminated strings, returned `char*` pointers,
out-parameters, argv arrays, struct fields — so binding code doesn't
re-derive them. It lives in `testsuite/std/ffi_util.lpc` and is portable
to any FluffOS mudlib (copy the file plus `include/ffi.h`).

```c
inherit "/std/ffi_util";
```

:::warning[Inherit it into the privileged object — don't simul-efun it]
`c_string()`/`c_string_enc()` call [ffi_peek](../efun/ffi/ffi_peek),
which the master gates via
[valid_ffi](../apply/master/valid_ffi)`("peek", addr, caller)` — and the
caller the master sees is the object this library is inherited into.
Exposed as a simul-efun, every peek in the mud would run with the
simul_efun object as its security principal. Inherit the library
directly into the object(s) allowed to do FFI work instead.
:::

## C strings in: `cstr()`, `cstr_enc()`

```c
buffer cstr(string s);
buffer cstr_enc(string s, string encoding);
```

`cstr()` returns the string's raw UTF-8 bytes plus a NUL terminator —
exactly what a `const char*` parameter (an `FFI_POINTER` argument)
wants. `cstr_enc()` converts through
[string_encode](../efun/strings/string_encode) first, for C functions
that expect Latin-1 or another byte-oriented encoding. (The terminator
is a single NUL byte; wide encodings like UTF-16 need a wider
terminator — build those by hand.)

```c
int sl = ffi_prepare(lib, "strlen", FFI_UINT64, ({ FFI_POINTER }));
ffi_call(sl, ({ cstr("café") }));                  // 5: UTF-8 bytes
ffi_call(sl, ({ cstr_enc("café", "latin-1") }));   // 4: é is one byte
```

## C strings out: `c_string()`, `c_string_enc()`

```c
string c_string(int addr);
string c_string_enc(int addr, string encoding);
```

Read the NUL-terminated bytes at a raw foreign address (a returned
`char*`) and decode them into an LPC string. A `NULL` (0) address
yields `0` instead of an error, matching the C convention, so returned
pointers can be passed straight through:

```c
int ge = ffi_prepare(lib, "getenv", FFI_POINTER, ({ FFI_POINTER }));
string path = c_string(ffi_call(ge, ({ cstr("PATH") })));   // 0 if unset
```

## Out-parameters: `c_out()`

```c
buffer c_out(int type_code);
```

A zeroed block sized for one scalar of the given `FFI_*` type code —
pass it where C wants `T *out`, then `ffi_read(out, 0, code)`:

```c
int fx = ffi_prepare(lib, "frexp", FFI_DOUBLE, ({ FFI_DOUBLE, FFI_POINTER }));
buffer exp = c_out(FFI_INT);
float mant = ffi_call(fx, ({ 8.0, exp }));   // 0.5
int e = ffi_read(exp, 0, FFI_INT);           // 4
```

`c_out(FFI_POINTER)` works for pointer out-parameters too — e.g.
`strtol`'s `char **endptr`, read back with `ffi_read` and `c_string()`.

## Argument vectors: `c_argv()`

```c
mixed *c_argv(string *args);
```

Builds a C `char *argv[]`: a NULL-terminated pointer array whose slots
hold the addresses of one `cstr()` buffer per string. Returns
`({ pointer_array, string_buffers })` — pass element 0 as the
`FFI_POINTER` argument, and **keep the returned array referenced for as
long as C may use the pointers**: the addresses in the pointer array
die when the buffers in element 1 are collected.

```c
mixed *av = c_argv(({ "ls", "-l" }));
ffi_call(f, ({ cstr("/bin/ls"), av[0] }));   // av stays in scope
```

## Struct fields: `c_field()`, `c_field_set()`

```c
mixed c_field(buffer data, mixed *layout, int idx, int type_code);
void  c_field_set(buffer data, mixed *layout, int idx, int type_code, mixed value);
```

Read/write field `idx` of a C struct held in `data`, using a layout
from [ffi_struct_layout](../efun/ffi/ffi_struct_layout) (the offset is
`layout[1][idx]`). With the symbolic indexes the
[tools/ffi generator](../driver/ffi#8-toolsffi--the-bindings-generator)
emits, field access reads by name:

```c
mixed *lay = ffi_struct_layout(STRUCT_TM_TYPES);
int year = 1900 + c_field(tm, lay, STRUCT_TM_tm_year, FFI_INT);
```

## See also

- [The FFI package reference](../driver/ffi) — worked libc examples
- [valid_ffi](../apply/master/valid_ffi) — the security gate
- Tests: `testsuite/single/tests/std/ffi_util.lpc`
