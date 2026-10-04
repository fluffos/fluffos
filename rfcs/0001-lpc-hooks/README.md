# RFC 0001: In-game hooks — eBPF-style join points for LPC

| | |
|---|---|
| Status | Draft v3 — six-angle review (v2) plus a game-design round (v3); see [§12](#12-review-log). Maintainer decisions in [§11](#11-decisions) |
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
* **One privileged hook daemon is the only subscriber, supplied by the master**
  (master apply `get_hook_daemon()`), the way the master names the
  simul_efun object. The driver delivers events to it and nothing else, so
  there is exactly one place where hook code runs with the driver's trust.
* **The daemon can observe and filter calls.** Beyond the rare events, it
  attaches `hook_*` probes to `call_other` and to selected efuns, with C-side
  filters, and can deny matching calls (`HOOK_DECIDE`).
* **It is also a gameplay tool.** The same probes, extended with a `function`
  join point (every call route, including local calls), per-object targeting
  and before/after/around advice with deterministic priorities, give mudlibs
  what they have faked with `shadow()` for thirty years: curses, protections,
  polymorph, world events and achievements over legacy content (§2A, §5.7).
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
| 2 | Call observation and filtering, available only to the daemon (`hook_attach()` etc.): `call_other` and opted-in `efun` probes that observe or deny, with per-program gating, pointer-keyed filters, `count`/`time` actions and positional handler arguments. **In scope** (maintainer decision) | S2 (runtime), S4, S6, S9–S12 |

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

## 2A. Game design: aspects for gameplay

S1–S12 are administrative. The larger use is gameplay: LPMud games have
always needed cross-cutting, temporary, per-instance behaviour (curses,
protections, polymorph, world events) and have faked it with `shadow()`.

### 2A.1 What real mudlibs do today

Grepped from public mudlibs (shallow clones; paths relative to each lib):

| Lib | Mechanism | Evidence | Pain points it shows |
|---|---|---|---|
| Discworld | 33 gameplay `shadow()` sites: speech curses, invulnerability, subdued/"dead" states, team XP, swimming, polymorph, room day/night and terrain, an effects framework that clones one shadow per effect | `lib/std/curses/*.c`, `lib/std/shadows/misc/{offler,surrender,harry,death,team,water}_shadow.c`, `lib/std/effect_shadow.c`, `lib/std/basic/effects.c:707-716` | Local calls bypass shadows, so `std/living/combat.c` carries **76** `this_object()->` self-calls just to stay shadow-visible (`effects.c:126-145` walks the shadow chain by hand); shadows don't survive login/restore (`effects.c:789-830` re-clones them; curses re-attach in `player_start()`); stacking order = attach order; signature drift (`offler_shadow.c` rev 1.2); reentrancy bug when adding an effect inside another (`effects.c:14-15`); display leak (`water.c` rev 1.11); a security blacklist of shadowable functions (`global/player.c:1789-1800`: `query_name`, `query_creator`, `dest_me`, `save_me`) |
| Dead Souls | `LIB_SHADOW` objects for polymorph/disguise (`shadows/bear.lpc`, `arbiter.lpc`), damage shields, breathing gear, zombie curse, traps; `shadows/diag.lpc` is a pure observe probe | `lib/lib/shadow.lpc:8-37`, `lib/shadows/*.lpc` | Its own shadow registry because the driver can't enumerate shadows per object; manual dedup by `base_name`; parser cache refresh after polymorph; `lib/body.lpc` has 75 self-calls |
| Lima | Gave up on shadows (`valid_shadow` defaults to deny) for a cooperative `add_hook`/`call_hooks` registry: **205** call sites hand-placed in base classes (`block_<dir>`, `prevent_combat`, `str_bonus`, `person_arrived`…) | `lib/std/object/hooks.c:30-153`, `std/modules/m_bodystats.c:259-262` | Only works where a base class anticipated the hook; removal needs the identical funptr; dead owners purged lazily |
| nt7 (Chinese lib) | Per-instance loot effects: `shadow(new(arg->weapon_effect))` at weapon creation; a shadow that blanks `init()`; 100+ per-heartbeat condition handlers | `inherit/self/weapon/*.lpc:98-101`, `shadow/no_init.lpc:5`, `feature/condition.lpc:88-130` | Temp buffs undone by hand (`add_temp(..., -x)` in 53 skill files) |

What the evidence asks for: interception of **local** calls (no shadow has
it), **per-instance** targeting that **stacks deterministically**, removal
handles and auto-cleanup, a documented re-attach path across login/restore,
per-object introspection ("which effects are on this player?"), and a trust
deny-list of advisable functions.

### 2A.2 Gameplay use cases

Verdict: **Hook** = cross-cutting advice is the right tool; **Retrofit** =
right where the base class cannot be edited (legacy content), otherwise an
explicit mudlib API is better; **No** = use the mudlib.

| # | Use case | Join point & filter | Kind | Rate on a 300-player mud | Verdict |
|---|---|---|---|---|---|
| G1 | **Curse of Butterfingers / Silence / speech curses** on one player | `function` `wield`/`cast`/`do_say`, object = victim | deny / around (rewrite text) | low | Hook (replaces DW curse shadows; local calls covered) |
| G2 | **Protection & states**: invulnerable at a temple, subdued, "you are dead" | `function` `adjust_hp`/`do_death`/`attack_by`, object = player | around (clamp, skip) / deny | 300–600/s mud-wide, only flagged players pay | Hook — the DW shadow family; local calls are the main win |
| G3 | **Vulnerability / damage shield / thorns** | `function` `receive_damage`, object = victim | around / after (result, attacker = `caller`) | as G2 | Hook over legacy combat; Retrofit if the lib has a damage pipeline |
| G4 | **Vampiric enchant on *this* heirloom sword** | `function` `hit`/`query_damage`, object = sword | after | ~1 sword | Hook — per-instance, keeps the item's identity |
| G5 | **Polymorph / disguise** ("everyone sees a frog"; false name that logs and `/secure/` still see through) | `function` `query_short/long/race/cap_name`, object = player, exclude `caller_program` `/secure/` | around | ~100/s looks | Hook — stacking + viewer discrimination shadows can't do |
| G6 | **Charm monster** | `function` `heart_beat` + deny `attack_ob` on the charmer, object = NPC | around / deny | heart_beat ~1000/s mud-wide | Hook; stress-tests per-object gating |
| G7 | **Escort quest / follow the leader** | `efun:move_object`, object = escorted NPC / leader | after (state *after* the move) | moves ~100/s, flagged objects pay | Hook — `move_object` is the only chokepoint; replaces heart_beat polling |
| G8 | **Blood Moon / seasonal overlay** (undead hit harder; frozen lake gains an exit) | `function` `query_damage` `defined_in /std/undead`; `query_exits` `target /d/lake/` | around | 50–300/s | Hook (live-ops, expires, no recompile); permanent rules move to code |
| G9 | **Regional pricing / faction tax** (shops in orc-held towns pay 1.3×) | `function` `query_value`, `caller_program /std/shop/` + `caller /d/orclands/` | around | ~2/s | Hook — a caller × callee rule no single API expresses |
| G10 | **Achievements & quests over legacy content** ("kill 100 orcs", "visit every room in Arnor") | `function` `die` `defined_in /std/monster`; `efun:move_object` `caller_program /std/player` | observe (deferred) | 10–100/s | Retrofit → feeds a mudlib event bus; one attachment per (program, function), fan-out in LPC |
| G11 | **Bounty, reputation, gossip, city guards** reacting to deeds anywhere | `function` `die`/`steal`/`attack` | observe (deferred, batched) | 10–100/s | Retrofit → bus |
| G12 | **NPC/LLM event stream**, economy simulation | as G11; `count`/`time` for trade volumes | observe (deferred, batched) | up to hundreds/s | Hook as a *source* only with batched delivery |
| G13 | **A/B balance experiments** | around on damage/exp formulas; per-object flag = cohort | around | 100s/s | Hook (with an explain view) |
| G14 | **Live invariants** ("gold is conserved") and **deterministic replay** on a dev server (around `efun:random` with a seeded stream) | `function` on every money-mutating function, local calls included; `efun:random` | observe / around | dev only | Hook — must see every route |
| G15 | **Suppress an apply for one object** (nt7's blank-`init()` shadow) | `function` `init`, object = target | around (no proceed) | low | Hook, via the apply allow-list (§5.7) |

**Where hooks are the wrong tool (verdict No):** double-XP weekends and
permanent stat stacking (one chokepoint or a modifier system owns display,
dispel and save); permadeath (identity belongs in the class); invisibility
through `id()` (a driver-questioned apply, AGENTS.md §13.24); builder
soft-code triggers, player housing and shop policies (owner *data* checked by
the room/shop that already receives the event); instancing/phasing;
accessibility rewriting and translation (`receive_message`/`catch_tell`);
verb blocking (parser-cooperative today, and it works).

**The pattern.** Hooks win when the callee is legacy or spread over many
files, when the effect depends on *who calls*, when it is per-instance and
temporary, or when the only chokepoint is an efun. They lose where the mudlib
already owns a single chokepoint, or the behaviour is permanent.

**Trust.** Hooks are a system / live-ops / game-designer instrument run by
the daemon. Builder- and player-facing features are exposed by the daemon as
**rule data** (zone prefix, program, function, verdict, expiry), never as
builder code running in a hook frame.

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
string|object get_hook_daemon();   // e.g. "/secure/hookd"; absent or 0 = hooks off
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
int hook_attach(string point, mixed action, mapping spec, int flags);
                              // spec: filter keys + "label", "priority", "expires" (§5.7)
void hook_detach(int id);
mapping hook_query(int id);   // rows, events, dropped, errors, last_error
void hook_reset(int id);
mapping *hook_list(object ob); // attachments that apply to ob (§5.7)
```

Only the daemon may call these (an error otherwise). The daemon exposes its
own, scoped API to wizards.

| Point | Fires | Site |
|---|---|---|
| `call_other` | a resolved `call_other`, before the function runs (so `HOOK_DECIDE` can deny) | `apply_low()`, after lookup, before `push_control_stack()`, `ORIGIN_CALL_OTHER` only |
| `function` | entry to a hooked function by any route (local, inherited, `call_other`, funptr, `call_out`, `heart_beat`) — added for game design, §5.7 | function entry in the interpreter, per-function gate |
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
| `object` | one target object instance (`O_HOOKED` bit; §5.7) |
| `caller_object` | calls made by one object instance (§5.7) |
| `exclude` | a mapping with the same keys, matched after the include keys |

**Actions.** `"count"` (rows keyed by caller program, target program,
function), `"time"` (count + summed and max eval microseconds, for S9), or an
LPC function. **Flags.** `HOOK_DECIDE` lets a handler deny (`call_other`,
`efun:*`): it returns `1` to deny with a generic error, or a string to deny
with that message; anything else (0, a promise, other types) allows.
`HOOK_FAIL_CLOSED` makes a handler error or budget overrun deny instead of
allow, for fences that must not fail open.

**Handler signature (positional, not a mapping).** Building a context mapping
costs ~200 ns per event; positional arguments do not:

```lpc
mixed handler(int id, object actor, object caller, object target, string fn, mixed *args);
// HOOK_AFTER adds `mixed result`; HOOK_AROUND adds `function proceed` (§5.7)
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
| Detach on the first "stale function pointer" or owner-destructed error; otherwise after `hook max errors` (default 10). `HOOK_FAIL_CLOSED` attachments never auto-detach; the daemon is told via `hook_detached(id, reason)` | §3 dangling funptrs; a fence that silently disappears |
| Attachments, handler funptrs and filters marked in `checkmemory.cc` (`mark_funp`, after `mark_call_outs()`'s pattern); count tables are C++ containers bounded by `hook max rows` with a `dropped` counter | §3 off-graph references; unbounded memory from attacker-chosen names |
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

### 5.7 Game-design extensions to Layer 2

The §2A cases need four things the administrative design lacks. All are
daemon-only, like the rest of Layer 2.

**1. A `function` join point** — entry to a function defined in a hooked
program, by *any* route: local call, inherited call, `call_other`, function
pointer, `call_out`, and the driver's cached `heart_beat` dispatch. This is
what no shadow and no `call_other` probe can do (Discworld's 76 self-calls
exist only to work around it).

* Gated **per function**, not per program: a `FUNC_HOOKED`-style bit on the
  function's flags, set at attach and recomputed when the program is
  compiled. A local call costs ~45 ns, so probing every local call of a hooked
  program (5–10 ns) would cost that program 10–20%; with a per-function bit,
  unhooked functions in a hooked program pay one predicted branch.
* Site: function entry in the interpreter (`setup_new_frame()` / the call
  opcodes), so it covers every route at one place.

**2. Per-object targeting.** Filter keys `object` (this target instance) and
`caller_object` (calls made *by* this instance, for tutorials and A/B cohorts),
backed by an `O_HOOKED` bit in `object_t::flags`. A hooked function called on
an unflagged object pays one more predicted branch; only flagged objects reach
the attachment table. Auto-detach when the object is destructed, reported as
`hook_detached(id, "target_destructed")`.

**3. Advice kinds and their order.** `flags` selects one kind:

| Kind | Handler | Can change the outcome? |
|---|---|---|
| `HOOK_BEFORE` (default) | `void h(int id, object actor, object caller, object target, string fn, mixed *args)` | No |
| `HOOK_AFTER` | same, plus `mixed result` | No — observes the result (thorns, vampiric sword, escort, follow) |
| `HOOK_DECIDE` | as `HOOK_BEFORE`, returns 1 or a message to deny | Deny only |
| `HOOK_AROUND` | as `HOOK_BEFORE`, plus `function proceed` | Yes: call `proceed(args...)` with the same or changed arguments, change its result, or skip it |

`actor` is `this_player()` at the intercepted call: handlers run with
`this_player()` = 0 (§5.4), but "who is doing this to whom" is what most game
effects need.

Order at one join point is total and deterministic: every attachment has a
`priority` (default 0; ties broken by attach id). The chain is
**deciders → AROUND (lower priority = outermost) → BEFORE → original
→ AFTER**, then the AROUND handlers unwind. A deny short-circuits everything
after it. Each AROUND handler receives the arguments as transformed by the
handlers outside it. The chain is snapshotted per invocation, so attaching or
detaching inside a handler affects the next call, not this one (the Discworld
effects reentrancy bug).

AROUND contract:
* Arguments arrive as `mixed *args`, so advice does not break when the target
  function's signature changes (Discworld's signature-drift bug).
* The value it returns is checked against the function's declared return type;
  a mismatch, a promise, a handler error or an eval overrun all mean **run the
  original unmodified** and count an error. A broken curse must never make a
  sword unwieldable or skip `die()`'s corpse logic.
* AROUND, AFTER and DECIDE are refused on driver-questioned applies (the
  generated `object_applies_table` names, `id`, `catch_tell`, every `valid_*`)
  except an allow-list (`heart_beat`, `init`), and on functions a mudlib marks
  protected (the daemon's deny-list, after Discworld's `player.c` blacklist:
  `query_name`, `query_creator`, `save_me`, money/exp/auth functions).

**4. Lifetime, state and introspection.**

* `hook_attach(string point, mixed action, mapping spec, int flags)`: `spec`
  holds the filter keys plus `"label"` (required), `"priority"` and
  `"expires"` (seconds; driver-side expiry so 300 status effects are not 300
  `call_out`s that leak on daemon reload; **not allowed with
  `HOOK_FAIL_CLOSED`**, so a fence cannot lapse silently). Per-effect state
  rides on the handler as bound arguments: `(: fumble, ([ "tries": 0 ]) :)`.
* `hook_list(object ob)` lists every attachment that applies to `ob` (object
  filters and program/function filters), with label, owner, expiry, hit count
  and handler eval time. The reference daemon renders it as a visible
  **status-effects list** for players and builders, which replaces Dead Souls'
  shadow registry and Discworld's `sh_adows` tool, and answers "why did that
  rat hit me for 40?". Error traces and `call_stack()` mark advised frames
  (`[hook #12 blood_moon]`).
* Hooks are runtime state: nothing survives a reboot, and per-object effects
  do not survive the target's destruct-and-reload. The daemon re-attaches them
  from its own saved effect data at login and `restore_object()` (the
  precedent is Discworld's `player_start()` / `init_after_save()`), using
  `object_created` to notice new objects.

**5. Deferred, batched observers** (`HOOK_DEFERRED`, with `HOOK_BEFORE` or
`HOOK_AFTER` only). Events go into a per-attachment ring buffer and are
delivered once per gametick as `h(int id, mixed *events)`, with object
arguments re-validated (destructed → 0) and overflow counted in `dropped`.
One LPC frame per batch instead of per event makes G10–G12 affordable, and it
is the only correct way to feed an `async` consumer (an LLM NPC, an economy
simulation), since an inline handler that returns a promise is declined.

**Cost gate additions** (§5.6): local call with nothing attached; local call
to an unhooked function in a hooked program; hooked function on an unflagged
object; one AROUND per call on a flagged object; 100 cursed players among 300;
one charmed orc among 2,000 `/std/monster` heart_beats.

**Deferred or rejected here.** Environment/room-tree filters (an LPC
`environment()` walk in the handler is ~100 ns; add only if measured hot);
`call_limited(fp, eval, args...)` for a daemon fanning out to many subscribers
under one budget (LDMud `limited()`; a small separate RFC); output/message
transform probes and per-player phasing (rejected, see §2A.2).

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
`HOOK_DECIDE | HOOK_FAIL_CLOSED` probe).

## 8. Implementation plan

Each phase is one PR, reviewed and merged before the next.

| Phase | Scope | Tests | Done when |
|---|---|---|---|
| 0 | Docs only: `docs/concepts/general/interposition.md` covering `valid_object`+`on_destruct`, `valid_write`, the tracer, `error_handler`, `valid_override`, and the transparent-wrapper idiom with its cost | — | merged |
| 1a | `get_hook_daemon` master apply; daemon resolution and reload; `g_hook_mask` + bit caching; the contained-invocation primitive; `call_other_miss` in `apply_low()` | LPC: miss via `->`, explicit, array, funptr; each reason; `undefined` preserved when declined; result when claimed; promise/error = declined; no event from master/simul/daemon callers; daemon reload keeps working. GTest: eval-state restore after a handler overrun. Benchmark gate §5.6 | closes #1414 |
| 1b | `object_created`, `object_destructed` with cause | LPC: driver destructs (shadow, environment contents, refused load) each reported once with the right cause; self-destructing `create()` not reported as created; Debug `check_memory()` clean | merged |
| 2a | `hook_*` efuns, filters, `PROG_HOOKED`, pointer-keyed set, `count`/`time`, `hook max rows`, `call_other` point | LPC: filter semantics table above; exclude; `HOOK_DECIDE` deny with message; `HOOK_FAIL_CLOSED`; re-entry guard (a handler's `write_file` seen by another probe); detach inside handler. Benchmark gate including attached cases | merged |
| 2b | `efun:*` opt-ins with `HOOK_EFUN()`, argument re-validation | LPC per efun group; a handler destructing an efun argument gets a clean error | merged |
| 3 | Reference daemon and `docs/concepts/general/hooks.md`, efun and apply pages, `include/hooks.h` | the reference daemon runs in the testsuite | merged |
| 4a | Game design: `function` join point with per-function gating; `object`/`caller_object` filters with `O_HOOKED`; `HOOK_BEFORE`/`HOOK_AFTER`; `actor` argument; auto-detach on target destruct | LPC: **local, inherited, funptr, `call_out` and `heart_beat` routes all observed**; unflagged instances untouched; target destruct detaches. Benchmark: local-call cases in §5.7 | merged |
| 4b | `HOOK_AROUND` with `proceed`, priorities and the deterministic chain; return-type check; apply allow-list and protected-function deny-list | LPC: G1 curse, G2 clamp (skip `proceed`), G5 disguise with `/secure/` excluded; stacking of two AROUNDs is order-independent of attach order given priorities; error/overrun/type mismatch runs the original unmodified; attach/detach inside a handler affects only the next call | merged |
| 4c | `expires`, `hook_list(object)`, labels in traces, `HOOK_DEFERRED` batching | LPC: expiry fires `hook_detached(id, "expired")` and is refused with `HOOK_FAIL_CLOSED`; batched delivery revalidates destructed objects; G10 achievements fed through a deferred observer | merged |
| 4d | Reference **effects daemon** on top of the hook daemon: status-effects view, re-attach at login/restore, rule-data API for builders | the testsuite runs G1, G2, G7 and G8 end to end through it | merged |

## 9. Security invariants (each has a test)

1. No event fires when the caller is the master, the simul_efun object or the
   daemon; during compile; inside `valid_*` or `error_handler`.
2. `hook_*` called by anything but the daemon errors.
3. A Layer 1 apply returning a promise, an error, or anything but 1 changes
   nothing.
4. `efun:*` probes observe `efun::` calls, and calls made by non-daemon code
   that a probe handler invoked; a probe never observes its own handler.
5. `HOOK_FAIL_CLOSED`: handler error or overrun denies; the attachment
   stays; `hook_detached` is not called for it.
6. `this_player()` is 0 inside Layer 1 applies and probe handlers, and the
   caller's `this_player()` is restored afterwards.
7. Count tables never exceed `hook max rows`; overflow is counted.
8. Filter canonicalization: `/secure/login`, `/secure/login.c` and
   `/secure/login#3` (as `target`) match the same attachments.
9. A `function` attachment observes every call route (local, inherited,
   `call_other`, funptr, `call_out`, `heart_beat`), and only on the filtered
   object instance when `object` is given.
10. AROUND/AFTER/DECIDE on a driver-questioned apply outside the allow-list,
    or on a deny-listed function, is refused at attach.
11. A failing AROUND handler (error, overrun, promise, wrong return type)
    leaves the call behaving exactly as if nothing were attached.

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

## 11. Decisions

Decided by the maintainer:

1. **The hook receiver is supplied by the master**: master apply
   `string|object get_hook_daemon()`, consulted at boot and after a master
   recompile. No config line.
2. **Names stay `hook_*`**: `hook_attach`, `hook_detach`, `hook_query`,
   `hook_reset`, `HOOK_DECIDE`, `HOOK_FAIL_CLOSED`, daemon callback
   `hook_detached`. (Mudlibs that already use `hook` as vocabulary, e.g.
   Lima's `add_hook`/`call_hooks`, are unaffected: these are efuns callable
   only by the daemon, not names a mudlib defines.)
3. **The daemon must be able to observe and filter calls**: Layer 2
   (`call_other` and `efun:*` probes, observe and `HOOK_DECIDE`) is part of
   this RFC, not a later option.
4. **Game design is in scope** (§2A, §5.7): `function` join point,
   per-object targeting, `HOOK_BEFORE`/`AFTER`/`DECIDE`/`AROUND` with
   priorities.

Settled by the game-design review: driver-side `expires` is allowed (status
effects are the dominant use and 300 daemon `call_out`s leak on reload) but
refused with `HOOK_FAIL_CLOSED`, so a fence can never lapse silently.

Still open:

5. A destruct veto (LDMud `prepare_destruct`)? Not proposed.
6. Whether the auto-object RFC (§10 A) should be written before or after
   this one is implemented. Not a dependency.
7. Should `HOOK_AROUND` ship at all, or stop at `HOOK_AFTER` until a
   prototype measures its per-call cost (it allocates a `proceed` closure per
   advised call)?

## 12. Review log

Draft v1 was reviewed from six angles; the main changes:

| Review | Main finding | Change |
|---|---|---|
| Evidence | P1/P3 came from a careless wrapper; a `bind()`-based wrapper keeps the caller and composes with shadows. S2, S3, S5 and S8 are already served | §3 rewritten; case rests on cost and on events LPC cannot see |
| VM safety | `call_other` miss and call cannot be at `f__call_other()` (arguments already popped, reason unknown, array/funptr routes missed); handler eval overruns leak into the caller; `efun:*` handlers can invalidate checked arguments; `error` at throw time already exists as `error_handler` | Sites moved into `apply_low()`; §5.4 primitive; argument re-validation; `error` join point dropped |
| Security | Argument disclosure, audit bypass via a global re-entry guard, fail-open fences, decide hijacking | One privileged daemon; per-attachment re-entry; `HOOK_FAIL_CLOSED`; hard exemptions; narrow decide |
| Mudlib API | Filter semantics undefined; mode was a filter key; count keys used arrays; `update` dropped attachments; missing scenarios | Filter table; flags; row-shaped `hook_query`; daemon resolved by path; S9–S12 |
| Performance | Detached cost below measurement floor; a ctx mapping costs ~200 ns per event; "< 1% in 5 runs" is unmeasurable | Positional handlers; `PROG_HOOKED` + pointer-keyed set; cachegrind gate |
| Prior art | DGD `callCritical` is unlimited, not budgeted; `valid_override` already closes `efun::`; missed LDMud `limited()`, `trace()`, `runtime_error`, `prepare_destruct` and kernellib's object-manager hooks | §4 corrected; daemon modelled on kernellib's manager |
| Game design (3 researchers: designer, real mudlibs, emergent/player-facing) | Real libs fake aspects with shadows (Discworld 33 sites, 76 self-calls to defeat the local-call gap; Lima's 205 hand-placed hook points); gameplay needs local-call interception, per-instance targeting, after/around advice, deterministic stacking, expiry, per-object introspection and batched observers; builder-facing features must be rule data, not code | §2A, §5.7, phase 4 |

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
