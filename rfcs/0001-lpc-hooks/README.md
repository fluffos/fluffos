# RFC 0001: In-game hooks — eBPF-style join points for LPC

| | |
|---|---|
| Status | Draft v2 — revised after a six-angle review (see [§12](#12-review-log)) |
| Issues | #1414 (call_other miss report) is the first consumer |
| Prior art | LDMud `set_driver_hook()`, `H_DEFAULT_METHOD`, `limited()`, `trace()`, Python hooks; DGD auto object, driver-object applies, kernellib object/error managers, `call_touch()`, `rlimits`, `atomic` |
| Evidence | Probes in [`probes/`](probes/), run on master `b5714e5f` (RelWithDebInfo, gcc) |

## 1. Summary

Mudlibs keep asking for small cross-cutting driver features: report a
`call_other` to a missing function (#1414), fence a directory off from callers,
count calls, account object lifetimes. This RFC proposes a small, layered
mechanism instead of one feature per request, split the way eBPF is split:

* **The driver is the kernel.** It defines a fixed set of join points, makes
  them free when unused, runs handlers in a contained frame, and offers cheap
  C-side filtering and counting for high-frequency events.
* **One privileged hook daemon is the only subscriber.** The master designates
  it, like it designates the simul_efun object. The driver delivers events to
  it and nothing else, so there is exactly one place where hook code runs
  with the driver's trust.
* **Policy is LPC.** Multiple subscribers, wizard-facing attach/detach,
  scoping ("only events touching your own objects"), dedup and reports are
  written in the daemon, in LPC, where mudlibs differ. A reference daemon
  ships with the docs.

The driver surface is three layers, each earned by scenarios the layer below
cannot serve:

| Layer | What | Earned by |
|---|---|---|
| 0 | No driver change: document the mechanisms that already cover a scenario | S2 (dev), S3, S5 (with a base object), S8 |
| 1 | Three rare driver events, zero cost on the normal path: `call_other_miss`, `object_created`, `object_destructed` | S1, S5 (no base object, with cause), S7 |
| 2 | Runtime probes on hot paths, available only to the daemon: `call_other` call probes and opted-in `efun` probes, with per-program gating, pointer-keyed filters, `count`/`time` actions and positional handler arguments | S2 (runtime), S4, S6, S9–S12 |

A DGD-style auto object (compile-time advice on efuns) is complementary and
is left to a separate RFC (§10).

## 2. Scenarios

| # | Scenario | Need |
|---|---|---|
| S1 | **Typo finder** (#1414): list every `call_other` to a function that does not exist or is not callable, once per (caller program, target program, function), on a dev server | Observe misses, including array targets and `(: ob, "fn" :)` calls, at zero cost on hits |
| S2 | **Hot-function profile**: which functions are called most, by whom | Dev-time: the tracer. Runtime on a live mud: counted call probes |
| S3 | **File-write audit**, including `efun::write_file` | Per-call, unbypassable observation of file efuns |
| S4 | **Capability fence**: objects under `/domains/` may not `call_other` into `/secure/` | Decide (deny) before the call runs; real caller identity |
| S5 | **Lifecycle accounting**: live objects per program; leaks; every destruct, driver-initiated included, with its cause | Observe create and destruct for all objects |
| S6 | **Deprecation**: warn with the caller when `/std/old_combat::attack()` is called, attachable at runtime | Observe one function, filtered in C |
| S7 | **Default method / proxy** (LDMud `H_DEFAULT_METHOD`) | Supply a result for a miss |
| S8 | **Error telemetry**, caught or not | Observe errors with their location |
| S9 | **Eval accounting** per player or domain | Count and time calls into a set of programs, in C |
| S10 | **`call_out` leak finder** | Observe `call_out` with caller and function |
| S11 | **Per-domain object quotas** | Decide before `clone_object`/`new`/`load_object` |
| S12 | **Security alarms**: `snoop`, `exec`, `seteuid`, `shutdown`, `save_object`/`restore_object` | Observe those efuns with caller |

Out of scope: hot-patching code (`recompile_object()`), per-opcode tracing
(the tracer and the debugger in #1286), and command/message auditing (already
served by the `process_input`, `receive_message` and `catch_tell` applies).

## 3. What FluffOS can do today (corrected after review)

### 3.1 Probe results

All rows reproduced by an independent reviewer. Timings: median of 5
launches, N = 200k, empty-loop baseline (7 ns) subtracted; a single launch can
spread ±25% on a shared machine, so read them as ratios.

| Probe | Result |
|---|---|
| H1 | A simul_efun named `call_other` sees `ob->fn()`, explicit `call_other()`, `(: call_other ... :)` and array targets. |
| H2 | A miss returns the **undefined** zero, so `undefinedp(r)` separates it from a real 0 and the wrapper only pays `function_exists()` on misses. |
| H4 | `efun::call_other` and `efun::destruct` skip simul overrides, **but** the master's `valid_override(file, efun, main)` already gates every `efun::` use at compile time, so bypass is mudlib policy. |
| P1/P3 | A **naive** wrapper (`efun::call_other(ob, fn, args...)`) makes the simul_efun object the caller: `previous_object()` in every callee becomes `/single/simul_efun`, and shadows that forward with `tgt->fn()` recurse until `Too deep recursion`. A **transparent** wrapper ([`simul_efun_transparent.lpc`](probes/simul_efun_transparent.lpc): `bind((: efun::call_other :), previous_object())`, cached per caller) preserves `previous_object()` and composes with shadows. The naive result is a pitfall, not a limit. |
| H3 | Shadows see external calls and driver applies (`id()` via `present()`), not local calls, not misses; one shadow per object. |
| P2 | A simul `destruct()` misses driver-initiated destructs: a shadow destructed with its target, environment contents that `move_or_destruct` could not move, and objects refused after load (`valid_object` denial, `creator_file` failure). `set_notify_destruct(1)` + `on_destruct` **does** see all of them. (Note: `on_destruct` on a shadowed object is routed to the shadow first, so it can fire twice.) `shutdown()` does not run `destruct_object()`. |
| H5 | Local lfun call 45 ns; direct `call_other` 122 ns; naive simul wrapper 379 ns; **transparent wrapper 417 ns (3.4x)**; transparent wrapper into a shadowed object 562 ns; `function_exists()` 78 ns. Most of the wrapper's cost is its own LPC frame and the `args...` repacking, not the miss check. |
| — | Simul resolution is decided at compile time, so a *new* override never reaches code compiled before it. A wrapper installed at boot can consult a runtime table, which makes attach/detach possible at the cost of the wrapper on every call. |
| — | Caught errors reach master `error_handler(map, 1)` when `mudlib error handler` is set, at throw time with the trace live. |

### 3.2 Coverage

| Scenario | Existing mechanism | Verdict |
|---|---|---|
| S1 typo finder | Transparent `call_other` wrapper | Works, **at 3.4x on every call_other** in the mud, and blind to `call_other`s that do not go through the simul (the driver's own `apply()`s are not misses anyone wants, so this is acceptable) |
| S2 profile | `trace_start()`/`trace_end()` (see `docs/concepts/general/tracing.md`); `PROFILE_FUNCTIONS` builds | **Yes, dev-time.** Not for always-on production counting |
| S3 file audit | `valid_write(path, caller, efun)` runs per call inside the efun, sees `efun::` calls; `call_stack()` gives the site | **Yes** |
| S4 fence | Transparent wrapper + `valid_override` denying `efun::call_other` under `/domains/`; or a callee-side `previous_object()` check (kernellib's way) | Works, at 3.4x on every call, or by editing every protected function |
| S5 lifecycle | `valid_object()` before every `create()`; `on_destruct` with a mandatory base that calls `set_notify_destruct(1)` | **Yes, with a mandatory base object.** No destruct cause |
| S6 deprecation | Edit the function, or redirect it with `inherit_program` | Yes, with a recompile; not attachable at runtime |
| S7 default method | Transparent wrapper + `undefinedp(r) && !function_exists()` → proxy | Partial: 3.4x on every call |
| S8 errors | master `error_handler` | **Yes** (master only) |
| S9–S12 | `valid_*` covers some efuns per call; `call_out` and object creation have no per-call observation | No |

**Honest residual case.** What the driver alone can add:
(a) miss observation and default methods at **zero cost on hits** instead of
3.4x on every call; (b) lifecycle events with a **cause** and without a
mandatory base object; (c) call and efun observation/decision **attachable at
runtime** at a cost proportional to what is observed, not to all traffic;
(d) composition of several observers without each paying the wrapper cost.

## 4. Prior art (corrected)

Sources: LDMud `github.com/ldmud/ldmud` `8afa5f50` (3.6.8); DGD
`github.com/dworkin/dgd` `733ea01`; kernellib `e602afe`; cloud-server
`7959357`; lpc-ext `0e299c8`.

### 4.1 LDMud

- **`set_driver_hook()`**: one handler per hook in a flat 32-slot array
  (`src/simulate.c:238`), gated by `privilege_violation()` unless the caller is
  the master or simul_efun object (`:5082-5091`); values are sealed (strings
  interned, arrays copied, unbound lambdas bound to the master, `:5199-5201`).
  Most hooks are **policy slots** — move, uids, create/reset/clean_up
  dispatch, command parsing, include dirs, auto-include, file encoding (compiler
  *and* `read_file`/`write_file`), prompts, telnet.
- **`H_DEFAULT_METHOD`** runs after normal lookup and the shadow chain fail
  (`src/interpret.c:20439-20444`): zero cost on hits, one type test on a miss.
  Protocol `fn(&result, ob, fun, args...)`, 0 = declined. Exempt: master
  (`:25095`, `:21016`), simul_efuns, lightweight objects (`:25089`),
  `call_direct*`. Not documented: it also fires for driver applies through
  `sapply()` (`interpret.h:314`; `__INIT` `object.c:498`, `catch_tell` `:1330`,
  `id` `:4472`). History: a closure bound to a destructed object (Mantis #523,
  `test/t-0000523.c`) and a leak when the handler errored (`test/t-errors.c`).
- **`limited(closure, limits...)`** runs a closure under its own eval/depth
  limits; `doc/master/runtime_error` recommends it for the error handler.
  Hooks are not wrapped in it automatically.
- **`trace()`/`traceprefix()`** + master `valid_trace()`: per-interactive,
  observe-only text trace of calls, call_others, applies and returns, gated by
  a global bit test at each site (`interpret.c:9495, 20585, 20642`) — the same
  cost class as this RFC's detached test; no aggregation.
- Master **`runtime_error(err, prg, curobj, line, culprit, caught)`** fires
  for caught errors too; **`prepare_destruct()`** can veto any
  `destruct_object()` (`simulate.c:2616-2626`).
- **Python `register_hook`**: `ON_OBJECT_CREATED`, `ON_OBJECT_DESTRUCTED`,
  `ON_HEARTBEAT`, signals, `BEFORE_INSTRUCTION` (3.6.8). Multiple subscribers,
  observe-only, exceptions printed and skipped (`pkg-python.c:18048-18061`);
  operator Python only. `BEFORE_INSTRUCTION` costs an out-of-line call per
  opcode in a Python build even with no handler (`interpret.c:10150-10152`).
- Error containment differs per hook by accident (`secure_*` wrappers for
  some, propagation for others); hooks run on the caller's eval budget.

### 4.2 DGD

- **Auto object**: inherited by everything except the driver object; may
  redeclare kfuns (`lpc-doc/Introduction:9-13`). `::kfun()` from an ordinary
  program resolves to the auto object's definition when one exists
  (`src/comp/control.cpp:1495-1512`), so advice cannot be bypassed the way
  FluffOS's `efun::` bypasses a simul (which `valid_override` must close
  instead). `a->f()` compiles to a call to the name `call_other`
  (`src/comp/compile.cpp:2544`). kernellib wraps `destruct_object`,
  `compile_object`, `clone_object`, `call_out`, file kfuns etc.: check, notify a
  manager, `::kfun()` (`kernellib/src/kernel/lib/auto.c:224-262`). cloud-server
  stacks a second auto layer by answering `include_file` with an inherit
  (`objectd.c:627-640`).
- **Driver-object applies**: `call_object` (string targets of `call_other`
  only, `kfun/builtin.cpp:2153-2158`), `path_read`/`path_write` on every file
  kfun (`src/path.cpp:47,70`), `touch`, `runtime_error`, `atomic_error`,
  `remove_program`, `compile_rlimits`/`runtime_rlimits`, and the rest. The
  driver calls `create()` lazily on the first call into an object
  (`interpret.cpp:2534-2539`).
- **kernellib object-manager hooks**: a fixed set of named events with **one
  LPC subscriber** — `compiling`, `compile`, `compile_lib`, `compile_failed`,
  `clone`, `destruct`, `destruct_lib`, `remove_program`, `include_file`,
  `touch`, `forbid_call`, `forbid_inherit`
  (`kernellib/src/doc/kernel/hook/driver:19-84`). This is the shape this RFC
  adopts for its daemon.
- **`call_touch()`/`touch()`**: one flag test per call
  (`!(obj->flags & O_TOUCHED)`, `interpret.cpp:2520`); the next call into a
  marked object fires `touch(obj, fn)` before lookup (so even for a missing
  function); the object is re-marked touched before the hook runs, and a true
  return re-arms it (`:2525-2531`).
- **Containment**: `rlimits (stack; ticks) {}`; `atomic` rolls back state;
  `callCritical` runs `runtime_error`/`atomic_error`/`remove_program`
  **unlimited** (`rlimits(-1;-1)`, `interpret.cpp:2925`) with errors caught and
  logged (`:2948`). `runtime_error` runs at throw time, before unwinding,
  with the catch depth (`error.cpp:166-171`).
- **Misses** return `nil` with no hook (`builtin.cpp:2177-2183`), and
  kernellib relies on it (`auto.c:359`).

### 4.3 What we take

| From | Take | Avoid |
|---|---|---|
| LDMud | `H_DEFAULT_METHOD`'s placement and by-ref result protocol; master gating; sealed values; `limited()`-style own budget | Silent widening to driver applies; per-hook accidental error policy; hooks on the caller's budget; an always-on per-opcode call |
| DGD / kernellib | One privileged LPC subscriber for a fixed event set (the object manager); contained critical calls; touch's cheap per-object/per-program gate and re-arm; the auto object as a separate, complementary layer | Unlimited budgets for code that wizards write; permissive returns (nil/0 = deny, AGENTS.md §13.24) |

## 5. Design

### 5.1 The hook daemon

```lpc
// master
string get_hook_daemon();   // e.g. "/secure/hookd"; absent or 0 = hooks off
```

* Resolved at boot and after a master recompile, like the simul_efun object;
  the driver loads it by path and re-resolves it if it is destructed and
  reloaded, so the ordinary `update` (destruct + reload) idiom keeps working.
* On load, the driver records which Layer 1 applies the daemon defines
  (`call_other_miss`, `object_created`, `object_destructed`) as bits in
  `g_hook_mask` — the same caching the driver does for master applies. A
  join point whose apply is not defined costs one predicted branch.
* Events never fire for calls made **by** the master, the simul_efun object
  or the daemon itself, during compile, inside `valid_*` applies, or inside
  `error_handler`.
* There is no `valid_hook()` apply: designating the daemon *is* the
  authorization. Wizard-facing APIs live in the daemon and are the daemon's
  policy.

### 5.2 Layer 1: rare events

| Apply on the daemon | Fires | Site | Protocol |
|---|---|---|---|
| `int call_other_miss(mixed ref result, object caller, object target, string fn, string reason, mixed *args)` | A `call_other` (`->`, explicit, array element, `(: ob, "fn" :)`) found nothing callable, after the shadow chain | inside `apply_low()`, only when `local_call_origin == ORIGIN_CALL_OTHER` — the one place all three dispatch routes converge, where the reason is known and the arguments are still on the stack | `reason` is `"undefined"`, `"private"`, `"static"`, `"protected"` or `"destructed"`. Return **1** to claim the call: `result` becomes its value. Anything else (0, a promise, an error) = declined; the caller gets `undefined` as today |
| `void object_created(object ob)` | After `create()` returns (in `call_create()`), not if `create()` destructed the object | `src/vm/internal/base/object.cc` `call_create()` | observe |
| `void object_destructed(object ob, string cause)` | At the start of `destruct_object()`, before the existing `O_DESTRUCTED` re-check | `src/vm/internal/simulate.cc` `destruct_object()`, beside `on_destruct` | observe; `cause` is `"efun"`, `"shadowed"`, `"environment"`, `"refused"` (`valid_object` denial / `creator_file` failure) |

S1 is a daemon that keeps a mapping of `(caller program, target program, fn)`
counts, filters out known optional hooks, and prints them on demand. Misses
are rare, so LPC dedup costs nothing measurable, and hits cost nothing at all.

Errors stay with `error_handler` (S8): it already runs at throw time with the
trace live, and a second error hook adds nothing but another way to recurse.

### 5.3 Layer 2: probes (daemon-only efuns)

High-frequency join points cannot call LPC on every event without becoming
the 3.4x wrapper again. Layer 2 lets the daemon attach **probes** with C-side
gating, filtering and aggregation:

```lpc
int probe_attach(string point, mixed action, mapping filter, int flags);
void probe_detach(int id);
mapping probe_query(int id);   // rows, events, dropped, errors, last_error
void probe_reset(int id);
```

Only the daemon may call these (an error otherwise). The daemon exposes its
own, scoped API to wizards.

| Point | Fires | Site |
|---|---|---|
| `call_other` | a resolved `call_other`, before the function runs (so `PROBE_DECIDE` can deny) | `apply_low()`, after lookup, before `push_control_stack()`, `ORIGIN_CALL_OTHER` only |
| `efun:<name>` | before the body of an efun that opts in (phase 1: `write_file`, `write_bytes`, `rm`, `rename`, `mkdir`, `rmdir`, `call_out`, `clone_object`, `new`, `load_object`, `save_object`, `restore_object`, `snoop`, `exec`, `seteuid`, `shutdown`, `socket_*`) | one `HOOK_EFUN()` line at the top of each opted-in efun; covers `F_EFUN*`, efun function pointers and `efun::` alike |

**Filter keys.** Exact strings or arrays of strings; prefix match only when
the pattern ends in `/`; no globs.

| Key | Matches |
|---|---|
| `target` | target object's name, `#n` clone suffix stripped |
| `defined_in` | the program that defines the called function (S6: a function inherited by many objects) |
| `function` | function name |
| `caller` | calling object's name, `#n` stripped |
| `caller_program` | the program of the calling frame |
| `exclude` | a mapping with the same keys, matched after the include keys |

**Actions.** `"count"` (rows keyed by caller program, target program,
function), `"time"` (count + summed and max eval microseconds, for S9), or an
LPC function. **Flags.** `PROBE_DECIDE` lets a handler deny (`call_other`,
`efun:*`): it returns `1` to deny with a generic error, or a string to deny
with that message; anything else (0, a promise, other types) allows.
`PROBE_FAIL_CLOSED` makes a handler error or budget overrun deny instead of
allow, for fences that must not fail open.

**Handler signature (positional, not a mapping).** Building a context mapping
costs ~200 ns per event; positional arguments do not:

```lpc
int handler(int id, object caller, object target, string fn, mixed *args);
```

**Gating.** At attach the driver resolves `target`/`defined_in` against loaded
programs and sets a `PROG_HOOKED` bit on matching `program_t`s (re-evaluated
when a program is compiled). A non-matching call pays one predicted branch on
a field `apply_low()` has already loaded. A matching program pays one probe of
an open-addressed set keyed on interned `(program*, function-name*)`
pointers, estimated at 5–10 ns. String-keyed lookups (~50 ns) are not
acceptable here.

### 5.4 Contained invocation (the "verifier")

Every daemon call — Layer 1 apply or Layer 2 handler — goes through one
primitive, modelled on `call_out`'s dispatch (`src/packages/core/call_out.cc`)
and `pop_control_stack()`'s defer loop (`src/vm/internal/base/interpret.cc`):

```cpp
int const num_arg = st_num_arg;                                  // AGENTS §13.16
auto ev = get_eval(); int oot = outoftime, mee = max_eval_error, tde = too_deep_error;
g_in_hook++;
DEFER { g_in_hook--; st_num_arg = num_arg;
        set_eval(ev); outoftime = oot; max_eval_error = mee; too_deep_error = tde; };
if (too_deep_error || max_eval_error || csp >= &control_stack[CFG_MAX_CALL_DEPTH - kHookHeadroom]) return;
set_eval(CONFIG_INT(__RC_HOOK_EVAL_COST__));
// push positional args, then safe_apply() / safe_call_function_pointer()
```

| Rule | Hazard it answers |
|---|---|
| Own eval budget; the caller's eval timer and `outoftime`/`max_eval_error`/`too_deep_error` restored afterwards | Without the restore a handler that runs out of eval makes the caller's next opcode die with "Too long evaluation" |
| Errors stop at the boundary (`safe_*`), are counted, and are reported through the normal uncaught-error path (debug.log + master `error_handler(map, 0)`) | LDMud's accidental per-hook propagation |
| `st_num_arg` latched and restored; for `efun:*` probes, every object argument re-validated after the handler (destructed → error), arguments are shallow copies (arrays and mappings shared) | §13.16; a handler destructing an argument the efun then uses blind (`f_destruct`) |
| A handler or apply that returns a promise, or is `async`, is "declined" | §13.24 |
| Per-attachment re-entry guard, not a global "no hooks inside hooks". The daemon's own calls are exempt (§5.5), but code a handler runs on someone else's behalf — e.g. a wizard subscriber the daemon dispatches to — is still observed by every *other* probe | A global guard would let any code reached from a handler bypass audit probes |
| Layer 1 never re-enters itself (`g_in_hook` per join point) | recursion |
| Detach on the first "stale function pointer" or owner-destructed error; otherwise after `hook max errors` (default 10). `PROBE_FAIL_CLOSED` attachments never auto-detach; the daemon is told via `probe_detached(id, reason)` | §3 dangling funptrs; a fence that silently disappears |
| Attachments, handler funptrs and filters marked in `checkmemory.cc` (`mark_funp`, after `mark_call_outs()`'s pattern); count tables are C++ containers bounded by `probe max rows` with a `dropped` counter | §3 off-graph references; unbounded memory from attacker-chosen names |
| `this_player()` is 0 inside handlers and Layer 1 applies, restored afterwards | a handler acting as the victim (`input_to`, `command`) |
| Detaching inside a handler marks the attachment; it is swept after dispatch | iterator invalidation (§13.14) |

Eval limits are enforced on Linux only (`src/vm/internal/eval_limit.cc`); on
macOS, Windows and WASM the handler shares the caller's wall clock. Document
it.

### 5.5 Security model

* **One subscriber.** Only the master-designated daemon receives events and
  attaches probes. Disclosure of other people's arguments (passwords to a
  login daemon, tells, file contents) is therefore the daemon's policy; the
  reference daemon scopes wizard probes to events whose caller or target lives
  in the wizard's directory, and redacts `args` otherwise.
* **Hard exemptions** regardless of daemon: nothing fires for the master,
  the simul_efun object or the daemon as caller; no probe may name the
  master or simul_efun program as `target`/`defined_in`; nothing fires
  during compile, inside `valid_*`, or inside `error_handler`; operations
  started by the master or simul_efun object are never denied.
* **Decide is narrow.** Layer 1 can only *supply* a result for a miss; Layer 2
  can only *deny*. Neither can grant what a `valid_*` apply refused: `efun:*`
  probes fire after the efun's own `valid_*` check.

### 5.6 Cost model and acceptance gate

A reviewer built master with a `g_hook_mask` test at all five candidate sites,
including one before **every** efun body, and compared it with plain master:
every operation stayed within the ±6% launch-to-launch noise (call_other
113/113 ns, local call 43/43, `sizeof` 19/19). The detached cost is below what
wall-clock timing can resolve, so the gate cannot be "< 1% in 5 runs".

| Gate | Method | Threshold |
|---|---|---|
| Detached | `valgrind --tool=cachegrind` instruction counts on a hooks microbenchmark, A/B | > 0.5% Ir/iter on any detached case fails |
| Detached, wall clock (local, numbers pasted in the PR) | interleaved A/B, ≥ 10 launches each, best-of-5 inner, medians | max(3%, 2× baseline inter-launch MAD) |
| Attached | same harness: `count` on misses; 1 and 100 non-matching filters; one matching LPC handler; `time` action | recorded, no fixed threshold; matching handler expected ~250 ns over the call |
| Memory | Debug build with a probe attached runs the suite with no `check_memory()` report | must pass |

A `testsuite/command/speed_hooks.lpc` (or a `-fspeed:hooks` section) holds the
microbenchmark: local call, call_other hit and miss, `sizeof`, new+destruct,
and the attached cases.

## 6. Interaction with existing features

* **Shadows**: Layer 1 and Layer 2 fire inside `apply_low()` after shadow
  resolution, so `target` is the object actually reached and a miss means the
  shadow chain had nothing either.
* **`recompile_object()`**: probes are keyed by program and function *names*;
  `PROG_HOOKED` is recomputed when a program is compiled.
* **Simul_efuns and `valid_override`**: unchanged. `efun:*` probes fire inside
  the efun, so `efun::` calls are observed.
* **Async**: handlers and applies are synchronous; `async` ones are declined.
* **The debugger (#1286) and the tracer** use their own bits in the same mask
  word.

## 7. Reference daemon (shipped with the docs)

A `/secure/hookd` in `docs/` and the testsuite: keeps per-wizard subscriptions
in LPC (multi-subscriber composition), scopes and redacts per §5.5, persists
subscriptions across its own reload, implements S1 (`call_other_miss` + an
exclude list for optional hooks such as `query_*`/`is_*`), S5 (created/destructed
counts by program, with cause) and S4 (a `/secure/` fence via a
`PROBE_DECIDE | PROBE_FAIL_CLOSED` probe).

## 8. Implementation plan

Each phase is one PR, reviewed and merged before the next.

| Phase | Scope | Tests | Done when |
|---|---|---|---|
| 0 | Docs only: `docs/concepts/general/interposition.md` covering `valid_object`+`on_destruct`, `valid_write`, the tracer, `error_handler`, `valid_override`, and the transparent-wrapper idiom with its cost | — | merged |
| 1a | `get_hook_daemon` master apply; daemon resolution and reload; `g_hook_mask` + bit caching; the contained-invocation primitive; `call_other_miss` in `apply_low()` | LPC: miss via `->`, explicit, array, funptr; each reason; `undefined` preserved when declined; result when claimed; promise/error = declined; no event from master/simul/daemon callers; daemon reload keeps working. GTest: eval-state restore after a handler overrun. Benchmark gate §5.6 | closes #1414 |
| 1b | `object_created`, `object_destructed` with cause | LPC: driver destructs (shadow, environment contents, refused load) each reported once with the right cause; self-destructing `create()` not reported as created; Debug `check_memory()` clean | merged |
| 2a | `probe_*` efuns, filters, `PROG_HOOKED`, pointer-keyed set, `count`/`time`, `probe max rows`, `call_other` point | LPC: filter semantics table above; exclude; `PROBE_DECIDE` deny with message; `PROBE_FAIL_CLOSED`; re-entry guard (a handler's `write_file` seen by another probe); detach inside handler. Benchmark gate including attached cases | merged |
| 2b | `efun:*` opt-ins with `HOOK_EFUN()`, argument re-validation | LPC per efun group; a handler destructing an efun argument gets a clean error | merged |
| 3 | Reference daemon and `docs/concepts/general/hooks.md`, efun and apply pages, `include/hooks.h` | the reference daemon runs in the testsuite | merged |

## 9. Security invariants (each has a test)

1. No event fires when the caller is the master, the simul_efun object or the
   daemon; during compile; inside `valid_*` or `error_handler`.
2. `probe_*` called by anything but the daemon errors.
3. A Layer 1 apply returning a promise, an error, or anything but 1 changes
   nothing.
4. `efun:*` probes observe `efun::` calls, and calls made by non-daemon code
   that a probe handler invoked; a probe never observes its own handler.
5. `PROBE_FAIL_CLOSED`: handler error or overrun denies; the attachment
   stays; `probe_detached` is not called for it.
6. `this_player()` is 0 inside Layer 1 applies and probe handlers, and the
   caller's `this_player()` is restored afterwards.
7. Count tables never exceed `probe max rows`; overflow is counted.
8. Filter canonicalization: `/secure/login`, `/secure/login.c` and
   `/secure/login#3` (as `target`) match the same attachments.

## 10. Alternatives considered

**A. A DGD-style auto object** (a config key naming a program every object
implicitly inherits; it may redefine efuns, and `valid_override` keeps
`efun::` from bypassing it). Zero runtime cost, caller identity preserved,
the kernellib model. It needs a recompile to change, cannot see misses or
driver-initiated destructs, and cannot observe `call_other` without the 3.4x
wrapper. It is the right tool for S3-style efun policy that never changes at
runtime. **Recommended as a separate RFC**; this RFC does not depend on it.

**B. LDMud single-slot driver hooks.** The Layer 1 daemon is this shape
(one subscriber per event), with LPC doing the composition, which LDMud
leaves to a mudlib-written dispatcher anyway.

**C. More master applies, one per feature.** Layer 1 is, in effect, this —
three applies — but on a designated object instead of the master, so the
master does not have to grow every policy.

**D. Any wizard attaches handlers directly** (draft v1). Rejected after
review: every wizard becomes an observer of everyone's arguments, a global
"no hooks inside hooks" rule lets audits be bypassed from inside a handler,
fences fail open when they error, and ordinary `update` silently drops
attachments. Putting one daemon in front of the mechanism removes all four.

**E. External tracing (USDT/bpftrace).** Complementary for operators;
`hooks.cc` can emit a USDT probe at each join point at no extra cost.

## 11. Decisions for the maintainer

1. Daemon designation: master apply `get_hook_daemon()` (proposed) or a
   config line.
2. Names: `probe_*` (proposed; `hook` is already common mudlib vocabulary,
   e.g. Lima's `add_hook`/`call_hooks`) or `hook_*`.
3. Is a destruct veto wanted (LDMud `prepare_destruct`)? Not proposed.
4. Phase 2 scope: ship `call_other` and `efun:*` probes, or stop after
   Layer 1 until a mudlib asks.
5. Should the auto-object RFC come first?

## 12. Review log

Draft v1 was reviewed from six angles; the main changes:

| Review | Main finding | Change |
|---|---|---|
| Evidence | P1/P3 came from a careless wrapper; a `bind()`-based wrapper keeps the caller and composes with shadows. S2, S3, S5 and S8 are already served | §3 rewritten; case rests on cost and on events LPC cannot see |
| VM safety | `call_other` miss and call cannot be at `f__call_other()` (arguments already popped, reason unknown, array/funptr routes missed); handler eval overruns leak into the caller; `efun:*` handlers can invalidate checked arguments; `error` at throw time already exists as `error_handler` | Sites moved into `apply_low()`; §5.4 primitive; argument re-validation; `error` join point dropped |
| Security | Argument disclosure, audit bypass via a global re-entry guard, fail-open fences, decide hijacking | One privileged daemon; per-attachment re-entry; `PROBE_FAIL_CLOSED`; hard exemptions; narrow decide |
| Mudlib API | Filter semantics undefined; mode was a filter key; count keys used arrays; `update` dropped attachments; missing scenarios | Filter table; flags; row-shaped `probe_query`; daemon resolved by path; S9–S12 |
| Performance | Detached cost below measurement floor; a ctx mapping costs ~200 ns per event; "< 1% in 5 runs" is unmeasurable | Positional handlers; `PROG_HOOKED` + pointer-keyed set; cachegrind gate |
| Prior art | DGD `callCritical` is unlimited, not budgeted; `valid_override` already closes `efun::`; missed LDMud `limited()`, `trace()`, `runtime_error`, `prepare_destruct` and kernellib's object-manager hooks | §4 corrected; daemon modelled on kernellib's manager |

## Appendix: reproducing the probes

```sh
mkdir -p testsuite/clone/probe
cp rfcs/0001-lpc-hooks/probes/{target,plain,shadow,shadow_arrow,notify_target,notify_shadow}.lpc testsuite/clone/probe/
cp rfcs/0001-lpc-hooks/probes/zz_probe*.lpc testsuite/single/tests/
# naive wrapper (P1/P3 pitfall) or transparent wrapper:
cat rfcs/0001-lpc-hooks/probes/simul_efun_transparent.lpc >> testsuite/single/simul_efun.lpc
cd testsuite && ../build/src/driver etc/config.test -ftest:single/tests/zz_probe_hooks.lpc | grep PROBE
```

These edits are for the experiment only; revert them afterwards
(`git checkout -- testsuite/single/simul_efun.lpc` and delete the copied files).
