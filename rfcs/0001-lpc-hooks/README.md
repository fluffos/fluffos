# RFC 0001: In-game hooks — eBPF-style join points for LPC

| | |
|---|---|
| Status | Draft, for discussion |
| Issues | #1414 (call_other miss report) is the first consumer; motivates a general mechanism instead of one-off driver features |
| Prior art studied | LDMud `set_driver_hook()` / `H_DEFAULT_METHOD` / Python hooks; DGD auto object, driver-object applies, `call_touch()`, `rlimits`, `atomic` |
| Evidence | Probes in [`probes/`](probes/), run against master `b5714e5f` (RelWithDebInfo, gcc, 8 cores) |

## 1. Summary

Mudlibs keep asking the driver for small, cross-cutting features: report a
`call_other` to a missing function (#1414), count calls per function, audit
file writes, track object lifetimes, veto a dangerous efun for some objects.
Each one, done as a driver feature, adds a config line, a master apply and a
check on a hot path. This RFC proposes **one** mechanism instead: a fixed set of
driver-defined **join points** to which mudlib code can **attach programs** at
runtime. The model is borrowed from eBPF:

| eBPF | This RFC |
|---|---|
| kernel tracepoints / kprobes | driver join points (`call_other.miss`, `object.destruct`, `efun.write_file`, ...) |
| BPF program | an *action*: a built-in C action (`count`, `log`) or an LPC handler |
| verifier: bounded, can't crash the kernel | runtime guarantees: own eval budget, no re-entry, errors contained, fail-closed returns |
| in-kernel filter | a C-side filter (program, function, caller) evaluated before any LPC runs |
| BPF maps | driver-side aggregation tables, read from LPC |
| attach / detach at runtime, zero cost when detached | `hook_attach()` / `hook_detach()`, one predicted branch per join point when nothing is attached |
| CAP_BPF | master `valid_hook()` |

The mudlib then builds the policy (aspect-style "advice") in LPC, which is
where policy belongs.

## 2. Scenarios

These are the concrete mudlib problems the design is checked against. Each
names who wants it and what "done" looks like.

| # | Scenario | Need |
|---|---|---|
| S1 | **Typo / dead-hook finder** (#1414). On a dev server, list every `call_other` to a function that doesn't exist, is `private` or `static`, once per (caller program, target program, function). | Observe misses; dedup; zero cost on normal calls. |
| S2 | **Hot-function profile.** "Which daemon functions are called most, by whom?" without editing any object. | Observe calls, filtered to some programs; aggregate cheaply. |
| S3 | **File-write audit.** Log every `write_file`/`rm`/`rename` by wizard-owned objects, including calls written as `efun::write_file`. | Observe efun calls; cannot be bypassed by the code being audited. |
| S4 | **Capability fence.** Objects under `/domains/` may not `call_other` into `/secure/`. | Decide (deny) on a call before it runs; caller identity must be the real caller. |
| S5 | **Lifecycle accounting.** Live object count per program, leak detection: every create and every destruct, including driver-initiated ones. | Observe create/destruct for all objects, whatever they inherit. |
| S6 | **Deprecation warnings.** Warn, with the caller, when `/std/old_combat::attack()` is called. | Observe one function, filtered in C. |
| S7 | **Default method / proxy.** A remote-object proxy answers any function it does not define (LDMud's `H_DEFAULT_METHOD`). | Decide: supply a result for a miss. |
| S8 | **Error telemetry.** Count runtime errors per program, caught or not, without changing `error_handler`. | Observe errors at throw time. |

Out of scope on purpose: **hot-patching code** (that is `recompile_object()`,
see `docs/concepts/general/hot_reload.md`) and **per-opcode tracing** (that is
the tracer and the debugger in #1286).

## 3. What FluffOS has today, measured

FluffOS already has four interposition mechanisms: **simul_efun overrides**
(with `efun::` to bypass), **shadows**, the master's **`valid_*` applies**, and
the master's **`error_handler`**. Before proposing a new one, the hypothesis
"these are enough" was tested with probe objects run on the real driver
([`probes/`](probes/); the simul_efun additions are in
[`probes/simul_efun_additions.lpc`](probes/simul_efun_additions.lpc)).

### 3.1 Probe results

| Probe | Question | Result |
|---|---|---|
| H1 | Does a simul_efun named `call_other` see `ob->fn()`? | **Yes.** It sees arrow calls, explicit `call_other()`, `(: call_other, ... :)` and array targets. |
| H2 | Can it detect a miss? | **Only by paying for it on every call.** The wrapper must run `function_exists()` (84 ns) on every call to find the rare miss. |
| H4 | Does `efun::call_other` go through it? | **No.** `efun::` bypasses the advice completely (counts `0, 0`). Same for `efun::destruct` (P2). |
| P3 | Is the wrapper transparent? | **No.** Inside the callee, `previous_object()` becomes `/single/simul_efun` instead of the real caller. Every `previous_object()`-based permission check in the mudlib silently changes meaning. |
| P1 | Does it compose with shadows? | **No.** A shadow that forwards the idiomatic way (`tgt->fn()`) recurses until `Too deep recursion`: the forward now comes from the simul object, not the shadow, so the driver routes it back into the shadow. |
| H3 | What do shadows see? | External calls **and** driver applies (`id()` via `present()` was intercepted). Not local calls inside the object. Not misses. One shadow per object, so S5 needs a shadow on every object. |
| P2 | Does a simul `destruct()` see every destruction? | **No.** It saw 1 of 2: the shadow destructed along with its target is driver-initiated and invisible. |
| H5 | What does LPC-level advice cost? | Direct `efun::call_other` 127 ns/call; through a simul wrapper 391 ns; arrow into a shadowed object 528 ns. LPC advice on every call roughly **triples** call cost. |
| — | Can advice be attached at runtime? | **No.** Simul resolution is decided at compile time (`lookup_ident(...)->dn.simul_num` in `grammar_rules_exprs.cc`), so code compiled before an override never sees it, and removing one means recompiling every caller. |
| — | Errors? | Covered, partly. With `mudlib error handler` set, caught errors also reach master `error_handler(map, 1)` (`simulate.cc`). There is only one handler (the master), so S8 means editing the master. |

### 3.2 Scenario coverage today

| Scenario | Simul override | Shadow | Master apply | Verdict |
|---|---|---|---|---|
| S1 typo finder | Explicit/arrow calls only, at ~3x cost on every call, breaks `previous_object()` (P3) and shadows (P1) | No | No | **Not acceptable** |
| S2 profile | Same cost and breakage; misses local calls, which is fine for S2 | One per object | No | **Not acceptable** |
| S3 audit | Bypassed by `efun::` (H4) | No | `valid_write` sees writes, but per-path, not per-call-site | Partial |
| S4 fence | Breaks `previous_object()` (P3), the very thing a fence checks | Per object | No | **No** |
| S5 lifecycle | Misses driver-initiated destructs (P2); `create` is an apply, not an efun | One per object | No | **No** |
| S6 deprecation | Not for lfuns | One per object | No | **No** |
| S7 default method | — | Shadow can define the names it knows, not unknown ones | No | **No** |
| S8 errors | — | — | `error_handler` | Yes, master-only |

The hypothesis is refuted: the existing tools cover S8 and part of S3. The
others are either impossible or achievable only by giving up caller identity
and paying ~3x on every call.

## 4. Prior art

Detailed, sourced notes: LDMud from `github.com/ldmud/ldmud` HEAD `8afa5f50`
(3.6.8); DGD from `github.com/dworkin/dgd` `733ea01`, `kernellib` `e602afe`,
`cloud-server` `7959357`, `lpc-ext` `0e299c8`.

### 4.1 LDMud

- **`set_driver_hook(hook, value)`** stores one handler per hook in a flat
  global array of 32 slots (`mudlib/sys/driver_hook.h`, `src/simulate.c`).
  Setting one is gated by `privilege_violation()` unless the caller is the
  master or simul_efun object. Values are sealed: strings interned, arrays
  copied, unbound lambdas bound to the master.
- Most hooks are **policy slots**, not observation: `H_MOVE_OBJECT` performs
  the move, `H_LOAD_UIDS`/`H_CLONE_UIDS` assign uids, `H_CREATE_*`/`H_RESET`/
  `H_CLEAN_UP` choose which lfun runs, `H_MODIFY_COMMAND`/`H_COMMAND` replace
  the parser, `H_INCLUDE_DIRS`/`H_AUTO_INCLUDE`/`H_FILE_ENCODING` steer the
  compiler, and `H_DEFAULT_PROMPT` / `H_MSG_DISCARDED` / `H_TELNET_NEG` and
  friends steer I/O.
- **`H_DEFAULT_METHOD`** is the direct analogue of S1/S7. It runs only after
  normal lookup **and** the shadow chain fail (`int_apply()`,
  `src/interpret.c`), so successful calls pay nothing; a miss pays one
  type-tag test when unset. Protocol: `fn(&result, ob, fun, args...)`;
  return 0 = declined, non-zero = handled, value in the by-ref `result`.
  `call_direct*` exists solely to bypass it. Master, simul_efun and
  lightweight-object calls are exempt. Two lessons from its history: it also
  fires for driver applies (`id`, `catch_tell`, `__INIT`), which the docs never
  say; and it has had a closure-bound-to-destructed-object bug (Mantis #523)
  and a leak when the handler raised an error (`test/t-errors.c`).
- **Python hooks** (`ldmud.register_hook`): `ON_OBJECT_CREATED`,
  `ON_OBJECT_DESTRUCTED`, `ON_HEARTBEAT`, signals, and (3.6.8)
  `BEFORE_INSTRUCTION`. Multiple subscribers per hook, observe-only, an
  exception is printed and the next handler runs. Only operator-trusted Python
  can register; LPC cannot. In a Python build, `BEFORE_INSTRUCTION` costs an
  out-of-line call per opcode even with no handler.
- **Error containment is per hook and accidental**: some hooks run under
  `secure_*` wrappers that swallow errors, others (`H_DEFAULT_METHOD`,
  `H_MOVE_OBJECT`, command hooks) propagate into the triggering operation.
  Hooks run on the caller's eval budget; no recursion guard was found.
- **Not offered**: generic before/after advice on arbitrary calls,
  multi-subscriber LPC hooks, per-object or per-program registration.

### 4.2 DGD

- **The auto object** is inherited by every object except the driver object.
  It can redeclare kfuns and reach the original with `::kfun()`
  (`src/comp/control.cpp`). This *is* around-advice on every kfun, woven at
  compile time, with no runtime dispatch cost. The kernel library builds its
  whole security and resource model this way: `destruct_object`,
  `compile_object`, `clone_object`, `call_out`, `read_file`/`write_file`, ...
  each check, notify a manager, then call `::kfun()`. Create, destruct and
  compile notifications are LPC policy, not driver applies.
- `a->f()` compiles to a call to the name `call_other`, so an auto-object
  override would see arrow calls too (verified in the compiler source; not
  exercised by kernellib, which interposes through the driver object's
  `call_object()` instead).
- **`call_touch(obj)` / `driver->touch(obj, fn)`**: a per-object one-shot flag.
  The next call into the object fires `touch()` *before* function lookup (so
  even for a missing function); the flag is cleared before the hook runs, so
  it cannot recurse. Zero cost on unmarked objects. Used for lazy upgrades.
- **Containment primitives**: `rlimits (stack; ticks) { ... }` scopes
  resource limits, authorized by `runtime_rlimits()`; `atomic` functions roll
  back all state on error; `callCritical` runs `runtime_error`,
  `atomic_error` and `remove_program` with their own limits and contains
  their errors. `runtime_error` fires at throw time, before unwinding, with
  the catch depth and the full `call_trace()` still live.
- **No miss hook**: `call_other` to a missing function returns `nil`, and the
  kernel library relies on it (`call_other(obj, "???")` forces lazy
  creation). Native extensions (`lpc-ext`) may add kfuns but may not call
  LPC.
- What DGD would argue: most join points need no dynamic mechanism. Kfun
  advice is the auto object plus `::kfun`, compiled statically; lifecycle is
  policy in the auto object. The driver should only expose events that
  cannot be expressed at an LPC call site: missing-function dispatch, errors,
  touch, input.

### 4.3 What we take

| From | Take | Avoid |
|---|---|---|
| LDMud | Flat slot array per join point, so "detached" is one test. `H_DEFAULT_METHOD`'s placement (after normal lookup and shadows) and its by-ref result + 0/1 protocol. A `call_direct`-style escape. Master gating. Sealing attached values. | One handler per hook. Accidental per-hook error policy. Hooks charged to the caller's eval budget with no recursion guard. Scope that silently widens to driver applies. An always-on per-opcode call. |
| DGD | Contained execution (`callCritical`): own limits, errors never cascade. Error hooks at throw time with the trace live. The one-shot, clear-before-call `touch` discipline. A small surface of events that LPC cannot express itself. | Permissive return values (DGD's nil-is-deny matches AGENTS.md §13.24). |

DGD's argument is taken seriously in §8, Alternative A: FluffOS can get most
of the static kfun-advice benefit from an auto-inherit, and this RFC does not
add join points for things that an auto-inherit does well. What the auto
object cannot do, and what the scenarios need, is **runtime** attach/detach,
**observation that the observed code cannot bypass or must opt into**, and
**driver-initiated events** (misses, driver destructs, applies, errors).

## 5. Design

### 5.1 Principles

1. **Detached costs one branch.** Each join point checks a bit in a global
   `g_hook_mask` (a `uint64_t`), the same cost class as the existing
   `Tracer::enabled()` test in `apply_low()` and `DBG_LPC` in the interpreter
   loop. No allocation, no call, no lookup until something is attached.
2. **Filter in C before any LPC runs.** An attachment carries a filter (target
   program, function name, caller program; exact or prefix). The common case
   for S2/S6 is "this one function"; the cost of the 99.9% non-matching calls
   is a hash probe, not an LPC call.
3. **Built-in actions first.** `count` and `log` are C actions that need no
   LPC at all. S1 is `count` on `call_other.miss` keyed by (caller program,
   target program, function), read back with `hook_query()`. That is the eBPF
   "map" model, and it is what makes S1 free enough to leave on.
4. **LPC handlers run contained.** A handler runs in its own frame with its own
   eval budget, cannot re-trigger hooks, cannot leak an error into the
   triggering operation, and is auto-detached after repeated errors or when
   its owner is destructed (§5.5).
5. **The observed call is unchanged.** Advice runs *beside* the call, never
   *instead of the caller*: `previous_object()`, `origin()`, `this_player()`
   and shadow routing inside the callee are exactly what they would be with
   nothing attached (contrast P1/P3).
6. **Deciding is explicit and fail-closed.** Only `decide` join points may
   change an outcome. A decision is "deny" or "handled" only when the handler
   returns exactly `1`; anything else, including a promise (§13.24) or an
   error, is "no opinion".
7. **No new mechanism where an existing one is right.** Errors already reach
   `error_handler`; hot patching is `recompile_object()`. Join points are only
   added for events LPC cannot see.

### 5.2 Join points (phase 1)

| Join point | Fires | Kind | Context passed | Notes |
|---|---|---|---|---|
| `call_other.miss` | `call_other` / `->` found no callable function, after the shadow chain (same placement as LDMud's default method), at the single site in `f__call_other()` | observe, decide | caller, target, function, reason (`"undefined"`, `"private"`, `"static"`, `"protected"`), args | `decide` = supply a result (S7); default stays "return undefined" (#1414's requirement) |
| `call_other.call` | a `call_other` / `->` that resolved | observe, decide | caller, target, defining program, function, args | Filter required (program or function). `decide` = deny with an error (S4) |
| `efun.<name>` | before the efun body runs, for efuns that opt in (phase 1: `write_file`, `write_bytes`, `rm`, `rename`, `mkdir`, `rmdir`, `destruct`, `exec`, `shadow`, `socket_*`) | observe, decide | caller, args | Fires for `efun::` calls and simul-forwarded calls alike (S3). `decide` = deny with an error |
| `object.create` | after an object's `create()` returns (also for virtual and replaced objects) | observe | object, program, clone flag | S5 |
| `object.destruct` | at the start of `destruct_object()`, for every destruct, driver-initiated included | observe | object, program, cause (`"efun"`, `"shadowed"`, `"clean_up"`, `"shutdown"`, ...) | S5. No veto in phase 1 (LDMud has `prepare_destruct` in the master; a veto needs separate design) |
| `error` | at throw time, before unwinding (DGD's placement) | observe | error string, object, program, line, caught flag, trace | S8. Does not replace `error_handler` |

Deliberately not in phase 1: per-opcode (`BEFORE_INSTRUCTION`; that is #1286's
job), all applies (`apply.*` would fire on every `heart_beat`/`id`/`catch_tell`;
LDMud's silent widening is the warning), and `object.move`.

### 5.3 API

```lpc
// Attach. Returns a hook id (> 0) or throws. Gated by master valid_hook().
int hook_attach(string join_point, mixed action, mapping filter | void);

//   action:  "count"                      C-side aggregation only, no LPC
//            "log"                        one debug.log line per event (rate-limited)
//            function handler             LPC: void|int handler(mapping ctx)
//   filter:  ([ "program": "/std/room", "function": "query_exit",
//               "caller":  "/domains/*", "kind": "observe" | "decide" ])

void    hook_detach(int id);
mapping hook_query(int id);        // "count": ([ key: n ]); also errors, last error
mapping *hook_list();              // all attachments the caller may see
```

Handler context (`ctx`) is a fresh mapping per event, e.g. for
`call_other.miss`: `([ "caller": ob, "target": ob, "function": "query_skils",
"reason": "undefined", "args": ({ ... }) ])`. Arguments are **copies**, so a
handler cannot disturb the VM stack the triggering code is using.

For `decide` handlers: return `1` to deny (`call_other.call`, `efun.*`) or to
claim a miss (`call_other.miss`, with the result in `ctx["result"]`).
Anything else is "no opinion"; the call proceeds as if nothing were attached.

Master apply:

```lpc
int valid_hook(object who, string join_point, mixed action, mapping filter);
```

No `valid_hook()` in the master means **deny**, so existing mudlibs get no new
surface by upgrading the driver.

### 5.4 Ordering and composition

Several attachments may share a join point (LDMud's single slot does not
compose). Observers run in attach order. For `decide`, the first `1` wins and
later deciders are not asked; observers still run. A decision is reported to
observers in `ctx["decided_by"]`. Hooks never fire while a hook handler is
running (a single global "in hook" depth, like DGD clearing the touch flag
before calling `touch()`), so a handler's own `call_other`s and
`write_file`s are not observed. That keeps S2 profiles from counting the
profiler.

### 5.5 Safety rules (the "verifier")

These are runtime guarantees, enforced by the driver for every LPC handler.
Each one answers a hazard already catalogued in AGENTS.md §3, §4 and §13.

| Rule | Hazard it answers |
|---|---|
| Handler runs under `hook eval cost` (new config), not the caller's budget; on overrun the handler is aborted, not the caller | LDMud runs hooks on the caller's budget; a slow handler would kill innocent code (§13.23) |
| Errors are caught at the handler boundary (`callCritical`-style), logged, counted in `hook_query()`, and the attachment is auto-detached after `hook max errors` (default 10) | LDMud's per-hook accidental propagation; DGD's contained critical calls |
| `st_num_arg`, `sp`, `csp`, `current_object`, `current_prog`, `caller_type` and `call_origin` are saved and restored around the handler | §13.16 (`st_num_arg` clobbered by nested LPC), §4 (half-initialised stack slot) |
| Context arguments are copies; the handler never sees the live stack | §4, §13.14 |
| The owner object of a handler funptr is tracked; destructing it detaches the hook. Attached funptrs and filters are marked in `checkmemory.cc` | §3 (off-graph references), LDMud Mantis #523 |
| A handler that returns a promise, or is declared `async`, counts as "no opinion" and is logged once | §13.24 (driver consumers must not treat an unknown tag as permissive) |
| Join points sit only where the driver already runs arbitrary LPC or holds no C state across the call (`f__call_other`'s miss branch, the top of `destruct_object()`, after `create()`) | §13.14 (C state that survives a call into LPC) |
| No hook fires inside a hook handler, inside the master's `valid_hook()`, or during compile | Recursion; mid-compile applies (§8 Applies) |

### 5.6 Cost model and acceptance criteria

| State | Target | How it is verified |
|---|---|---|
| No attachment anywhere | No measurable change on `speed.lpc` and the H5 probe (< 1%, within noise) | A/B benchmark in the PR, RelWithDebInfo, 5 runs |
| Attached to `call_other.miss`, `count` | Successful calls unchanged; a miss costs one hash increment | H5 probe plus a miss-heavy loop |
| Attached to `call_other.call` with a function filter | Non-matching calls pay one hash probe (< 20 ns target); matching calls pay one contained LPC call (expect ~250-400 ns, see H5) | Benchmark with 1 and 100 filters |
| Handler error storm | Caller unaffected; hook auto-detached after `hook max errors` | Regression test |

### 5.7 How the scenarios map

| Scenario | Implementation |
|---|---|
| S1 (#1414) | `hook_attach("call_other.miss", "count")` at boot on dev; a wizard command prints `hook_query(id)`. No LPC runs per event; dedup is the count key. |
| S2 | `hook_attach("call_other.call", "count", ([ "program": "/daemons/*" ]))` |
| S3 | `hook_attach("efun.write_file", (: audit :), ([ "caller": "/w/*" ]))`, also seen for `efun::write_file` |
| S4 | `hook_attach("call_other.call", (: fence :), ([ "program": "/secure/*", "caller": "/domains/*", "kind": "decide" ]))`; `fence` returns 1 to deny; `previous_object()` inside `/secure/` is still the real caller if allowed |
| S5 | `object.create` and `object.destruct` with `count` keyed by program |
| S6 | `call_other.call` with `([ "program": "/std/old_combat", "function": "attack" ])` and `"log"` |
| S7 | `call_other.miss` with `"kind": "decide"` and a filter on the proxy's program |
| S8 | `hook_attach("error", "count")`, keyed by program and line |

## 6. Interaction with existing features

- **Simul_efuns** stay as they are. Efun join points fire inside the real
  efun, so they see `efun::` calls and simul-forwarded calls once each.
- **Shadows**: `call_other.call` fires once per external call, after shadow
  resolution, with `ctx["target"]` the object actually reached.
  `call_other.miss` fires only when the shadow chain also has no such
  function.
- **`recompile_object()`**: attachments are keyed by program *name*, so they
  survive a recompile; nothing caches a `program_t*`.
- **Async**: handlers are plain synchronous functions. `async` handlers are
  rejected at attach time (§5.5).
- **The debugger (#1286)** and the tracer keep their own bits in the same
  mask word, so all "is anyone listening" checks share one cache line.

## 7. Implementation sketch

| File | Change |
|---|---|
| `src/vm/internal/hooks.{h,cc}` (new) | join-point enum, `g_hook_mask`, attachment table, filters, `count`/`log` actions, contained handler invocation, `mark_hooks()` |
| `src/packages/core/efuns_main.cc` | `f__call_other()`: miss branch and resolved-call notification |
| `src/vm/internal/simulate.cc` | `destruct_object()` top; `error()` throw site; after `create()` in `load_object()`/`clone_object()` |
| opted-in efuns | one `HOOK_EFUN(...)` line each |
| `src/packages/core/hooks.spec` (new) | `hook_attach`, `hook_detach`, `hook_query`, `hook_list` |
| `src/vm/internal/applies` | `valid_hook` |
| `src/base/internal/rc.cc` | `hook eval cost`, `hook max errors` |
| `src/packages/develop/checkmemory.cc` | call `mark_hooks()` |
| tests | GTest for the dispatcher; LPC tests for each join point; the §3 probes turned into regression tests (caller identity unchanged, shadow forwarding works with a hook attached, `efun::` observed, driver destructs observed) |
| docs | `docs/concepts/general/hooks.md`, efun pages, `docs/apply/master/valid_hook.md` |

## 8. Alternatives considered

**A. A DGD-style auto object.** Add a config option naming an object that every
program implicitly inherits; it may redefine efuns and reach the originals with
`efun::`. This gives zero-cost, compile-time around-advice on efuns, keeps
`previous_object()` intact (the advice is a local call in the caller's own
program), and is a small driver change. It does not give runtime
attach/detach (every change recompiles the world), cannot see
driver-initiated events (misses, driver destructs, applies, throw-time
errors), and an object that calls `efun::write_file` still bypasses it, so it
does not solve S1, S3, S5 or S8. It is complementary rather than a replacement,
and worth a separate RFC.

**B. LDMud-style single-slot driver hooks** (`set_driver_hook`). Simple and
proven, but one handler per hook does not compose, and it has no filter or
aggregation, so S1/S2 run LPC on every event.

**C. More master applies, one per feature** (the #1414 proposal as written).
This is the status quo, and the reason for this RFC.

**D. External tracing only (USDT / bpftrace).** Zero cost and great for
operators, but invisible to the mudlib and Linux-only. It is complementary:
`hooks.cc` can emit a USDT probe at each join point at no extra cost.

**E. Compile-time aspect weaving** (a pragma that wraps matching functions
when they are compiled). Powerful, but it needs recompiles to change and is a
large compiler change; Alternative A gets most of the benefit for less.

## 9. Phasing

1. `hooks.{h,cc}` with `call_other.miss` and the `count`/`log` actions only;
   `hook_attach/detach/query`, `valid_hook`. Closes #1414 with no LPC per
   event. Benchmark gate per §5.6.
2. LPC handlers with the §5.5 rules; `object.create`, `object.destruct`,
   `error`.
3. `call_other.call` with filters, `decide` kind; `efun.*` for the
   file/destruct/exec/socket set.
4. Re-evaluate Alternative A (auto object) as a separate RFC.

## 10. Open questions

1. Should `efun.*` observation really be impossible to bypass, or should a
   privileged object be able to opt out (LDMud's `call_direct` is the
   precedent for an escape)?
2. Is `object.destruct` veto worth having (LDMud master `prepare_destruct`)?
   It is a new failure mode for `destruct()`.
3. Do we want per-object attachment (DGD `call_touch` style, a flag on the
   object) in addition to program/function filters?
4. Which name: `hook_*` (this draft), `probe_*`, or `advice_*`?
5. Should the master be able to see and detach every attachment
   (`hook_list()` returning all), regardless of owner?

## Appendix: reproducing the probes

```sh
cp rfcs/0001-lpc-hooks/probes/{target,plain,shadow,shadow_arrow}.lpc testsuite/clone/probe/
cp rfcs/0001-lpc-hooks/probes/zz_probe*.lpc testsuite/single/tests/
cat rfcs/0001-lpc-hooks/probes/simul_efun_additions.lpc >> testsuite/single/simul_efun.lpc
cd testsuite && ../build/src/driver etc/config.test -ftest:single/tests/zz_probe_hooks.lpc | grep PROBE
```

`zz_probe_hooks.lpc` needs `shadow.lpc` (forwarding with `efun::call_other`);
`zz_probe2.lpc` uses `shadow_arrow.lpc` (the idiomatic `tgt->fn()` forward) to
show P1. These edits are for the experiment only; do not commit them to the
testsuite.
