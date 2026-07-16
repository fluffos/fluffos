# tools/ffi — LPC bindings generator for `package_ffi`

Generates ready-to-use LPC bindings for a native library from its C
header, targeting the FFI package (`src/packages/ffi`, efuns declared in
`<ffi.h>`). See the full design in
[docs/driver/ffi.md](../../docs/driver/ffi.md).

## Usage

```bash
tools/ffi/generate.py <header.h> --lib <libpath> --out <basename> \
    [--string-convenience] [--emit-json]
```

Emits next to `<basename>`:

| File | Contents |
|---|---|
| `<basename>.lpc` | One LPC wrapper per exported C function. Handles are prepared lazily and cached; the library is `ffi_load`ed on first use. |
| `<basename>_structs.h` | `ffi_struct_layout` field-type arrays + symbolic field-offset `#define`s for each `struct` in the header. |

Example:

```bash
tools/ffi/generate.py /usr/include/math.h --lib libm.so.6 --out mudlib/std/libm
```

## The buffer / byte boundary

A C `char*` (or any data pointer) is emitted as an LPC **`buffer`**
parameter, **never** `string` — LPC strings are UTF-8-native and the byte
encoding must be explicit (`string_encode`/`string_decode`). With
`--string-convenience` the generator also emits a `<name>_s(string ...)`
overload that `string_encode`s to a NUL-terminated UTF-8 buffer for the
common case, clearly named so the encoding is never hidden by default.

## Supported C subset

Scalar types (`char`…`long long`, the `intN_t`/`uintN_t` family,
`size_t`, `float`, `double`), any pointer (→ `buffer`), `void` returns,
and plain `struct { ... }` bodies of those field types. Anything else —
function-pointer parameters, unions, bitfields, varargs — is **reported
to stderr and skipped**, never mis-bound. `--emit-json` dumps the parsed
signature table (the shared contract for the bindings and tests).

## Tests

```bash
python3 tools/ffi/test.py
```

Dependency-free; generates from `test_sample.h` and checks the emitted
LPC/struct output and the JSON contract. The end-to-end path (generated
bindings compiled and calling real native functions) is pinned by the
driver testsuite: `testsuite/single/tests/efuns/ffi_generated.lpc`.
