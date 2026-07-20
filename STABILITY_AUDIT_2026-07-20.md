# Driver stability audit — 2026-07-20

Triggered by a report that the driver is unstable ("stack corruption in the
VM"). This audit reviewed all 2026 commits, built an ASan+UBSan Debug
driver, and stress-tested/fuzzed the compiler front-end and several
packages. Bugs 1-8 below were **confirmed, reproducible crashes** against
the unmodified binary — each independently re-run to rule out flakes.
Bugs 9-10 were real, well-evidenced findings at a distinct, explicitly
lower evidence tier (see their own sections) — one a statically
unambiguous bug not reproduced as a live crash, the other a confirmed leak
rather than a crash.

**All 10 are now fixed, on this branch.** See "Fixes applied" below for
what changed and how each was re-verified; "Status — audit complete" at
the bottom has the original find-phase tally for reference.

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

### 6. `fill_default_args()` is not exception-safe — corrupts the VM value stack ★ matches the original report almost exactly

- **File**: `src/vm/internal/base/interpret.cc:1602-1662`, `fill_default_args()`
  (new this cycle, commit `aec12ca`, "Default arguments: fix helper-name
  collisions and fill on all direct-call paths").
- **Root cause**: a defaulted parameter's value is computed by calling a
  closure: `push_svalue(call_function_pointer(sv_funcp.u.fp, 0));` followed
  by `free_svalue(&sv_funcp, ...)`. If evaluating that closure calls
  `error()` — a real C++ `throw` per AGENTS.md §4 — the throw unwinds past
  this line: `push_svalue`'s pending push never completes, `free_svalue`
  never runs (leaked ref), and the enclosing loop's normal-path-only
  `fp = saved_fp;` restore is skipped too. The VM's global `sp` is left
  pointing into a stack region the closure's own (now-unwound) frame left
  in an inconsistent state. When the nearest error handler later does
  `pop_n_elems(sp - econ->save_sp)`, it walks and frees a corrupted slot;
  `int_free_svalue()` writes through a garbage pointer and crashes.
  The pre-refactor code had explicit `fatal()` sanity checks around this
  exact sequence that did not carry over into the new shared helper.
  Reachable through **every** caller: `F_CALL_FUNCTION_BY_ADDRESS` (plain
  direct calls, interpret.cc:3403), `F_CALL_INHERITED` (interpret.cc:3446),
  and `FP_LOCAL` function-pointer calls (interpret.cc:4765) — all three
  independently reproduced below, same crash signature on each.
- **This is functionally the same class of bug as the #1295 report that
  commit `ec9b6a4` fixed earlier today** (an eval-stack corruption
  triggered from perfectly ordinary LPC), just in the brand-new
  default-arguments code path instead of string/object concatenation — and
  it doesn't even need a `catch()` to trigger, just an ordinary uncaught
  `error()` in a default-argument expression.
- **Repro** (`testsuite/single/fuzz_tmp/defargs_pp/min3_nocatch.lpc`,
  independently re-verified — SIGSEGV, exit 139, "Invalid permissions for
  mapped object" i.e. a write through a corrupted pointer):
  ```lpc
  int foo(int a: (: error("boom\n") :)) { return a; }
  void create() {
      foo();
      write("after\n");
  }
  ```
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/defargs_pp/min3_nocatch.lpc
  ```
  The `F_CALL_INHERITED` and `FP_LOCAL` variants crash identically (same
  "Invalid permissions for mapped object" SIGSEGV signature), independently
  re-verified:
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/defargs_pp/inherited_err.lpc     # ::foo() — F_CALL_INHERITED
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/defargs_pp/fp_local_err.lpc      # (: foo :)() — FP_LOCAL
  ```
- **Fix shape**: wrap the closure call + push + free in an RAII guard (or
  restructure so `sv_funcp`'s ref and `sp`'s position are both restored on
  the unwind path), matching the pattern AGENTS.md §4 already prescribes for
  every other error()-adjacent cleanup in this codebase.

### 7. Macro-expansion nesting cap (`MAX_EXPANSION_NESTING = 128`) is too high for the instrumented build — real C-stack overflow at ~75-80 levels

- **File**: `src/compiler/internal/lexer_utils.cc:591`
  (`#define MAX_EXPANSION_NESTING 128`) and the mutual recursion
  `lpc_lex_resolve_identifier()` → `return yylex(...)` (~line 708).
- **Root cause**: each level of object-like macro expansion recurses one
  more `yylex()`/`lpc_lex_resolve_identifier()` C-stack frame pair. The
  128-level cap is checked, but on the ASan-instrumented build (redzone
  overhead inflates each Flex-generated frame) the process's stack is
  exhausted well before reaching it — bisected boundary: 70 levels survive,
  80 crash, i.e. the cap is only ~60% of the way to the real limit on this
  build configuration, which is one of the configurations CI itself runs
  (`ubuntu-24.04` + Clang + `ENABLE_SANITIZER=ON`, per AGENTS.md §6).
- **Repro**: `testsuite/single/fuzz_tmp/defargs_pp/pp/chain_80.lpc` — 80
  linear `#define M<n> M<n+1>` chained macros (nothing exotic; comparable to
  what layered mudlib headers could plausibly produce). Independently
  re-verified: exit 139 at depth 80, exit 0 (clean) at depth 70.
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/defargs_pp/pp/chain_80.lpc
  ```
- **Fix shape**: lower `MAX_EXPANSION_NESTING` to a value verified safe
  under the sanitizer build (mirroring how `kMaxOptimizeDepth`/
  `kMaxGenerateNodeDepth` were deliberately set to 500, not some larger
  "plausible" number, specifically because ASan frames are much larger —
  see commit `117cbc1`'s reasoning), or convert the recursive rescan to an
  explicit-stack iterative form.

### 8. `restore_object()`/`restore_variable()`: dirty sizing scratch state → uninitialized heap memory used as an allocation size in `allocate_class_by_size()` ★ remotely reachable, not just in-game

- **Network attack surface, verified directly (not just cited from the
  investigation)**: `restore_svalue()` is called on **raw, unauthenticated
  network bytes** with no sanitization beyond a length-prefix bound check —
  `src/net/transport_libevent.cc:453`, the `PORT_TYPE_MUD` connection type:
  a peer sends a 4-byte big-endian length prefix followed by that many
  bytes, which go straight into `restore_svalue(ip->text + 4, &value)`.
  `src/packages/sockets/socket_efuns.cc:1413` has the same pattern for
  MUD-mode `efun` sockets. Neither requires any authentication or a logged-in
  player — just a TCP connection to a MUD-mode port. This means the bug
  below is not only triggerable by malicious in-game LPC
  (`restore_variable()`), it's a remote pre-auth crash if a MUD-mode port
  is exposed.

- **Files**: `src/vm/internal/base/object.cc` — `restore_array()` (size
  check ~line 1031), `grow_restore_sizes()` (~line 271-292), the
  `save_svalue_depth`/`sizes[]` scratch globals (~line 74-75);
  `src/vm/internal/base/class.cc:52`, `allocate_class_by_size(int size)`.
- **Root cause, a 3-part chain**:
  1. `restore_array()`'s "too many elements" path throws a genuine C++
     `error()` ("Illegal array size.") from inside `allocate_array()`. Unlike
     the two `restore_mapping()` error sites (already guarded by
     `reset_restore_scratch()`, the fix behind AGENTS.md's documented
     "stale sizes" lesson), this array-size throw is **not** guarded, so it
     skips `restore_svalue()`'s own cleanup and leaves the file-scope
     `save_svalue_depth`/`sizes[]` scratch state dirty for whatever
     `restore_svalue()` call happens next.
  2. `grow_restore_sizes()` grows `sizes[]` via `realloc()` without
     zero-filling the newly-extended region (only the very first allocation
     is zeroed). Once the dirtied `save_svalue_depth` walks past the
     highest index a call actually wrote, reading `sizes[save_svalue_depth - 1]`
     returns genuine uninitialized heap memory.
  3. `allocate_class_by_size(int size)` performs **zero validation** of
     `size` — no bounds check at all, unlike `allocate_array()` (which
     cleanly errors on an oversized count) or `allocate_mapping()` (which
     clamps). Whatever garbage value comes out of step 2 flows straight
     into a `DMALLOC()` byte-count computation, which the debug allocator's
     own `if (size <= 0) fatal(...)` turns into a hard process abort.
- **Repro** (`testsuite/single/fuzz_tmp/saverestore/dirty_state_uninit_probe.lpc`
  — independently re-verified, exit 134/SIGABRT, deterministic): call 1
  poisons the scratch state with a wide array whose last element is an
  oversized (>100000-element) nested array; call 2 is a small truncated
  nested class literal (`"(/(/9"`) whose own one-level-deeper nesting reads
  straight into the poisoned, uninitialized region.
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/saverestore/dirty_state_uninit_probe.lpc
  ```
  ```
  DBG poison err="*Illegal array size.\n"
  ******** FATAL ERROR: illegal size in debugmalloc()
  ```
  This repro drives both calls through the `restore_variable()` LPC efun
  for a simpler, debuggable reproduction — it does **not** independently
  verify the raw-network two-packet variant. But since the poisoned
  `save_svalue_depth`/`sizes[]` state is file-scope global and every
  `restore_svalue()` caller shares it, two consecutive malicious
  `PORT_TYPE_MUD` packets (no LPC execution needed at all) are plausible
  by the same mechanism — flagged here as a reasonable inference from the
  verified root cause, not as something separately tested.
- **Fix shape**: guard the array-size throw the same way the two mapping
  error sites already are (`reset_restore_scratch()` before erroring), and
  add a bounds check to `allocate_class_by_size()` matching
  `allocate_array()`'s, as defense in depth.
- **Related, lower-severity findings from the same investigation** (not
  separately counted): `restore_variable()` has no `ROB_CLASS_ERROR` branch
  (unlike `restore_object_from_line()`, which does), so a malformed class
  literal passed to `restore_variable()` alone silently resolves to `0`
  instead of raising a catchable error; and a single oversized-nested-array
  `restore_variable()` call leaks the outer `array_t` on the same unguarded
  throw path (confirmed via the debug ref-count checker, see
  `testsuite/single/fuzz_tmp/saverestore/dirty_state_array.lpc`).

## High-confidence finding, statically verified, not yet triggered with a live crash

### 9. `socket_acquire()` never sets `O_EFUN_SOCKET` → dangling `owner_ob` after the acquiring object is destructed

- **Files**: `src/packages/sockets/socket_efuns.cc` — compare `socket_acquire()`
  (~line 1822-1845: sets `lpc_socks[fd].owner_ob = current_object;` only)
  against `socket_create()` (~line 505-515), `socket_accept()` (~line 682,
  803), `socket_connect()` (~line 855), which all pair that assignment with
  `current_object->flags |= O_EFUN_SOCKET;`. `src/vm/internal/simulate.cc:1334`
  (`destruct_object()`): `if (ob->flags & O_EFUN_SOCKET) close_referencing_sockets(ob);`
  — the force-close-on-destruct pass is gated entirely on that flag.
- **Root cause**: an object that receives a socket via `socket_acquire()`
  (having never itself called `socket_create`/`socket_accept`/`socket_connect`,
  so it has no `O_EFUN_SOCKET` flag of its own) becomes `lpc_socks[fd].owner_ob`
  without the flag ever getting set. If that object is destructed while it
  still owns the socket, `destruct_object()` skips `close_referencing_sockets()`
  entirely — the socket is never force-closed and `owner_ob` is left pointing
  at freed object memory. The next network event on that fd calls
  `safe_apply(callback, lpc_socks[fd].owner_ob, ...)`, whose very first
  action is to read `ob->flags` through the dangling pointer.
- **Evidence tier — different from bugs 1-7**: this is a static-analysis
  finding backed by direct code reading (the asymmetry between
  `socket_acquire()` and its three siblings, and the exact flag
  `destruct_object()` gates on, are unambiguous), **not yet reproduced as a
  live crash**. A repro was written
  (`testsuite/single/fuzz_tmp/socketparser/socket_acquire_uaf.lpc`:
  server/client loopback sockets, `socket_release()`/`socket_acquire()` to
  hand a socket to a throwaway clone, `destruct()` the clone, then trigger a
  read event on the orphaned fd) but it depends on real async TCP I/O +
  `call_out()`-scheduled steps, and single-file `-ftest:` runs shut down
  synchronously without servicing the backend event loop or pending
  call_outs (a harness limitation documented in AGENTS.md §7 — "call_outs
  only fire after the suite in FULL runs, never in single-file runs"), so
  the timing never lined up in that mode. Confirming this one with an actual
  crash needs either a full-suite run or a harness that drives the event
  loop directly.
- **Fix shape**: add `current_object->flags |= O_EFUN_SOCKET;` to
  `socket_acquire()`, matching its three siblings.

### 10. `mudlib_error_handler()` leaks its diagnostic mapping when building it triggers a second error (confirmed via the debug ref-count checker, not a segfault)

- **File**: `src/vm/internal/simulate.cc:2263` onward, `mudlib_error_handler()`.
- **Root cause**: the function builds a diagnostic mapping with a raw,
  non-RAII pointer (`m = allocate_mapping(6); add_mapping_string(m, "error", err); ...`)
  and only reaches `push_refed_mapping(m)` on the success path. If
  `__MAX_MAPPING_SIZE__` has been configured (legitimately, via
  `set_config()`) below what this diagnostic mapping needs (~6 entries), one
  of the `add_mapping_*()` calls trips `mapping_too_large()` → `error()` a
  *second* time while already inside error handling for a first error. That
  second `error()` unwinds straight out with `m` un-freed — the mapping and
  every string it's holding onto (an add_slash'd malloced filename, the
  shared strings `"program"` and the mapping's key) leak permanently, one
  set per such error, unreachable to any later GC or LPC code.
- **Different evidence tier from crashes 1-7**: found and confirmed via the
  driver's own DEBUG ref-count checker (`check_memory()`), which AGENTS.md
  documents as "a hard CI gate (gcc/clang Debug)" — it is a real,
  deterministic bug the project's own CI would catch, but it manifests as a
  leaked-reference report, not a segfault/ASan abort.
- **Repro** (`testsuite/single/fuzz_tmp/refloop_hotreload/copy_map_leak_min.lpc`
  — independently re-verified, deterministic, 4 leaked-reference lines every
  run):
  ```
  cd testsuite && ../build-asan/src/driver etc/config.test \
      -ftest:single/fuzz_tmp/refloop_hotreload/copy_map_leak_min.lpc
  ```
  ```
  Bad ref count for mapping, is 1 - should be 0
  Bad ref count for malloc string ".../copy_map_leak_min.lpc" add_slash 0429, is 1 - should be 0
  Bad ref count for shared string "program", is 2 - should be 1
  Bad ref count for shared string "y", is 2 - should be 1
  ```
- **Predates and is unrelated to the reference-loop/hot-reload commit** this
  was found while auditing — it's pre-existing code, surfaced as a
  side-effect of probing `copy()`'s interaction with mapping-size limits.
- **Fix shape**: build the diagnostic mapping through an RAII wrapper (per
  AGENTS.md §4's standard remedy for this exact hazard shape), or
  pre-validate against the configured max-mapping-size before starting to
  populate it.

## Areas audited with no crash found (real negative results, not gaps)

- **Reference-loop cycle efuns / orphan collector** (`src/packages/contrib/cycles.cc`,
  `src/packages/develop/checkmemory.cc`, commit `86d13cd`) and
  **`recompile_object()` hot-reload** (`src/vm/internal/simulate.cc`):
  extensive adversarial testing (multi-edge cycles into shared ancestors,
  cycle detection from inside `foreach`/`foreach ref` on the container being
  iterated, `FP_FUNCTIONAL` staleness across recompiles, recompiling a
  parent while a child executes its inherited code, nested/reentrant
  `recompile_object()` calls, self-cyclic variables carried across a swap)
  found no crash, ASan/UBSan report, or ref-count corruption in either
  subsystem. Matches the commit's own documented round-2 self-review.

- **`src/packages/math/math.cc` / `src/packages/matrix/matrix.cc`**: thorough
  static review + adversarial dynamic fuzzing (malformed/oversized/NaN/Inf
  vector arguments, boundary domain errors) found no crash. Both files
  already carry the 2026 hardening (per-element type checks, out-of-band
  status codes replacing the old sentinel-float bug). Clean.

## Status — audit complete

**8 confirmed, independently-verified crashing bugs** (each reproduced by a
second, independent run — most against the exact same commit,
`dca0eae`/`aec12ca`-era tip):

1. `disassemble()` stack-buffer-overflow — live-driver reachable via
   `dump_prog()` (package `develop`).
2. `push_function_context()`/`pop_function_context()` depth-cap asymmetry —
   null-pointer deref, 11 nested closures.
3. `ast_json()`/`dump_tree()` missing recursion cap — `lpcc --ast` segfault.
4. FFI callback use-after-free (callback frees its own handle mid-dispatch).
5. FFI call-after-unload SIGSEGV.
6. `fill_default_args()` VM eval-stack corruption — **the closest match to
   the original report**, in the brand-new default-arguments feature.
7. Macro-expansion nesting cap (128) too high for the sanitizer build —
   real overflow at ~80 levels.
8. `restore_object()`/`restore_variable()` dirty scratch state →
   uninitialized heap memory used as an `allocate_class_by_size()` size —
   hard process abort (`illegal size in debugmalloc()`).

Plus **2 further real, well-evidenced bugs** at an adjacent (not
strictly "crash") tier: #9 `socket_acquire()`'s missing `O_EFUN_SOCKET`
flag (a statically-unambiguous dangling-pointer setup, not yet triggered
live — needs real async socket timing a single-file test run can't
service) and #10 `mudlib_error_handler()`'s ref-count leak (confirmed via
the project's own hard CI-gating debug checker).

Audited with genuine adversarial effort and found clean: reference-loop
cycle efuns/orphan collector, `recompile_object()` hot-reload,
`math.cc`/`matrix.cc`. Parser/sockets got a lighter pass (one dispatched
sub-investigation was blocked by an automated safety classifier on its
adversarial-sounding brief; picked up directly by hand afterward, which is
where #9 came from).

## Fixes applied

All 10 findings above are fixed on this branch. Each fix was independently
re-verified against its own repro (crash → clean error/pass), and the full
LPC testsuite was run to completion **4× on the ASan+UBSan Debug build and
1× on a RelWithDebInfo build** — all five runs report `Checks succeeded.`
with zero ASan/UBSan reports and zero failed checks (611 files / ~5800
checks each run; the exact check count varies a few dozen between the two
build types since some checks are Debug-only).

1. **`disassemble()` F_PUSH overflow** — bounded the accumulation loop
   against `buff`'s remaining space (`fmt::format_to_n`, clamped, explicitly
   null-terminated). A **second, previously-undocumented overflow of the
   same shape** was found one frame later in the same function while
   re-verifying this fix: the instruction's raw-hex-byte dump (`tmp[257]`)
   overflows the identical way for a long instruction, and needed the
   identical treatment (bounded, explicitly terminated — `fmt::format_to_n`
   does not null-terminate on its own, which the first pass at this fix got
   wrong once before the null-termination was made explicit).
   `src/compiler/internal/disassembler.cc`.
2. **`push_function_context()`/`pop_function_context()` asymmetry** — added
   a `failed_function_context_pushes` counter: a push past the depth cap
   increments it instead of silently no-op'ing, and a pop consumes that
   budget first (closing brackets reduce innermost-first, i.e. in exactly
   the reverse order pushes were attempted, so failed-push pops always
   arrive before real ones) before touching the real stack.
   `src/compiler/internal/lexer_utils.cc`.
3. **`ast_json()`/`dump_tree()` missing recursion cap** — mirrored
   `optimize()`'s `g_optimize_depth`/`kMaxOptimizeDepth` guard (a `{"k":
   "too_deep"}` sentinel past depth 500), and converted `ast_json_seq()`'s
   `NODE_TWO_VALUES` chain walk from recursion to an explicit work-stack,
   matching how `optimize()`/`i_generate_node()` already flatten that same
   chain (so this cap bounds genuine expression nesting only, never an
   object's definition/statement count). `src/compiler/internal/generate.cc`.
4. **FFI callback use-after-free** — capture `ret_code` into a local before
   calling into LPC. Re-verification (via the full suite) surfaced a
   **deeper, related issue** the first fix didn't cover: a callback freeing
   its own handle mid-dispatch also frees the libffi trampoline
   (`ffi_closure_free`) that is *currently executing* to have reached the
   call at all, crashing inside libffi itself rather than on the later
   struct read. Fixed by tracking a `dispatching` flag on `FfiCallback` and
   having `ffi_callback_free()` refuse (clean `error()`) to free a callback
   that's currently dispatching — freeing a callback from its own body has
   no legitimate use, so refusing it doesn't cost any real functionality.
   `src/packages/ffi/ffi.cc`.
5. **FFI call-after-unload** — first attempt made `ffi_unload()` refuse to
   run at all while any function was ever prepared from that library; the
   full-suite run caught that this broke the ordinary prepare-call-unload
   lifecycle (13 existing FFI tests regressed, since `ffi_prepare()` has no
   matching "un-prepare" to balance a refcount against). Corrected to the
   originally-documented alternative fix shape: `FfiLibrary` now tracks the
   IDs of every `FfiFunc` prepared from it; `ffi_unload()` marks each
   `valid = false` and proceeds with the unload (no refusal), and
   `ffi_call()` checks `valid` and errors cleanly instead of dereferencing
   the now-stale address. `src/packages/ffi/ffi.cc`.
6. **`fill_default_args()` VM eval-stack corruption** — root cause was
   `push_svalue(x)`'s `STACK_INC; assign_svalue_no_free(sp, x);` expansion
   running `STACK_INC` before `x` (`call_function_pointer(...)`) is even
   evaluated; fixed by evaluating the closure's result into a local first,
   *then* pushing the already-computed value. Also wrapped `sv_funcp`'s ref
   release and the `fp = saved_fp;` restore in `DEFER` so both happen
   unconditionally (including when the default-argument expression's
   `error()` unwinds out), not only on the normal-return path.
   `src/vm/internal/base/interpret.cc`.
7. **`MAX_EXPANSION_NESTING` too high for the sanitizer build** — lowered
   128 → 64, comfortably under the measured 70-survives/80-crashes
   boundary, mirroring why `kMaxOptimizeDepth`/`kMaxGenerateNodeDepth` were
   deliberately kept small rather than "plausible"-sized.
   `src/compiler/internal/lexer_utils.cc`.
8. **`restore_object()` uninitialized-size abort** — guarded the
   previously-unguarded array/class oversized-size `error()` paths in
   `restore_array()`/`restore_class()` with `reset_restore_scratch()`
   (matching the two mapping error sites that already did this), and added
   the bounds check `allocate_class_by_size()`/`allocate_empty_class_by_size()`
   were missing entirely (unlike `allocate_array()`, which already had
   one), as defense in depth. Also fixed the two related, lower-severity
   findings noted alongside this one: added the missing `ROB_CLASS_ERROR`
   branch to `restore_variable()`'s error dispatch (previously silently
   resolved to `0`), and converted `restore_array()`/`restore_class()`'s
   raw `array_t* v` to a `std::unique_ptr` so a nested restore call
   throwing no longer leaks the outer (partially-built) array/class — this
   fully eliminated the "Bad ref count for array" leak the debug checker
   was still reporting after the crash fix alone.
   `src/vm/internal/base/object.cc`, `src/vm/internal/base/class.cc`.
9. **`socket_acquire()` missing `O_EFUN_SOCKET`** — added the one-line flag
   set, matching its three siblings (`socket_create()`/`socket_accept()`/
   `socket_connect()`) exactly. `src/packages/sockets/socket_efuns.cc`.
10. **`mudlib_error_handler()` diagnostic-mapping leak** — wrapped the
    mapping itself in a `std::unique_ptr` (released only at the successful
    `push_refed_mapping` handoff). Re-verification showed this alone wasn't
    sufficient — the original repro still leaked a shared-string key and a
    malloc'd filename string — tracing those down surfaced **two further,
    more general leaks** in the mapping subsystem itself, both fixed:
    `insert_in_mapping()` (`mapping.cc`) didn't release the shared-string
    conversion of its key on the `mapping_too_large()`/OOM throw path
    (affects every `add_mapping_*()` caller, not just this one); and
    `load_mapping_from_aggregate()` (`mapping.cc`, backing LPC mapping
    *literals* like `(["x":1,"y":2])`) doesn't free the current/remaining
    key-value pairs on its own `mapping_too_large()`/OOM throw paths, since
    the caller (`F_AGGREGATE_ASSOC` in `interpret.cc`) already moves the
    VM's real `sp` below the whole aggregate *before* calling in — making
    those elements invisible to the normal error-unwind's stack sweep.
    Also applied the matching malloc'd-string RAII fix at the two
    `add_slash(...)` call sites inside `mudlib_error_handler()` itself
    (the argument is allocated before the call that might reject it).
    `src/vm/internal/simulate.cc`, `src/vm/internal/base/mapping.cc`.

**Honesty note on the fix process**: three of the ten (1, 4, 5, 10 — FFI and
the two leak-adjacent fixes) needed a second iteration after the first
attempt was shown incomplete or actively regressive by re-running the
repro/full-suite rather than by inspection alone. That's recorded here
deliberately rather than only presenting the final diffs, since it's the
same "don't trust it until you've run it" discipline this audit's find
phase depended on.
