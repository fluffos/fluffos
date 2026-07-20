# Driver stability audit — 2026-07-20

Triggered by a report that the driver is unstable ("stack corruption in the VM").
This audit reviewed all 2026 commits, built an ASan+UBSan Debug driver, and
stress-tested/fuzzed the compiler front-end and several packages. Findings
below are **confirmed, reproducible crashes**, each independently re-run to
rule out flakes. Work is ongoing; this file is updated as more findings land.

## Method

- `build-asan/` — Debug build, clang, `-DENABLE_SANITIZER=ON` (ASan+UBSan).
- Minimal reproducers live under `testsuite/single/fuzz_tmp/<area>/` (outside
  `testsuite/single/tests/`, so they are never picked up by the randomized
  full-suite run — see AGENTS.md's testsuite-harness caution about that).
- Two entry points were used: `lpcc etc/config.test <path>` for compile-only
  bugs (fast, no live object needed), and
  `driver etc/config.test -ftest:<path>.lpc` for bugs that need runtime
  execution (single-file run only, does not affect the full suite).
- A finding only counts if it's a real memory-safety crash: process killed by
  signal, or an ASan/UBSan report. A clean LPC `error()`/compile error is
  correct behavior, not a finding.

## Confirmed bugs

### 1. `disassemble()` stack-buffer-overflow (F_PUSH's unbounded accumulation loop)

- **File**: `src/compiler/internal/disassembler.cc`, `disassemble()`,
  `char buff[2048]`, F_PUSH case (~line 396-409).
- **Root cause**: the F_PUSH case loops over up to 255 packed push-descriptors
  and writes each with `p += sprintf(p, "%s %d", ...)` into the same 2048-byte
  `buff`, never checking `p` against `buff + sizeof(buff)`. More than ~85
  packed descriptors in one F_PUSH instruction overflow it.
- **Why it slipped through**: commit `0f91897` (2026-07-19, "address Coverity
  Scan defects in disassembler and lpcc") converted every *single* unbounded
  strcpy/sprintf into this buffer (class/function/variable/simul_efun names)
  to snprintf — but that pass targeted single large copies, not an
  accumulating loop of many small, individually-bounded-looking sprintf
  calls. Coverity's checker doesn't reason about loop iteration counts the
  way it flags single calls, so this shape was missed.
- **Live-driver reachable**: `dump_prog(object, flags)` is an efun in package
  `develop` (compiled ON by default). Any script that can call it crashes the
  **live game driver process**, not just the offline `lpcc` tool. Ordinary
  compiler peephole optimization merges runs of similar constant pushes
  (e.g. a chain of 85+ `x[0][0][0]...` index operations) into one wide
  F_PUSH — no hand-crafted bytecode needed.
- **Repro**: `testsuite/single/fuzz_tmp/min/disassembler_fpush_overflow.lpc`
  — generates and loads an object with 200 chained `[0]` indexes, then calls
  `dump_prog()` on it.
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/min/disassembler_fpush_overflow.lpc
  ```
- **ASan**: `AddressSanitizer: stack-buffer-overflow ... in vsprintf`,
  inside `disassemble()` at disassembler.cc:402.
- **Fix shape**: track remaining space (or snprintf with a running, clamped
  offset), or build the F_PUSH text into a dynamically-growing `std::string`
  the way `function_sig_string()` already does elsewhere in this file.

### 2. `push_function_context()`/`pop_function_context()` depth-cap asymmetry → null-pointer deref

- **Files**: `src/compiler/internal/lexer_utils.cc` (`MAX_FUNCTION_DEPTH 10`,
  `push_function_context()`/`pop_function_context()`, ~line 482-507) and
  `src/compiler/internal/grammar_rules_exprs.cc:958`
  (`rule_primary_expr_functional_3`, crash site).
- **Root cause**: when the closure-nesting depth cap (10) is hit,
  `push_function_context()` calls `lexerror(...)` — a *soft*, non-throwing
  diagnostic — and returns without touching `current_function_context` or
  `last_function_context`. The grammar has no way to know a push silently
  no-op'd: every reduction of a closing `(: ... :)` unconditionally calls
  `pop_function_context()`, which unconditionally walks
  `current_function_context = current_function_context->parent`. Beyond the
  cap, pushes stop but pops keep firing, eventually walking the pointer past
  the real root (`parent == nullptr`) into `nullptr` itself.
- **Same bug class already fixed twice this cycle** in a different subsystem
  (interpret.cc's F_ADD_EQ stack-imbalance, commit `ec9b6a4`: "one arm forgot
  what its sibling arm does") — not caught there because it's a different
  file entirely.
- **Repro** (11 nested closures — the minimum past the depth-10 cap):
  `testsuite/single/fuzz_tmp/min/closure_context_null_deref.lpc`
  ```lpc
  mixed main() {
      function f = (:(:(:(:(:(:(:(:(:(:(:1:):):):):):):):):):):);
      return 1;
  }
  ```
  ```
  cd testsuite && ../build-asan/src/lpcc etc/config.test \
      single/fuzz_tmp/min/closure_context_null_deref
  ```
- **UBSan**: `runtime error: member access within null pointer of type
  'function_context_t'` at grammar_rules_exprs.cc:958:33.
- **Reachable from ordinary mudlib compiles** (not just lpcc CLI) — any
  source file nesting closures more than 10 deep crashes the compile.
- **Fix shape**: mirror push/pop success on both sides (a saturating counter
  checked by both), or have `push_function_context()` push a sentinel
  "overflow" context past the cap so every later pop has something valid to
  walk instead of silently no-op'ing.

### 3. `ast_json()`/`dump_tree()` has no recursion-depth cap (unlike its sibling walkers)

- **File**: `src/compiler/internal/generate.cc`, `ast_json()` (~line 567) /
  `dump_tree()` (~line 771) — the `lpcc --ast` / `--ast --json` renderer.
- **Root cause**: commit `117cbc1` (2026-07-14) added a depth-500 recursion
  cap to `optimize()` and `i_generate_node()` because a left-nested operator
  chain builds a parse tree as deep as the input, and walking it recursively
  with no cap overflows the C stack. `ast_json()` is a **third** recursive
  walker over the same tree shape, in the same file, that never got the
  matching guard. It runs *before* codegen in the `--ast` pipeline, so it
  crashes before the codegen-side cap ever gets a chance to reject the input.
- **Repro** (`testsuite/single/fuzz_tmp/min/ast_deep_crash.lpc`, 5000 nested
  *non-constant-foldable* unary-not operators — a literal chain like
  `!!!!!1` gets folded away at parse time and never builds a deep tree):
  ```
  cd testsuite && ../build-asan/src/lpcc --ast etc/config.test \
      single/fuzz_tmp/min/ast_deep_crash
  ```
  At depth 600 the codegen-side cap correctly rejects the input with no
  crash; at depth 5000 `lpcc --ast` segfaults inside `ast_json()`'s
  recursion, well before that cap is ever reached.
- **Crash**: `Segmentation fault` (C-stack exhaustion).
- **Reachable via**: `lpcc --ast`/`--ast --json` tooling (editor/CI tooling
  that runs lpcc on arbitrary or generated source) — not directly wired to a
  live-driver efun, so lower severity than #1, but a genuine deterministic
  crash in shipped tooling and the same missed-sibling-guard bug class.
- **Fix shape**: mirror the `g_optimize_depth`/`kMaxOptimizeDepth` guard from
  `optimize()` in `ast_json()` (emit a `{"k":"too_deep"}` sentinel past the
  cap, since `--ast` is diagnostic output, not a compile decision).

### 4. FFI callback use-after-free when a callback frees its own handle mid-dispatch

- **File**: `src/packages/ffi/ffi.cc`, `closure_dispatch()` (read at ~line
  406), freed via `f_ffi_callback_free()` → `free_callback()` (~line 420).
- **Root cause**: `closure_dispatch()` captures a raw `FfiCallback* cb` from
  the libffi closure's user-data, then calls into arbitrary LPC via
  `safe_call_function_pointer(cb->fun, nargs)`. Nothing stops that LPC call
  from calling `ffi_callback_free()` on its *own* id — which frees the
  `FfiCallback` (`ffi_closure_free`, `free_funp`, `delete cb`). Control then
  returns to `closure_dispatch()`, which immediately reads `cb->ret_code`
  from the just-freed struct.
- **Repro**: `testsuite/single/fuzz_tmp/ffi/cb_free_self.lpc` — a `qsort()`
  comparator callback that calls `ffi_callback_free()` on its own handle;
  `qsort()` on 2 elements guarantees exactly one comparator call.
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/ffi/cb_free_self.lpc
  ```
- **ASan**: `heap-use-after-free ffi.cc:406:23 in closure_dispatch(...)`.
  Independently re-run and confirmed deterministic.
- **Fix shape**: defer the actual free (mark "pending free", reap after
  `closure_dispatch()` returns), or have `closure_dispatch()` copy every
  field it still needs (`ret_code`) into a local *before* calling into LPC.

### 5. FFI call-after-unload → SIGSEGV

- **File**: `src/packages/ffi/ffi.cc` — `FfiFunc` caches a raw `dlsym()`
  address with no back-reference to its owning `FfiLibrary`; `f_ffi_unload()`
  unconditionally `dlclose()`s with no check for outstanding `FfiFunc`
  entries; `f_ffi_call()` (~line 557) calls through the now-stale address.
- **Repro**: `testsuite/single/fuzz_tmp/ffi/call_after_unload.lpc` (needs a
  throwaway `.so` — a two-line `vlib_add(int,int)`; build it with any C
  compiler and point `ffi_load()` at the path in the script).
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/ffi/call_after_unload.lpc
  ```
- **Confirmed**: driver exits 139 (SIGSEGV), deterministic across repeated
  runs (checked the real exit code via redirect + `$?`, not through a pipe).
- **Fix shape**: give `FfiFunc` a reference back to its owning `FfiLibrary`
  (or a shared refcount); refuse `ffi_unload()` while prepared functions
  still reference it, or invalidate them at unload time and have
  `ffi_call()` check.

## Areas audited with no crash found (real negative results, not gaps)

- **`src/packages/math/math.cc` / `src/packages/matrix/matrix.cc`**: thorough
  static review + adversarial dynamic fuzzing (malformed/oversized/NaN/Inf
  vector arguments, boundary domain errors) found no crash. Both files
  already carry the 2026 hardening (per-element type checks, out-of-band
  status codes replacing the old sentinel-float bug). Clean.

## Status

5 confirmed, independently-verified crashing bugs so far (3 compiler
front-end, 2 FFI package). Audit continuing across save/restore_object,
sockets/parser, reference-loops/hot-reload, and default-arguments/
preprocessor; this file will be updated as further findings are confirmed.
