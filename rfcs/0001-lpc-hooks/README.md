# RFC 0001: In-game hooks — eBPF-style join points for LPC

| | |
|---|---|
| Status | Draft v10. Rewritten as one design after a second review round (consistency, scope, security, implementer dry run); history in [Appendix D](#appendix-d-review-log). Open decisions in [§9](#9-decisions) |
| Issues | #1414 (call_other miss report) is the first consumer |
| Prior art | LDMud `set_driver_hook()`, `H_DEFAULT_METHOD`, `limited()`, `trace()`, Python hooks; DGD auto object, driver-object applies, kernellib object/error managers, `call_touch()`, `rlimits`, `atomic` |
| Evidence | Probes on the real driver ([Appendix C](#appendix-c-reproducing-the-probes)); use cases read from real mudlibs ([Appendix A](#appendix-a-evidence-from-real-mudlibs)) |

## 1. Summary

Mudlibs keep asking for small cross-cutting driver features: report a
`call_other` to a missing function (#1414), fence a directory off from callers,
audit who changes money, account object lifetimes, and, in gameplay, curses,
protections and world events that today are faked with `shadow()`. This RFC
proposes one mechanism instead of one feature per request, split the way eBPF
is split:

* **The driver is the kernel.** It defines a small, fixed set of join points,
  makes them free when unused, runs handlers in a contained frame, and offers
  C-side filtering, counting and batching for high-frequency events.
* **Authorization follows the `valid_*` pattern, with tiers the driver
  computes.** Any object may call `hook_attach()`. The driver classifies the
  request (hooking yourself; observing code in a scope the master can check;
  everything else) and asks the master's `valid_hook()`, which must grant at
  least that tier. A master that naively returns 1 allows self-hooks only.
* **Few, generic join points.** Function entry by any route (`function`, with
  an `origin` filter for `call_other`, driver applies, `call_out`, simul_efuns,
  local calls), any efun (`efun:<name>`), misses (`call_other:miss`), object
  creation and destruction with its cause, and connection accept
  (`net:accept`). Everything the driver can do is reachable through these
  (§4.4 has the coverage map); bespoke points that only duplicate them are
  listed in Appendix B and added on demand.
* **Policy is LPC.** Who may hook what is the master's decision; reports,
  status-effect bookkeeping and builder-facing rule tables are mudlib code.

A compile-time complement, a DGD-style auto object, needs no driver change
and is part of this document (§4.7).

## 2. Motivation

### 2.1 Administrative scenarios

| # | Scenario | Served by |
|---|---|---|
| S1 | **Typo finder** (#1414): every `call_other` to a function that does not exist or is not callable, once per (caller program, target program, function) | `call_other:miss` + `count` |
| S2 | **Hot-function profile** on a live mud | `function` + `count` (dev-time: the tracer, §3.2) |
| S3 | **File-write audit**, including `efun::write_file` | already served by `valid_write` (§3.2) |
| S4 | **Capability fence**: `/domains/` may not `call_other` into `/secure/` | `function` DECIDE, `origin: "call_other"` |
| S5 | **Lifecycle accounting**: live objects per program, every destruct with its cause | `object:create`, `object:destruct` |
| S6 | **Deprecation**: who still calls `/std/old_combat::attack()`, attachable at runtime | `function` + `count` |
| S7 | **Default method / proxy** (LDMud `H_DEFAULT_METHOD`) | `call_other:miss` DECIDE (claim) |
| S8 | **Error telemetry** | already served by master `error_handler` (§3.2) |
| S9 | **Eval accounting** per player or domain | `function` + `time` |
| S10 | **`call_out` leak finder** | `efun:call_out` + `count` |
| S11 | **Per-domain object quotas** | `efun:clone_object` / `efun:new` DECIDE |
| S12 | **Security alarms**: `snoop`, `exec`, `seteuid`, `shutdown`, `save_object` | `efun:<name>` observe or DECIDE |
| S13 | **Connection bans and rate limits**: refuse a connection by address before the driver allocates a session (today the only refusal is the master's `connect()`, after accept, negotiation and allocation) | `net:accept` DECIDE |

Out of scope: hot-patching code (`recompile_object()`), per-opcode tracing
(the tracer and the debugger in #1286), and command/message auditing (already
served by the `process_input`, `receive_message` and `catch_tell` applies).

### 2.2 Gameplay scenarios

LPMud games need cross-cutting, temporary, per-instance behaviour and have
faked it with `shadow()` (Discworld: 33 gameplay shadow sites and 76
`this_object()->` self-calls in its combat code only to make local calls
visible to shadows) or with cooperative registries (Lima: 205 hand-placed
hook points). Appendix A has the evidence. What that evidence asks for:
interception of **local** calls, **per-instance** targeting that stacks
deterministically, removal handles and auto-cleanup, and per-object
introspection.

| # | Use case | Mechanism | Kind | Verdict |
|---|---|---|---|---|
| G1 | Curse of Butterfingers / Silence on one player | `function` `wield`/`cast`, `object` = victim | DECIDE (answer 0 and tell the player); rewriting speech needs AROUND | Hook |
| G2 | Invulnerable at a temple, subdued, "you are dead" | `function` `adjust_hp`/`do_death`/`attack_by`, `object` = player | DECIDE; clamping needs AROUND | Hook |
| G3 | Vulnerability / damage shield / thorns | `function` `receive_damage`, `object` = victim | AFTER (thorns); AROUND (scaling) | Hook over legacy combat |
| G4 | Vampiric enchant on *this* heirloom sword | `function` `hit`, `object` = sword | AFTER | Hook |
| G5 | Polymorph / disguise that `/secure/` still sees through | `function` `query_short`/`query_cap_name`, `object` = player, exclude `caller_program` `/secure/` | AROUND | Hook |
| G6 | Charm monster | `function` `heart_beat` and `attack_ob`, `object` = NPC | DECIDE | Hook |
| G7 | Escort quest / follow the leader | `efun:move_object`, `caller_object` = leader | AFTER | Hook |
| G8 | Blood Moon, seasonal overlays | `function` on `/std/undead::query_damage`, `/d/lake/` exits | AROUND | Hook (expires; permanent rules move to code) |
| G9 | Regional pricing | `function` `query_value`, `caller_program /std/shop/` | AROUND | Hook |
| G10 | Achievements and quests over legacy content | `function` `die` in `/std/monster`; `efun:move_object` | AFTER, deferred | Hook as an event source |
| G11 | Bounty, reputation, gossip, city guards | `function` on deed functions | AFTER, deferred | Hook as an event source |
| G12 | NPC/LLM event stream, economy simulation | as G11, plus `count` | AFTER, deferred | Hook as a source, batched only |
| G13 | A/B balance experiments | `function`, `caller_object` cohort | AROUND | Hook |
| G14 | Live invariants ("gold is conserved") | `function` on every money-mutating function | AFTER, deferred | Hook |
| G15 | Suppress an apply for one object | `function` `init`, `object` = target | DECIDE | Hook |

G1–G4, G6, G7, G10–G12, G14 and G15 need only observe and decide. G5, G8, G9
and G13, and the rewriting halves of G1–G3, need AROUND, which ships last and
behind a gate (§7, §9).

### 2.3 Cases validated in real mudlibs

Eleven cases were checked against the code they cite (full write-ups, with
file and line, in Appendix A).

| # | Case | Evidence in one line | Mechanism | Phase |
|---|---|---|---|---|
| V1 | Find calls to functions that do not exist | pkuxkx calls `set_busy()`, `unconsious()`, `is_fight()` and es2 calls `stop_busy()`; none is defined anywhere, so cooldowns and knock-outs silently never applied | `call_other:miss` | 1 |
| V2 | Audit every exp/money change | pkuxkx's polling audit is commented out; 613 direct `add("combat_exp", …)` calls bypass the reward daemon but all pass through one `add()` | `function` AFTER, deferred, `args` filter | 2 |
| V3 | Money conservation and dupe hunting | Discworld has an unchecked `set_money_array()` beside `adjust_money()` | `function` AFTER + DECIDE fence | 2 |
| V4 | Quest credit on NPC death | dtxyzjb has 281 copy-pasted `die()` overrides | `function` AFTER, deferred | 2 |
| V5 | Conditions that block verbs | pkuxkx checks "cannot equip" in `wield` but not `wear` | `function` DECIDE, `object`, `expires` | 2 |
| V6 | Safe zone that holds on every damage route | Dead Souls checks `"no attack"` in 9 places; spell damage skips it | `function` DECIDE | 2 |
| V7 | Follow and escort | Lima re-registers room hooks on every move | `efun:move_object` AFTER, `caller_object` | 3 |
| V8 | Deprecation telemetry | Discworld logs obsolete-API callers by hand at 4 sites | `function` + `count` | 2 |
| V9 | Parser-rule fence | Dead Souls' master greps the source of every object it loads | auto object (§4.7); `efun:parse_add_rule` DECIDE as the runtime form | 1 / 3 |
| V10 | Wizard forensics cheap enough to leave on | Dead Souls ships destruct logging switched off as too expensive | `efun:<name>` + `count`; `object:destruct` | 3 / 1 |
| V11 | Cleanup when a room's destruct cascades | pkuxkx's destruct bookkeeping never runs for objects destructed with their room | `object:destruct` (cause) | 1 |

### 2.4 Where hooks are the wrong tool

Hooks win when the callee is legacy or spread over many files, when the
effect depends on *who calls*, when it is per-instance and temporary, or when
the only chokepoint is an efun. They lose where the mudlib already owns a
single chokepoint, or the behaviour is permanent. Checked examples:

| Case | Why not a hook |
|---|---|
| Timed stat buffs (`add_temp("apply/…")`, pkuxkx: 373 files with a manual undo) | The consumers are the hottest getters in combat and the lib owns the chokepoint; fix with a buff daemon |
| PK / newbie / `no_fight` gates (pkuxkx `kill.lpc` and its copies) | `kill_ob()` is the chokepoint; move the rules there |
| Discworld PK pair rules (`pk_check`, 47 sites) | A rule about a pair of players asked before unrelated verbs; a simul_efun question is the right shape |
| Guard NPCs reacting to killers | `init()` fires per move × inventory; per-NPC data in an inherit |
| Genesis guild shadows | They *add functions* to the player. **Non-goal:** hooks advise functions that exist; they do not add new ones |
| Dead Souls damage protections | The lib already has an ordered modifier list |
| `destruct` policy (cleanup, `remove()`) | Permanent efun policy, already in the simul_efun; the auto object's job (§4.7) |
| Double XP, permanent stat stacking, permadeath | One chokepoint, or identity that belongs in the class |
| Invisibility through `id()` | A driver-questioned apply (AGENTS.md §13.24); needs a first-class concept |
| Builder triggers, player housing and shop policies | Owner *data* checked by the object that already receives the event |
| Phasing, accessibility rewriting, translation, verb blocking | Lib architecture, or `receive_message`/`catch_tell`/the parser already do it |

**Trust.** Builder- and player-facing features are exposed by a mudlib
daemon as **rule data** (zone, program, function, verdict, expiry), so
builders never need hook code of their own.

## 3. Today and prior art

### 3.1 What FluffOS can do today: probe results

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
| S9–S13 | `valid_*` covers some efuns per call; `call_out` and object creation have no per-call observation; a connection can only be refused by the master's `connect()`, after the session is allocated | No |

**Honest residual case.** What the driver alone can add:
(a) miss observation and default methods at **zero cost on hits** instead of
3.4x on every call; (b) lifecycle events with a **cause** and without a
mandatory base object; (c) call and efun observation/decision **attachable at
runtime** at a cost proportional to what is observed, not to all traffic;
(d) composition of several observers without each paying the wrapper cost.

### 3.3 Prior art

Sources: LDMud `github.com/ldmud/ldmud` `8afa5f50` (3.6.8); DGD
`github.com/dworkin/dgd` `733ea01`; kernellib `e602afe`; cloud-server
`7959357`; lpc-ext `0e299c8`.

#### LDMud

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

#### DGD

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
  (`kernellib/src/doc/kernel/hook/driver:19-84`). Mudlibs can build the same
  shape on top of hooks (the example effects daemon in §6 does).
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

### 3.4 What we take

| From | Take | Avoid |
|---|---|---|
| LDMud | `H_DEFAULT_METHOD`'s placement (after normal lookup and shadows fail) and its "handled or declined" protocol, here an answer of `({ value })`; master gating; sealed values; a per-call budget like `limited()`; a global bit test at each site, like `trace()` | One handler per hook; silent widening to driver applies; per-hook accidental error policy; an always-on per-opcode call |
| DGD / kernellib | Contained critical calls whose errors never cascade; touch's cheap per-object gate and its clear-before-call discipline; error observation at throw time (already `error_handler`); the auto object as the compile-time layer (§4.7); a small surface of events LPC cannot express itself | Unlimited budgets for code wizards write; permissive returns (nil/0 = deny, AGENTS.md §13.24) |

## 4. Design

### 4.1 Authorization: tiers and `valid_hook()`

```lpc
// master
int valid_hook(object who, string point, mixed action, mapping spec, int flags, int tier);
// returns the tier it grants: 0 (deny), 1, 2 or 3
```

Any object may call `hook_attach()`. The driver does three things before any
attachment exists:

1. **Normalizes the request.** Unknown or inapplicable spec keys, malformed
   patterns and kinds a point does not allow are errors. Names are
   canonicalized (`filename_to_obname()`; `#n` stripped for name keys;
   prefixes only with a trailing `/`), so `/secure/login`, `/secure/login.c`
   and `//secure/login` are one spelling. `who` is always the object that
   called the efun (a mudlib simul_efun wrapping `hook_attach()` therefore owns
   what it attaches).
2. **Computes the tier the request needs.** This is the part a master cannot
   get wrong by accident:

   | Tier | The driver has checked that | Typical use |
   |---|---|---|
   | **1 Self** | the spec pins `object` or `caller_object` to `who`, the handler belongs to `who`, and neither `HOOK_FAIL_CLOSED` nor `HOOK_OVERRIDE_NOMASK` is set. Any kind | an object advising calls made on it or by it: a proxy answering its own misses, an item reacting to its own use |
   | **2 Scoped observe** | the kind is BEFORE or AFTER (LPC handler, `count` or `time`), the handler belongs to `who`, the point is `function`, `efun:*`, `call_other:miss`, `object:create` or `object:destruct`, and at least one scoping key is present (`target`, `defined_in`, `object`, `caller`, `caller_program`, `caller_object`) | a wizard profiling or auditing code under their own directory; the master checks the paths |
   | **3 Privileged** | nothing: everything else. `HOOK_DECIDE`/`HOOK_AROUND` on anything but yourself, `HOOK_FAIL_CLOSED`, `HOOK_OVERRIDE_NOMASK`, no scoping key, a handler owned by another object, `net:*`, the simul_efun program as a target, and delivery of events raised inside simul_efun code | the mudlib's own daemons: audit, quests, effects, security |

3. **Asks the master**, passing the tier. The attachment is created only if
   `valid_hook()` returns an integer **≥ that tier**. An absent apply, 0, a
   lower number, any other value, a promise (AGENTS.md §13.24) or an error
   denies, and so does attaching before the master is loaded. A master that
   simply returns 1 therefore allows self-hooks only; passwords and other
   people's arguments are out of reach until the master deliberately returns
   2 or 3. The master receives a fresh copy of the normalized spec; changing
   it has no effect.

This mirrors how existing gates work: `shadow()` applies hard driver rules
before asking `valid_shadow()`; `bind()` hands `valid_bind()` the binder and
both owners; sockets ask `valid_socket()` per operation.

**Hard rules, whatever the master returns**

* The master object is never a target: no attachment matches a function in
  the master, and no call-type event fires for calls the master makes.
  Lifecycle events (`object:create`, `object:destruct`) fire regardless of
  who caused them.
* Nothing fires inside `valid_hook()`, any other `valid_*` apply,
  `error_handler`, or while a compile is active (a depth counter around the
  generated set of those master applies, plus the compiler's `current_file`).
* **Simul_efun code is transparent, not exempt, and privileged to observe.**
  Real mudlibs issue `destruct`, `call_out`, `move_object`, `exec` and `snoop`
  from simul_efun wrappers (Dead Souls, Lima, Nightmare, Discworld; Appendix
  A), so exempting it would blind `efun:*` hooks. An efun called from
  simul_efun code is attributed to the object that called the simul_efun.
  But events raised inside simul_efun code are delivered to tier-3
  attachments only: otherwise a wizard could hook `efun:users` on their own
  call to a simul `users()` and read what the wrapper filters out, undoing
  `valid_override()`. Functions of the simul_efun program can be hooked at
  tier 3, never with `HOOK_AROUND`.
* **Secrets are redacted at every tier.** The line delivered to a no-echo
  `input_to()`/`get_char()` callback, and argument 0 of `crypt()`/`oldcrypt()`,
  reach handlers as 0.
* `HOOK_FAIL_CLOSED` is refused on `net:accept`, `net:ws_upgrade` and
  `call_other:miss`: a broken handler must not be able to lock everyone out.
* A runtime config switch (`hooks enabled : 0`) makes `hook_attach()` always
  fail: the break-glass for a mud locked up by its own hooks, since owners
  re-attach in `create()` and a reboot alone would not help.

**Ownership, revocation, visibility**

* The attachment belongs to `who`; the handler runs as its owner (at tiers 1
  and 2, that is `who`).
* It is detached when its owner or its handler's owner is destructed, when
  the handler's function pointer goes stale (its owner was recompiled), when
  an `object`-filtered target is destructed, on expiry, after
  `hook max errors` errors (never for `HOOK_FAIL_CLOSED`), or by the master.
  Attachments survive recompilation of the programs they *target*. Nothing
  persists across a reboot: owners attach in `create()`, the same idiom as
  `call_out()` and `set_heart_beat()`.
* Every detach is reported to the owner (`hook_detached(id, reason)`, if it is
  alive) and to the master (`hook_detached(id, reason, owner)`), so a fence
  never disappears silently.
* When the master is recompiled the driver re-asks `valid_hook()` for every
  live attachment and detaches those no longer granted.
* `hook_query()`/`hook_detach()` work for the owner and the master.
  `hook_list(ob)` shows other callers only the labels of attachments that
  apply to `ob`.

**Upgrade hazard.** A master that already defines a function named
`valid_hook` would answer the new apply. Release notes and the apply's page
must say so.

### 4.2 The attachment model

```lpc
int hook_attach(string point, mixed action, mapping spec, int flags);
```

**Kinds** (`flags`, exactly one):

| Kind | Runs | Can change the outcome? |
|---|---|---|
| `HOOK_BEFORE` (0) | before the call, for every attempt, including ones a decider then refuses | No |
| `HOOK_DECIDE` | after the BEFORE handlers | Refuse or answer (below) |
| `HOOK_AFTER` | after the call returned; gets its result | No |
| `HOOK_AROUND` (phase 4, gated; `function` only) | wraps the call; gets a `proceed` function | Yes: change arguments or result, or skip the call |

Event points have one kind each (`object:create` is after, `object:destruct`
is before); for them `flags` kind 0 means "the point's kind".

Order at one join point is total: **BEFORE → DECIDE → (AROUND) → the call →
AFTER**, and within a kind by `priority` (lower first, ties by attach id).
The list is fixed when dispatch starts: attaching or detaching inside a
handler affects the next event, and an attachment detached mid-dispatch
(for example because its owner was destructed) does not run.

**Decisions.** A `HOOK_DECIDE` handler returns:

| Return | Meaning |
|---|---|
| 0 or nothing | no opinion; the next decider is asked |
| a string | **refuse**: the call does not run and the caller gets an error with that text |
| 1 | refuse with a generic message naming the attachment's label |
| `({ value })` | **answer**: the call does not run and `value` is its result (checked against the function's declared return type). This is how a curse makes `wield()` return 0 after telling the player why, and how a proxy claims a miss |

The first refusal or answer wins. What each means at a point is in the
catalogue's "decision" column (§4.4): an efun can be refused but not
answered; `net:accept` refuses by closing the connection; a call the driver
itself makes (origin `driver`) is skipped without raising anything.

Anything else a decider does — returns another type or a promise (AGENTS.md
§13.24), errors, runs out of budget, or cannot be run at all (§4.3) — is "no
opinion" and is counted. With `HOOK_FAIL_CLOSED` it is a refusal instead, so
a fence cannot fail open.

**Actions.** A function pointer (the handler), `"count"`, or `"time"` (count
plus summed and maximum eval microseconds; needs the call's exit, so only
where AFTER is allowed). `count` and `time` run entirely in C and keep rows
keyed by (caller program, target program, function); at event points the
"function" column holds the cause or reason.

**Handlers** take positional arguments (a context mapping costs about 200 ns
per event to build):

```lpc
mixed handler(int id, object actor, object caller, object target, string fn, mixed *args);
// HOOK_AFTER appends `mixed result`; HOOK_AROUND appends `function proceed`;
// call_other:miss appends `string reason`. Event points pass their own arguments (§4.4).
```

* `actor` is `this_player()` at the intercepted call. Inside a handler
  `this_player()`, `this_interactive()` and `previous_object()` are 0: a
  handler learns identities only from its arguments, and cannot `input_to()` or
  `command()` as the victim.
* `caller` may be 0 (the backend calls `reset()`, `heart_beat()` and
  `call_out`s with no current object).
* `args` holds the call's arguments; arrays, mappings, classes and buffers
  are one-level copies, so an observer cannot change what the callee sees.
  Only `HOOK_AROUND` can, through `proceed`.

**Batched delivery** (`HOOK_DEFERRED`, with BEFORE or AFTER). Events are
queued per attachment and delivered once per gametick as
`handler(int id, mixed *events)`, each event being the argument list after
`id` (for AFTER: `({ actor, caller, target, fn, args, result })`). Objects
destructed in the meantime are 0; overflow beyond `hook max deferred` is
counted in `dropped`; the queue is discarded on detach. One LPC frame per
batch is what makes event-stream consumers (quests, ledgers, NPC and LLM
feeds) affordable, and it is the only way to feed an `async` consumer.

**Filter keys** (in `spec`; a string or an array of strings unless noted;
prefix match only when a pattern ends in `/`; no globs). Which keys a point
accepts is in the catalogue.

| Key | Matches |
|---|---|
| `target` | the target object's name, clone suffix stripped |
| `object` | one target object instance (for `efun:*`: the first object-typed argument) |
| `defined_in` | the program that defines the called function |
| `function` | function name |
| `origin` | how the function was entered: `"local"`, `"call_other"`, `"driver"` (applies, `heart_beat`), `"internal"` (`call_out`), `"simul"`, `"efun"` (verb functions and efun callbacks), `"function pointer"` — the names the `origin()` efun already uses |
| `caller` | the calling object's name, clone suffix stripped |
| `caller_program` | the program of the calling frame |
| `caller_object` | one calling object instance |
| `args` | a mapping from argument position to a value or array of values (strings, ints, objects), compared in C: `([ 0: ({ "combat_exp", "balance" }) ])` |
| `exclude` | a mapping with the same keys; an event matching it is skipped |

`spec` also carries `"label"` (required; shown in listings, errors and
traces), `"priority"` (int, default 0) and `"expires"` (seconds; refused with
`HOOK_FAIL_CLOSED`, so a fence cannot lapse silently).

**Re-entry.** While an attachment's handler runs, further matching events
are treated per cause: events caused by the handler's own owner are ignored
(the handler's own work must not recurse); events caused by anyone else are
queued for an observer and delivered when the handler returns, and are "no
opinion" for a decider (a refusal under `HOOK_FAIL_CLOSED`). Other
attachments always see everything, so doing a write from inside your own
handler does not hide it from an audit hook.

**Rules for `function` hooks**

* `nomask` is respected: DECIDE and AROUND attachments skip `nomask`
  functions unless `HOOK_OVERRIDE_NOMASK` is set (tier 3). Libraries such as
  Genesis make their combat pipeline `nomask` precisely to stop shadows.
* Calls the driver makes (origin `driver`) can be decided or advised only
  when the driver ignores the result: `create`, `init`, `heart_beat`,
  `on_destruct`, `receive_message`, `net_dead` and the telnet applies. For
  the rest a refusal would either be read as an answer by C code (`id`,
  `catch_tell`, verb functions) or take the "function not found" path that
  permanently disables the apply (`reset`, `clean_up`, `process_input`,
  `write_prompt`). Observing is always allowed. `__INIT` is not hookable.
* `(: … :)` functionals have no named function and are not hookable.
* For an `async` function, AFTER runs when the body first parks and its
  result is the promise.

### 4.3 Contained invocation

Every handler call goes through one primitive. Written against the current
source (`safe_call_function_pointer()` cannot be used as is: it expects the
arguments already pushed, and pushing can itself throw into the middle of,
say, `destruct_object()`):

```cpp
HookRun hook_call(Attachment* a, PushArgsFn push_args, svalue_t* out) {
  funptr_t* f = a->fp;
  if (a->detached) return kSkipped;
  if (!f->hdr.owner || (f->hdr.owner->flags & O_DESTRUCTED)) return detach(a, "handler_owner_destructed");
  if (stale_layout(f)) return detach(a, "stale_function");            // pre-check: the error text cannot be recognised afterwards
  if (current_file || too_deep_error || max_eval_error ||
      csp >= &control_stack[CFG_MAX_CALL_DEPTH - kHookHeadroom]) return kSkipped;

  int const num_arg = st_num_arg;                                       // AGENTS §13.16
  auto const t0 = steady_clock::now(); int64_t const ev = get_eval();
  int const oot = outoftime;
  object_t *ci = current_interactive, *hb = g_current_heartbeat_obj;    // raw pointers: hold refs
  if (ci) add_ref(ci, "hook"); if (hb) add_ref(hb, "hook");
  auto const sim = hook_save_simulate_state();                          // restrict_destruct, num_objects_this_thread
  f->hdr.ref++; a->running = true; g_hook_depth++;
  save_command_giver(nullptr);                                          // this_player() == 0
  current_interactive = nullptr; g_current_heartbeat_obj = nullptr;
  DEFER {
    restore_command_giver();
    current_interactive = (ci && !(ci->flags & O_DESTRUCTED)) ? ci : nullptr;
    g_current_heartbeat_obj = (hb && !(hb->flags & O_DESTRUCTED)) ? hb : nullptr;
    if (ci) free_object(&ci, "hook"); if (hb) free_object(&hb, "hook");
    hook_restore_simulate_state(sim);
    st_num_arg = num_arg;
    set_eval(ev - elapsed_us(t0)); outoftime = oot;                     // the caller pays for the handler
    max_eval_error = too_deep_error = 0;
    a->running = false; g_hook_depth--; free_funp(f);
  };
  set_eval(std::min<int64_t>(hook_eval_cost(), ev));
  error_context_t econ; save_context(&econ);
  svalue_t* ret = nullptr;
  try { int const n = push_args(); ret = call_function_pointer(f, n); }
  catch (const char*) { restore_context(&econ); }
  pop_context(&econ);
  if (!ret) { a->errors++; return kError; }
  if (ret->type == T_PROMISE || a->detached) return kDeclined;
  assign_svalue_no_free(out, ret);                                      // copy before the next handler overwrites it
  return kRan;
}
```

| # | Rule | Why |
|---|---|---|
| 1 | Budget is the smaller of `hook eval cost` and the caller's remaining eval; elapsed time is charged to the caller. An overrun aborts the handler only | Restoring the caller's full budget would make hooked calls free: `while (1)` on a self-hooked no-op never ends. Not charging at all bills the time to nobody (AGENTS.md §13.23) |
| 2 | `outoftime`, `max_eval_error`, `too_deep_error` are reset on the way out | Otherwise a handler that ran out of eval makes the caller's next opcode die with "Too long evaluation" |
| 3 | `restrict_destruct` and `num_objects_this_thread` are saved and restored | The driver's error handler zeroes both on *every* error, contained or not; a handler error during a destruct cascade or an inherit-chain load would silently remove the guard that protects it |
| 4 | `current_interactive` and `g_current_heartbeat_obj` are nulled for the call (with refs) and restored only if still alive; `command_giver` is saved and restored | A handler's too-deep error would otherwise switch off the heart_beat of the innocent outer object |
| 5 | `st_num_arg` is latched and restored; arguments are pushed inside the `try` | §13.16; a stack overflow while pushing must not throw into the join point's caller |
| 6 | A handler that cannot run (compile active, driver already in an error state, fewer than `kHookHeadroom` = 16 control frames left) returns "skipped": counted, and a refusal under `HOOK_FAIL_CLOSED` | A silent skip would let anyone bypass a fence by recursing to the depth limit |
| 7 | Stale or ownerless handlers are detached by a pre-check, not by recognising an error | `safe_*` paths report only "it failed" |
| 8 | After any BEFORE or DECIDE handler, the join point re-checks that its target is not destructed, and efun points re-run the efun's argument type check | `apply_low()` checks for a destructed target only before; `call_direct()` never does; a handler that destructs an efun's object argument must produce a clean "bad argument" |
| 9 | Handler errors are counted and reported through the normal uncaught-error path, with advised frames labelled (`[hook #12 blood_moon]`) | Debuggability; no per-attachment "last error" text is kept, because the unwind does not carry it |
| 10 | The attachment table, handler function pointers, owners and object-valued filters are marked in `checkmemory.cc` (`mark_funp`, after `mark_call_outs()`); count tables are C++ containers bounded by `hook max rows` | AGENTS.md §3; attacker-chosen names must not grow memory |
| 11 | No detach callbacks at shutdown; the table has no static-destructor side effects | The Windows exit-abort class of bug (AGENTS.md §14) |

Eval limits are enforced on Linux only (`src/vm/internal/eval_limit.cc`).
Each handler call costs three timer system calls on top of the LPC call;
phase 1 measures this and the cost gate (§4.5) records it.

### 4.4 Join-point catalogue (normative; the only one)

Sites are under `src/`. Tier is the lowest that can attach (§4.1); DECIDE and
AROUND are tier 3 unless the attachment is a self-hook.

| Point | Fires | Site | Kinds | A decision means | Filter keys | Handler arguments after `id` | Frequency | Phase |
|---|---|---|---|---|---|---|---|---|
| `call_other:miss` | a `call_other`/`->` (single or array target) found nothing callable, after the shadow chain | `vm/internal/apply.cc` `apply_low()`, origin `call_other`, at the not-found and no-permission exits, where the arguments are still on the stack | BEFORE, DECIDE | answer only: `({ value })` becomes the result; otherwise the caller gets `undefined` as today | `target`, `object`, `function`, `caller`, `caller_program`, `caller_object`, `exclude` | actor, caller, target (the object the caller named, before the shadow walk), fn, args, reason (`"undefined"`, `"private"`, `"protected"`, `"destructed"`; `static` reads as protected) | rare | 1 |
| `object:create` | an object finished `create()` (also when it defines none); not if `create()` destructed it | `vm/internal/base/object.cc` `call_create()` | after | — | `target`, `exclude` | ob | medium | 1 |
| `object:destruct` | any destruct, driver-initiated included | `vm/internal/simulate.cc` `destruct_object()`, after its `O_DESTRUCTED` re-check and before `remove_object_from_stack()`; the handler's own destruct of the object is detected afterwards | before | — | `target`, `object`, `exclude` | ob, cause (`"efun"`, `"shadowed"`, `"environment"`, `"refused"`, `"reload"`) | medium | 1 |
| `function` | a named function is entered, by any route | one helper called after default arguments are filled and before the frame is pushed, at the five entry sites: `apply_low()`, `F_CALL_FUNCTION_BY_ADDRESS`, `F_CALL_INHERITED`, `call_direct()`, `FP_LOCAL` function pointers | BEFORE, DECIDE, AFTER; AROUND in phase 4 | refuse: error in the caller (driver-origin calls are skipped silently); answer: the value is returned | all of §4.2; at least one of `defined_in`, `target`, `object`, `function` is required | actor, caller, target, fn, args | per call | 2 |
| `efun:<name>` | an efun is about to run; `efun::foo()` and `foo()` are the same event | `vm/internal/base/interpret.cc` `call_the_efun` (all efun opcodes) and `vm/internal/base/function.cc` efun function pointers | BEFORE, DECIDE, AFTER | refuse: error in the caller. No answer | `object` and `target` (first object-typed argument), `caller`, `caller_program`, `caller_object`, `args`, `exclude` | actor, caller (the simul_efun's caller when issued from simul_efun code), target, name, args | per efun call | 3 |
| `net:accept` | a connection was accepted on a driver port, before any session or object exists | a shared helper in `comm.cc`, called by `net/transport_libevent.cc` `new_conn_handler()` and `wasm/comm_wasm.cc` before anything is allocated | BEFORE, DECIDE | refuse: the descriptor is closed | none (tier 3 only) | port, kind (`"telnet"`, `"ascii"`, `"binary"`, `"mud"`, `"websocket"`), tls flag, peer address | per connection | 3 |
| `net:ws_upgrade` | a websocket upgrade request, before the `101` | a new `LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION` arm → one helper in `net/ws_common.cc` | BEFORE, DECIDE | refuse: the upgrade is rejected | none (tier 3 only) | port, subprotocol, peer address (honouring `X-Real-IP`), Origin, Host, User-Agent | per connection | 3 (own PR, with the browser matrix of AGENTS.md §14) |

Point-specific rules:

* **`efun:<name>`** fires at dispatch, before the efun's own `valid_*`
  checks, so observers also see attempts the efun then refuses; a decision
  can only refuse, so nothing a `valid_*` denies is ever granted. Not
  hookable: the `hook_*` efuns and `call_other` (use `function` with
  `origin: "call_other"`). DECIDE is refused on `error`, `throw`,
  `set_eval_limit`, `reset_eval_cost` and `eval_cost`. The shared type-check
  helper also closes an existing gap (efun function pointers check fewer
  arguments than the opcode route does today).
* **`function`** keys on the *defining* program. `defined_in` names it
  directly; with only `target` or `object`, the driver resolves the programs
  that define `function` for the matching objects (their own program and what
  they inherit) and re-resolves when a program is loaded. No double firing:
  one entry is one event, whatever the route.
* **`net:*`** handlers never see a session: they run before any
  `interactive_t` exists, so `users()` inside a handler cannot return a
  half-built one. For proxied websocket clients `net:accept` sees the proxy's
  address; the real client is known at `net:ws_upgrade`. Address-keyed
  `count` rows obey `hook max rows`. Hook calls live in shared code
  (`comm.cc`, `net/ws_common.cc`), so the WASM transport is covered without
  `#ifdef` (AGENTS.md §5); inside libwebsockets, handlers run only in the
  filter callback, never in `SERVER_WRITEABLE` (AGENTS.md §14).

**Coverage map.** Everything the driver does is reachable through the points
above:

| To hook | Use |
|---|---|
| an efun call | `efun:<name>` |
| a simul_efun call | `function`, `defined_in` = the simul_efun object, `origin: "simul"` (tier 3) |
| an apply (the driver calling an object): `create`, `init`, `reset`, `clean_up`, `logon`, `net_dead`, `process_input`, `catch_tell`, `receive_message`, telnet applies | `function`, `origin: "driver"` (observe always; decide per §4.2) |
| `call_other` / `->` | `function`, `origin: "call_other"`; misses: `call_other:miss` |
| `call_out` | scheduling: `efun:call_out`; firing: `function`, `origin: "internal"` (named functions); removal: `efun:remove_call_out` |
| `heart_beat` | `function` `heart_beat`, `origin: "driver"`; on/off: `efun:set_heart_beat` |
| destruct | every destruct with its cause: `object:destruct`; refusing an explicit one: `efun:destruct` DECIDE |
| move | `efun:move_object` (the driver's only move path): `caller_object` = the moved object, `object` = the destination; BEFORE sees the old environment, AFTER the new one |
| clone, load, create | `efun:clone_object`, `efun:new`, `efun:load_object`; `object:create` |
| `exec`, `snoop`, `shadow`, `recompile_object`, `replace_program`, sockets | `efun:<name>` (callbacks: `function`) |
| commands and input | `function` on verb functions and on `process_input`; `efun:command` |
| network | connections: `net:accept`, `net:ws_upgrade`; GMCP/MSDP/telnet subnegotiation in: `function` on `gmcp`, `msdp`, `telnet_suboption`, `window_size`, `terminal_type` (origin `driver`); out: `efun:send_gmcp` and friends |

Bespoke points that were surveyed and would only duplicate these, or that
have no use case yet, are in Appendix B with their sites and hazards, so they
can be added when a mudlib asks.

### 4.5 Gating and the cost gate

**When nothing is attached** a join point costs one predicted branch on its
bit in a global `g_hook_mask`. A build with that test at five sites,
including before *every* efun dispatch, was indistinguishable from master
(call_other 113/113 ns, local call 43/43, `sizeof` 19/19; launch-to-launch
noise was ±6% in that A/B run).

**When something is attached**, cost is confined to what is hooked:

| Point | Second-level gate |
|---|---|
| `function` | a `PROG_HOOKED` bit in the defining program's flags, then a lazily allocated per-function byte map on that program (there is no free bit in `function_flags`); an unhooked function in a hooked program pays one more branch |
| object filters (`object`, `caller_object`) | an `O_HOOKED` bit in `object_t::flags` (bit `0x20` is free) |
| `efun:<name>` | a byte per efun (`g_efun_hooked[]`) |
| matching | an open-addressed set keyed on interned `(program, function-name)` pointers, estimated 5–10 ns; never a string-keyed lookup |

Bits are recomputed when a program is compiled, on `recompile_object()` and on
`replace_program()` (which swaps `ob->prog` without a generation bump).
`count`/`time` rows are keyed on ref-held shared-string pointers from the
start, so the phase-1 table does not need rewriting for phase-2 traffic.

**Acceptance gate** (each phase):

| Gate | Method | Threshold |
|---|---|---|
| Detached | `valgrind --tool=cachegrind` instruction counts on a hooks microbenchmark (`testsuite/command/speed_hooks.lpc`: local call, call_other hit and miss, `sizeof`, new+destruct), A/B | more than 0.5% instructions per iteration on any detached case fails |
| Detached, wall clock (local, numbers in the PR) | interleaved A/B, ≥ 10 launches each, best-of-5 inner, medians | max(3%, 2× baseline inter-launch MAD) |
| Attached | same harness: `count` on misses; 1 and 100 non-matching filters; an unhooked function in a hooked program; a hooked function on an unflagged object; one matching handler; a deferred observer; 100 cursed players among 300; one charmed NPC among 2,000 heart_beats | recorded; the handler-call cost (including its three timer system calls) is measured in phase 1 before any budget is promised |
| Memory | Debug build with attachments live: the suite passes with no `check_memory()` report | must pass |

### 4.6 API reference

**Master applies**

```lpc
int  valid_hook(object who, string point, mixed action, mapping spec, int flags, int tier);  // §4.1
void hook_detached(int id, string reason, object owner);                                     // optional
```

**Optional apply on the owner:** `void hook_detached(int id, string reason)`.
Reasons: `"expired"`, `"owner_destructed"` (master only),
`"handler_owner_destructed"`, `"target_destructed"`, `"stale_function"`,
`"errors"`, `"master"`.

**Efuns**

| Efun | Who | Returns |
|---|---|---|
| `int hook_attach(string point, mixed action, mapping spec, int flags)` | anyone; §4.1 | the attachment id (monotonic, never reused); errors when refused or malformed |
| `void hook_detach(int id)` | owner or master | — |
| `mapping hook_query(int id)` | owner or master | `([ "id", "point", "owner", "label", "flags", "priority", "expires", "spec", "events", "errors", "skipped", "dropped", "eval_us", "last_fired", "rows" ])`; `rows` (count/time only) is an array of `([ "caller_program", "target_program", "function", "count", "eval_us" ])` |
| `mapping *hook_list(object\|void ob)` | anyone | the caller's own attachments (the master: all), as `hook_query()` mappings without `rows`; with `ob`, the attachments that apply to `ob` — other callers get only `([ "label" ])` for those they do not own |

**Flags:** one kind (`HOOK_BEFORE` = 0, `HOOK_DECIDE`, `HOOK_AFTER`,
`HOOK_AROUND`), plus `HOOK_FAIL_CLOSED` (with DECIDE), `HOOK_DEFERRED` (with
BEFORE/AFTER), `HOOK_OVERRIDE_NOMASK` (with DECIDE/AROUND). Constants in
`include/hooks.h`.

**Runtime config**

| Key | Default | Meaning |
|---|---|---|
| `hooks enabled` | 1 | 0 makes `hook_attach()` always fail (break-glass) |
| `hook eval cost` | one tenth of `maximum evaluation cost` | budget of one handler call |
| `hook max errors` | 10 | errors, overruns included, before an attachment is detached (never for `HOOK_FAIL_CLOSED`) |
| `hook max attachments` | 10000 | total live attachments |
| `hook max rows` | 10000 | rows per `count`/`time` table; overflow counted in `dropped` |
| `hook max deferred` | 1024 | queued events per attachment |

### 4.7 Compile-time layer: the auto object

DGD gives every program an implicit parent, the auto object, which may
redefine kfuns and reach the originals with `::kfun()`; kernellib builds its
whole security and resource model there (§3.3). FluffOS can do the same today:

* **An inherited `protected` function overrides the efun of the same name**
  in every inheriting program, `efun::name()` still reaches the real efun, and
  the override is not callable from outside (probe on `b5714e5f`: an
  inherited `protected int sizeof()` returned 4242 to the child,
  `efun::sizeof` returned 3, `call_other` to it returned 0; an inherited
  `protected void destruct()` saw the child's `destruct()` call and the
  object was destructed).
* **An `inherit` arriving through `#include` works** (probe: an included
  header containing `inherit "/x/auto";` made the includer inherit it).
* **The global include file is prepended to every compile**, and the master's
  `include_file(compiled, from, path)` apply may return per-file source text
  (an array of strings). So the global include file names a header, and
  `include_file()` returns `inherit "/secure/auto";` for ordinary files and an
  empty text for the master, the simul_efun object, the auto object and its
  own inherits. This is cloud-server's layered-auto trick
  (`objectd.c:627-640`) with FluffOS's existing applies. The end-to-end
  combination is **not yet verified**; phase 1 verifies it and falls back to
  a one-line master apply if needed (§9).

What it is for, and what it is not:

| Use | Auto object | Hooks |
|---|---|---|
| Permanent efun policy (`destruct` logging and guarantees, `write`→`message`, file-efun checks) | **Yes**: zero runtime cost beyond the override's frame; `previous_object()` in callees unchanged (it is a local call) | No |
| Veto explicit `destruct()` (LDMud `prepare_destruct`) | **Yes**: the override raises an error | No |
| Close `efun::` bypass | With `valid_override()`: refuse `efun::name` outside the auto object | `efun:*` probes see `efun::` calls anyway |
| Change at runtime, attach and detach | No: every change means recompiling every program | **Yes** |
| Misses, driver-initiated destructs, `call_other` traffic | No | **Yes** |
| Advice on mudlib functions (curses, world events) | No: the auto object only redefines efuns and adds functions | **Yes** (`function` join point) |

Rules for the auto object: overrides and helpers are `protected` (callable
inside each program, not from outside) and `nomask` where children must not
replace them; private state lives in the auto object's own variables, which
each inheriting object gets a copy of, so keep it small; the auto object must
not inherit from mudlib code that itself gets the auto object injected.

## 5. Interaction with existing features

* **Shadows.** `function` fires on the function actually entered, after
  shadow resolution; `call_other:miss` means the shadow chain had nothing
  either, and reports the object the caller named.
* **`recompile_object()` and `replace_program()`.** Attachments are keyed by
  names; gate bits are recomputed on both. An attachment survives
  recompilation of what it targets, not of its handler's owner (§4.1).
* **Simul_efuns and `valid_override()`.** Unchanged. `efun:*` fires inside
  the real efun, so `efun::` calls are observed; events from simul_efun code
  reach tier 3 only (§4.1).
* **Async.** Handlers are synchronous; a handler that returns a promise is
  "no opinion". Batched delivery is the way to feed an `async` consumer.
* **Errors.** There is no error join point: master `error_handler` already
  runs at throw time with the trace live (§3.2).
* **The debugger (#1286) and the tracer** use their own bits in the same mask
  word.

## 6. Reference code (shipped with the docs and the testsuite)

* **A reference `valid_hook()`**: returns 3 for admin objects (by directory
  or euid); 2 for wizards when every scoping key in the spec lies under their
  own directory; 1 otherwise; caps live attachments per owner with
  `hook_list()`.
* **Examples**, each also a testsuite scenario: V1 typo finder
  (`call_other:miss` + `count`, filtering `query_*`/`is_*` when printing);
  S5 lifecycle counts with cause; S4 `/secure/` fence
  (`HOOK_DECIDE | HOOK_FAIL_CLOSED`); V2 exp/money audit; V5 verb-blocking
  condition; V7 follow.
* **An example effects daemon** (optional mudlib code, not a driver concept):
  status effects as attachments with labels and expiry, a player-visible
  effects list, re-attach at login and `restore_object()`, and a rule-data API
  for builders (§2.4).

## 7. Implementation plan

### 7.1 Phases

Ordered by validated value per unit of driver change. Each phase ships its
tests, its docs and the Appendix A cases it serves as testsuite scenarios.

| Phase | Delivers | Scope | Main driver changes | Serves |
|---|---|---|---|---|
| **1. Core** | #1414, lifecycle events, the compile-time layer | `valid_hook()` with tiers; request normalization; `hook_attach`/`detach`/`query`/`list`; ownership, detach and master re-validation rules; the contained-invocation primitive (§4.3); kinds BEFORE/AFTER/DECIDE with `HOOK_FAIL_CLOSED`; `count`; `HOOK_DEFERRED`; priority ordering; `call_other:miss`, `object:create`, `object:destruct`; config keys; `mark_hooks()`; reference `valid_hook()`; docs: interposition guide and the auto-object recipe (§4.7) | new `vm/internal/hooks.{h,cc}`, `packages/core/hooks.spec`; `apply.cc`, `simulate.cc` (`destruct_object()` gains a cause), `object.cc`, `master.cc`, `applies`, `rc.cc`, `checkmemory.cc` | V1, V9 (auto object), V10 (destruct half), V11; S1, S5, S7 |
| **2. Function entry** | observe and decide any function, per program or per object | `function` at its five entry sites; `PROG_HOOKED` + per-function map; `O_HOOKED`; all filter keys including `origin` and `args`; `time`; `expires`; `hook_list(ob)`; the driver-origin and `nomask` rules; secret redaction for no-echo callbacks | `interpret.cc`, `function.cc`, `apply.cc`, `program.h`, `object.h`, `comm.cc` (no-echo latch) | V2–V6, V8; S2, S4, S6, S9; G1–G4, G6, G10–G12, G14, G15 (observe/decide) |
| **3. Efuns and connections** | observe and refuse any efun; refuse connections | `efun:<name>` (dispatch sites, per-efun byte, shared type-check helper, simul_efun attribution, tier-3 delivery from simul frames, `crypt` redaction); `net:accept`; `net:ws_upgrade` as its own PR | `interpret.cc`, `function.cc`, efun table generation; `comm.cc`, `net/transport_libevent.cc`, `wasm/comm_wasm.cc`, `net/ws_common.cc` | V7, V9 (runtime form), V10; S10–S13; G7 |
| **4. Around advice** | rewriting arguments and results | `HOOK_AROUND` with `proceed` (valid once, only inside its handler call), the around chain, return-type check, `HOOK_OVERRIDE_NOMASK`; the example effects daemon | `apply.cc` (dispatch tail split), `interpret.cc`, `hooks.cc` | G5, G8, G9, G13 and the rewriting halves of G1–G3 — none validated in a real mudlib yet, so this phase is gated (§9) |

Phases 2 and 3 are independent of each other; phase 4 needs phase 2.

Suggested PR split: **1a** auto-object probe and the interposition guide;
**1b** the core with `call_other:miss` (closes #1414); **1c** the object
events and destruct-cause plumbing; **2a** `function` observe with gating;
**2b** decide, object filters, `expires`, redaction; **3a** `efun:<name>`;
**3b** `net:accept`; **3c** `net:ws_upgrade`. Phase 1 is roughly 3,500–4,000
lines including tests and docs; phase 2 and 3 about 3,000 each.

### 7.2 Tests

Each guard has a test that fails when the guard is removed (for new efuns
"fails on the unfixed driver" would be vacuous).

| What | Harness |
|---|---|
| Authorization matrix: tier computed correctly for each request shape; `valid_hook` returning 0/1/2/3, a promise, an error | LPC suite; the policy switch lives in `testsuite/inherit/master/valid.lpc` |
| No `valid_hook` in the master; attach before the master is loaded | GTest (null the cached apply); a probe in the testsuite simul_efun's `create()` |
| Miss by each route (`->`, explicit, array target) and reason; answer and no-opinion; promise and error are no-opinion | LPC suite (`"destructed"` via a GTest gametick bump) |
| Every destruct cause exactly once, including nested destructs and a handler that destructs the object | LPC suite, with a `valid_object` special case for `"refused"` |
| Owner or handler-owner destruct detaches; stale handler detaches; master recompile re-validates | LPC suite |
| Eval charged to the caller; `command_giver`, `current_interactive`, the heart_beat object, `restrict_destruct` restored after a forced overrun or error | GTest, Linux only (a small `hook eval cost`) |
| Skipped handler near the depth limit refuses under `HOOK_FAIL_CLOSED` | LPC suite |
| Refused while compiling | LPC suite, through the master's compile hooks |
| `check_memory()` clean with attachments live | LPC suite on Debug (call the efun in the test, then detach unconditionally) |
| `function`: every entry route observed once; functionals refused; driver-origin decide only on the allow-list; `nomask` skipped; target destructed by a handler; no-echo line redacted | LPC suite; `heart_beat` and `call_out` routes through GTest (`call_heart_beat()`, tick bump) |
| `efun:*`: `efun::` observed; simul-wrapper attribution; simul-frame events reach tier 3 only; handler destructing an argument gives "bad argument"; `crypt` redacted | LPC suite |
| `net:accept` refuses before any allocation, on the native and wasm paths; upgrade refused by real address | `tools/e2e-live.js` over real connections; browser matrix for the upgrade |

Also on every phase: Debug + sanitizer and RelWithDebInfo builds, the LPC
suite twice (randomized order), the §4.5 gate with numbers in the PR, and
docs for anything user-visible (`gen_config_docs.py`, `gen_sidebar.py`).

### 7.3 Rollout and risks

Hooks are off until a master defines `valid_hook()`, so existing mudlibs see
no behaviour change; the only cost is the detached branches. The testsuite
master ships the reference policy so CI exercises it.

| Risk | Mitigation |
|---|---|
| Detached cost regresses a hot path | cachegrind gate on every phase; per-function and per-object gating |
| Handler code corrupts VM state | one primitive (§4.3) used everywhere; GTests that force overruns and errors at each point |
| Off-graph references leak or trip `check_memory()` | `mark_hooks()` from phase 1; Debug suite with attachments live |
| A hook locks the mud up | tiers; no fail-closed on connection points; `hooks enabled : 0` |
| `function` is the hardest point (five sites) and now ships second | prototype the entry helper and its gate first in 2a, observe-only |
| Mudlibs misuse hooks as their stat system | §2.4 in the user docs |
| A master already defines `valid_hook` | release note; apply documentation |

## 8. Alternatives considered

**A. A DGD-style auto object instead of hooks.** Now part of this design as
the compile-time layer (§4.7), as a complement rather than an alternative: it covers
permanent efun policy at zero runtime cost, and it cannot cover runtime
attach/detach, misses, driver-initiated destructs, `call_other` traffic or
advice on functions it does not define.

**B. LDMud single-slot driver hooks.** One handler per event, set by
privileged code. Attachments here compose (several per join point, ordered by
priority) and are authorized per request, which LDMud leaves to a
mudlib-written dispatcher.

**C. More master applies, one per feature.** The status quo. Here the
master grows exactly one apply, `valid_hook()`, and the features become
mudlib code.

**D. A single master-designated hook daemon** (draft v2/v3:
`get_hook_daemon()`, the only object allowed to receive events and attach).
It answered the v1 review's security findings, but it put an extra LPC hop on
every advised call (per-instance gameplay effects would all dispatch through
one object) and created a second trust root beside the master. v4 follows the
`valid_*` pattern instead and answers the same findings directly: the master
sees the normalized filter and decides (observation scope); per-attachment
re-entry instead of a global guard (audit bypass); `HOOK_FAIL_CLOSED` and
`hook_detached()` (fences failing open); owner-destruct detach with re-attach
in `create()`, the `call_out()` idiom (`update` dropping attachments). v10
adds driver-computed tiers (§4.1), so the master cannot grant observation of
other people's data by accident. A dispatcher daemon remains a good *mudlib*
pattern (§6).

**E. External tracing (USDT/bpftrace).** Complementary for operators;
`hooks.cc` can emit a USDT probe at each join point at no extra cost.

**F. One join point per driver event** (drafts v7–v9: 36 point families,
`object:move`, `user:exec`, `call_out:fire`, `apply:<name>`, `net:telnet` …).
Most were exact duplicates of an efun or of function entry, each a second
site with its own re-validation rules, and the eleven validated cases used
four of them. v10 keeps seven points plus an `origin` filter; the rest are in
Appendix B.

## 9. Decisions

**Decided by the maintainer**

1. Authorization follows the `valid_*` pattern through the master; no
   designated hook daemon.
2. Names stay `hook_*`.
3. Hooks must observe **and** filter (decide on) calls.
4. Game design is in scope.
5. Everything stays in this one document.
6. Hooks should cover efuns, simul_efuns, applies, `call_other`, `call_out`,
   `heart_beat`, destruct, move and the network layer.
7. Avoid duplicate and low-value work.

**Resolved in v10, for the maintainer to confirm**

8. **Generic points instead of one point per event** (from 6 and 7). The v9
   catalogue had 36 point families; the eleven validated cases use four of
   them, and most of the rest were exact duplicates of an efun or of function
   entry (`object:move` is `efun:move_object`, the only caller of the driver's
   move; `user:exec`, `user:snoop`, `shadow:attach`, `object:clone`,
   `call_out:schedule` likewise; `apply:*`, `heart_beat`, `call_out:fire` and
   `simul:*` are function entry with a different origin). v10 keeps seven
   points and the §4.4 coverage map shows how each thing in decision 6 is
   hooked. The surveyed bespoke points are kept in Appendix B, to be added
   when a mudlib needs what they add.
9. **Network.** `net:accept` and `net:ws_upgrade` are planned (scenario S13:
   bans and rate limits before a session exists; nothing else can do it).
   `net:telnet`, `net:tls`, `net:read`, `net:write` and `net:resolve` have no
   use case yet and are in Appendix B; inbound GMCP/MSDP is hookable today
   through the `function` point on the telnet applies.
10. **Tiers computed by the driver** (§4.1). `valid_hook()` returns the tier
    it grants, so a naive `return 1` allows self-hooks only.
11. **One decision protocol**: refuse (string or 1) or answer (`({ value })`),
    with per-point meaning in §4.4.
12. **Handler time is charged to the caller**, within a per-call budget.
13. **Auto object** (§4.7): no driver change; if the recipe fails its
    end-to-end check in phase 1, the fallback is a one-line master apply
    `string get_auto_object()`. **Destruct veto**: by the auto object's
    `destruct()` override for explicit calls (or `efun:destruct` DECIDE);
    driver-initiated destructs are never vetoable.
14. **`HOOK_AROUND` ships last and gated**: a prototype must show one
    matching AROUND costs no more than about 1.5x a matching AFTER, and at
    least one mudlib use case from §2.2 must be confirmed as wanted. No
    validated case needs it today.
15. **Dropped from this RFC**: `call_limited()`, `hook_reset()`, per-protocol
    network points, driver-side throttling.

**Open**

16. Is charging handler time to the caller (12) acceptable, or should audit
    hooks installed by the mudlib be free for the code they observe?
17. Default for `hook eval cost` (proposed: one tenth of
    `maximum evaluation cost`).
18. Which Appendix B points, if any, should be promoted into the plan now.

## Appendix A: evidence from real mudlibs

### A.1 How real mudlibs do gameplay interposition today

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

### A.2 Validated use cases

Read from the libs the FluffOS project maintains: Discworld, Dead Souls,
Lima and Nightmare 3 (English), Genesis, and the restored Chinese libs in
[`fluffos/mudlibs`](https://github.com/fluffos/mudlibs) (es2, pkuxkx 北大侠客行,
fengyun434 风云, dtxyzjb 大唐西游, xyj2006n 西游记, xkx2017). Paths are
relative to each lib's root. Each case was checked against the code it
cites; where a hook is the wrong tool the case is listed under "No" instead.

Note: the Chinese XKX-family libs essentially never use `shadow()` (every
master but fengyun's returns `valid_shadow() == 0`), so their case for hooks
is not "replace shadows" but **misses, event sources and per-victim gates** in
`dbase`-style libs where every state change funnels through `/feature/*`.


**V1. Find calls to functions that do not exist** (S1, #1414) — **Hook**, `call_other:miss`.
*Evidence:* a call-vs-definition diff of pkuxkx found shipped gameplay bugs that
silently returned 0 for years: `me->set_busy(1)` in `d/jiaofei/npc/box.lpc:46`
and `d/shaolin/damodong.lpc:70` (no `set_busy` exists anywhere; the real one is
`start_busy()`), so those cooldowns never applied; `me->unconsious()` in
`kungfu/skill/xuanming-zhangfa/duipin.lpc:59-60` (0 definitions of the
misspelling, 297 of `unconcious`), so neither duellist is knocked out;
`me->is_fight(target)` at 14 sites, never defined; es2 calls `me->stop_busy()`
in 4 rooms (`d/chuenyu/east_castle.lpc:48`), never defined.
*Hook:* zero cost on hits; misses are rare.
```lpc
hook_attach("call_other:miss", (: log_misses :),
    ([ "label": "misses", "exclude": ([ "function": ({ "is_character", "got_fight" }) ]) ]),
    HOOK_BEFORE | HOOK_DEFERRED);
void log_misses(int id, mixed *events) {           // event: ({ actor, caller, target, fn, args, reason })
  foreach (mixed *e in events)
    log_file("misses", sprintf("%s -> %s::%s (%s)\n", base_name(e[1]), base_name(e[2]), e[3], e[5]));
}
```

**V2. Audit every exp/money change, wherever it is made** — **Hook**, `function` +
`HOOK_AFTER | HOOK_DEFERRED` + an argument filter (§4.2, added for this case).
*Evidence:* pkuxkx `adm/daemons/natured.lpc:157-190` `check_all_data()` polls
every user's money and exp and logs large deltas, but both scheduler calls are
commented out (`:147`, `:153`): the anti-dupe/anti-robot audit was switched
off as too costly. The intended chokepoint `REWARD_D->add_exp` is used at 123
sites, while **613** `add("combat_exp", …)` calls in 463 files and 171
`add("balance", …)` bypass it; all of them go through one function,
`feature/dbase.lpc:67 add(string prop, mixed data)`.
```lpc
hook_attach("function", (: audit :),
    ([ "label": "exp-money-audit", "defined_in": "/feature/dbase", "function": "add",
       "target": "/clone/user/user",
       "args": ([ 0: ({ "combat_exp", "balance", "potential" }) ]) ]),   // filtered in C
    HOOK_AFTER | HOOK_DEFERRED);
void audit(int id, mixed *events) {
  foreach (mixed *e in events)                   // ({ actor, caller, target, fn, args, result })
    if (abs(e[4][1]) > 5000)
      log_file("exp.log", sprintf("%s %s %+d by %s\n", e[2]->query("id"), e[4][0], e[4][1], base_name(e[1])));
}
```
Without the `args` filter, 99% of `add()` events (every other property) would
reach LPC only to be discarded.

**V3. Money conservation and dupe hunting** — **Hook**, `function` AFTER +
DECIDE fence. *Evidence:* Discworld `obj/money.c:160-195` `adjust_money()` is
the checked mutator but `:197 set_money_array()` is a second, unchecked one;
`:408-413` carries a one-off dupe-bug fix ("Zap the money array, this should
stop money duplication"); 40 files call the money API (shops, bureau de
change, bounty handler, `accept`). *Hook:* a deferred AFTER ledger on
`adjust_money`/`set_money_array` (`defined_in "/obj/money"`), plus
`HOOK_DECIDE | HOOK_FAIL_CLOSED` on `set_money_array` excluding
`caller_program` `/obj/` and `/std/shops/`. Mutations are ~1–5/s.

**V4. Quest and achievement credit on NPC death** — **Hook** (event source).
*Evidence:* dtxyzjb has **281 NPC files overriding `die()`** with copy-pasted
quest credit, e.g. `d/12gong/npc/renma.lpc:136-147` (the 12-palace chain, the
same block in each zodiac boss); the killer is stored inconsistently (an id
string in `feature/damage.lpc:367,557`, an object in
`std/char/cjnpc_easy.lpc:287`), so half the copies misbehave. *Hook:* one
`function die defined_in "/std/npc"` AFTER+DEFERRED attachment in the quest
daemon, with per-NPC rule data; it sees `die()` reached through `::die()`
chains and `call_out`s, which no `call_other` probe would.

**V5. Conditions that must block verbs** (禁止装备 / 封穴) — **Hook**, per-object
DECIDE with `expires`. *Evidence:* pkuxkx's `cannt_eq` condition is checked in
`cmds/std/wield.lpc:83` but **not in `wear.lpc`**, and six NPCs re-check it by
hand (`d/jiangzhou/npc/qd1.lpc:87 … qd6.lpc:93`); 123 condition daemons are
enforced only where a verb author remembered (`query_condition` read in 11
commands and 84 skill files).
```lpc
mixed sealed(int id, object actor, object caller, object victim, string fn, mixed *args) {
  tell_object(victim, "你穴道被封，无法装备！\n");
  return ({ 0 });                       // answer: wield()/wear() return 0 without running
}
hook_attach("function", (: sealed :),
    ([ "label": "cannt_eq", "object": victim, "defined_in": "/feature/equip",
       "function": ({ "wield", "wear" }), "expires": duration ]),
    HOOK_DECIDE);
```
Only the afflicted object pays (`O_HOOKED`); expiry replaces the manual countdown.

**V6. A safe zone that holds whatever route the damage takes** — **Hook**,
`function` DECIDE. *Evidence:* Dead Souls re-checks the room's `"no attack"`
property in 9 places (`verbs/players/attack.lpc:44`, `verbs/items/throw.lpc:58`,
`lib/combat.lpc:264,887`, `lib/living.lpc:246,264,464`,
`lib/events/shoot.lpc:33`, `secure/obj/weirder.lpc:375`), but
`lib/spell.lpc:648` deals damage through `eventReceiveDamage()` with no check —
the hole this pattern always leaves. *Hook:* a backstop DECIDE on
`eventReceiveDamage` (`defined_in "/lib/body"`) that refuses when the
victim's environment has `"no attack"` and the caller is living; verbs keep
their friendly refusals. `hook_list(player)` explains "why did my spell fail".

**V7. Follow and escort without re-registering on every room** — **Hook**,
`efun:move_object` AFTER on one object. *Evidence:* Lima
`std/modules/m_follow.c:193-211` removes and re-adds `person_left` /
`object_arrived` hooks on the old and new room on **every** move, and only
works where a room's exit code reaches `call_hooks` (Lima's 205 hand-placed
points). *Hook:* the follower attaches `efun:move_object` with
`"caller_object": leader` (`move_object()` moves its caller) and `HOOK_AFTER`,
and moves itself to `args[0]` once the leader has arrived; only the leader
pays.

**V8. Deprecation telemetry** — **Hook**, `count`. *Evidence:* Discworld logs
obsolete-API callers by hand (`obj/handlers/money_handler.c:405-410`,
`nmoney_handler.c:356`, `log_file("OBSOLETE_CALLS", …)`), at 4 sites only;
every other retired function has no telemetry. *Hook:*
`hook_attach("function", "count", ([ "target": "/obj/handlers/money_handler",
"function": "query_alias_for", "label": "obsolete" ]), 0)`: rows by caller
program, no source edit, nothing when detached.

**V9. A parser-rule fence instead of grepping source** — **Auto object**
(**Hook** as the runtime form). *Evidence:* Dead Souls
`secure/daemon/master.lpc:525-545` `read_file()`s the **whole source of every
object it loads or clones** and `strsrch()`es for `"parse_add_rule"` /
`"SetRules"`, denying outside `ParserDirs`: a file read per clone, and
defeated by an include, an inherit, or `efun::`. *Auto object:* a `protected
nomask parse_add_rule()` override that checks the caller's directory, with
`valid_override()` closing `efun::`; zero runtime cost. *Hook:* the same rule
as `efun:parse_add_rule` with `HOOK_DECIDE | HOOK_FAIL_CLOSED`, switchable
without recompiling.

**V10. Wizard-action forensics that are cheap enough to leave on** — **Hook**
(observation) + **auto object** (policy). *Evidence:* Dead Souls logs every
`destruct()` with `get_stack(1)` only under `DESTRUCT_LOGGING`, which
`secure/include/config.h:69` sets to **0**: too expensive to leave on;
Discworld `modified_efuns.c:427-462` wraps `call_out` to count zero-delay
call_outs per object and log floods. *Hook:* `count`/`time` on
`efun:destruct`, `efun:call_out`, `efun:snoop`, `efun:exec` — aggregated in C,
no stack capture on the hot path, attachable while hunting — plus
`object:destruct` for the destructs no wrapper sees. These libs issue the
efuns from simul_efun wrappers, which is why §4.1 makes wrappers transparent
rather than exempt (and why such hooks are tier 3).

**V11. Cleanup when a room's destruct cascades** — **Hook**,
`object:destruct`. *Evidence:* pkuxkx `adm/simul_efun/object.lpc:73-87`
wraps `destruct` to run area `move_out` and `ob->remove()` bookkeeping, but
objects destructed *with* their environment never pass through it.
*Hook:* `hook_attach("object:destruct", (: cleanup :), ([ "label":
"destruct-cascade" ]), 0)` acting on `cause == "environment"`; low rate,
nothing otherwise.

### A.3 Checked and rejected (not a hook)

| Case | Evidence | Why not |
|---|---|---|
| Timed stat buffs, `add_temp("apply/…")` + `call_out("remove_effect")` | pkuxkx 373 files with the manual negative undo; 51 of 58 skills using `call_out("remove_effect")` never `remove_call_out` (re-cast double-applies); `kungfu/skill/xueshan-jianfa/xuejian.lpc:88-100` | The consumers (`query_skill`, `query_temp("apply/…")`) are the hottest getters in combat; the lib owns the chokepoint. Fix with a `BUFF_D` that owns expiry and cleanup |
| PK / newbie / `no_fight` gates | pkuxkx `cmds/std/kill.lpc:12-56` nine rules, copies in `hit`, `ansuan`, `touxi`; `feature/attack.lpc:116 kill_ob()` checks none | `kill_ob()` *is* the chokepoint; move the rules there (a 20-line edit). Only an expiring live-ops rule earns a hook |
| Discworld PK pair rules | `secure/simul_efun/pk_check.c:33`, 47 call sites (theft, drag, food splash, combat) | A rule about a *pair* of players asked before unrelated verbs: a simul_efun question is the right shape |
| Guard NPCs reacting to killers | `init()` attack logic in 357 pkuxkx NPC files; `d/beijing/npc/jinyiwei.lpc:46-72` | `init()` fires per move × room inventory; per-NPC data in an inherit is the right tool |
| Genesis guild shadows and combat states | `std/guild/guild_base.lpc:93-133`; `std/combat/cbase.lpc:1830-1876` (nomask) | Guild shadows *add functions* (a non-goal); combat states already have a chokepoint |
| Dead Souls damage protections | `lib/body.lpc:705-745` `AddMagicProtection` list with ordering and owner cleanup | The lib already has the modifier pipeline (the single-slot `SetProtect` at `:1964` is an LPC bug) |
| `destruct` policy (cleanup, `remove()`) | pkuxkx `adm/simul_efun/object.lpc:73-87` | Permanent efun policy already central in the simul_efun; only the cascade (V11) needs a hook |

The rule in §2.4 held: hooks lose wherever the mudlib already owns a single
chokepoint (4 of the 7). They win for misses, event sources over legacy code,
per-instance gates with expiry, and observation cheap enough to leave on.

## Appendix B: surveyed points, available on demand

Two source surveys mapped these candidates to their sites. They are not in
the plan because a generic point already delivers the event, or because no
use case exists yet. Each row records what a bespoke point would add and the
hazard found, so the work is not lost. Paths are under `src/`.

| Candidate | Covered today by | A bespoke point would add | Site and hazards |
|---|---|---|---|
| `object:move` (before/decide/after) | `efun:move_object` (the only caller of `move_object()`; eviction goes through the `move_or_destruct` apply, which calls the efun) | `from` and a `reason`, an "after" before `init()` runs | `vm/internal/simulate.cc` `move_object()`. After any LPC inside it, the recursion, shadow and destructed checks must all be re-run (today only destructed); never fire inside `setup_new_commands()`, which iterates the inventory with prefetched pointers |
| `object:clone`, `object:load` | `efun:clone_object`/`new`/`load_object`; `object:create`; `valid_object()` is the load veto | a load cause (`file`/`virtual`/`source`) | `load_object()` has three sibling entry points |
| `object:reset`, `object:clean_up` | `function`, origin `driver` | nothing | `object.cc` `reset_object()`, `backend.cc` |
| `object:recompile`, `object:replace_program`, `shadow:attach` | `efun:<name>` | the moment `replace_program()` takes effect | `simulate.cc`, `packages/core/replace_program.cc`, `efuns_main.cc` |
| `shadow:detach` | nothing (driver-initiated) | the event itself; no use case | `destruct_object()`, `reload_object()`, `replace_programs()`, `remove_shadow()`; pointers half-updated, observe only |
| `call_out:schedule`, `call_out:remove` | `efun:call_out`, `efun:remove_call_out` | nothing | `packages/core/call_out.cc` |
| `call_out:fire` | `function`, origin `internal` | functional `(: … :)` call_outs and the handle; a refusal must still reject the entry's promise and free it | `call_out.cc` `call_out()`; events are snapshotted before dispatch, so scheduling inside a handler is safe |
| `heart_beat`, `heart_beat:set` | `function` `heart_beat`, origin `driver`; `efun:set_heart_beat` | driver-initiated heart_beat off | `packages/core/heartbeat.cc` |
| `simul:<name>`, `apply:<name>`, `call_other` as separate points | `function` with `origin` | a spelling | `simul_efun.cc` `call_simul_efun()`; `apply_low()`; master applies take two routes (`call_direct()` and `apply_low()`) |
| `user:logon` | `function` `logon`, origin `driver` | port and address as arguments | `comm.cc` |
| `user:input` (decide: drop a line) | `function` `process_input` (observe only: a refusal there would disable the apply) | dropping a line; bodies with no `process_input` | four input sites (`comm.cc` and three in `net/transport_libevent.cc`) need one helper; re-check `IP_VALID` after a handler; no-echo lines must be redacted |
| `user:command` | `function` on the verb function | verb and argument before dispatch | `packages/core/add_action.cc` `user_parser()`; the sentence must be snapshotted (with refs) before any handler, or a handler that moves or destructs anything frees it |
| `user:disconnect` with a cause | `function` `net_dead`; `object:destruct` | a transport cause (`eof`/`error`/`tls`/`protocol`), and the case where the body is destructed (no apply fires today) | `comm.cc` `remove_interactive()`; needs `BEV_EVENT_ERROR` split from `EOF` |
| `user:exec`, `user:snoop` | `efun:exec`, `efun:snoop` (DECIDE at tier 3) | nothing | `packages/core/interactive.cc`, `comm.cc` |
| `socket:event` | `function` on the callback | functional callbacks | `packages/sockets/socket_efuns.cc` `call_callback()` |
| `net:telnet` | `function` on `gmcp`/`msdp`/`telnet_suboption`/`window_size`/`terminal_type` | negotiation (WILL/DO…) and pre-logon events; dropping a subnegotiation before its apply | `net/telnet.cc` `telnet_event_handler()`; negotiation cannot be refused (libtelnet has already replied); a handler that disconnects the user on the read path must be deferred, not immediate |
| `net:tls` | nothing | handshake outcome, version, cipher (no client-cert or SNI data exists) | `transport_libevent.cc` `on_user_events()` |
| `net:read`, `net:write` | nothing | byte counts per user or port | counting in C or coalesced batches only: per-segment LPC is attacker-paced, and `add_vmessage()` formats into a static buffer that a re-entering handler would clobber |
| `net:resolve` | `efun:resolve` (the request) | completion of the driver's own reverse lookup | `packages/core/dns_libevent.cc` |
| inline output hooks | `efun:write`/`tell_object`/`message`; `receive_message`/`catch_tell` applies | — | rejected: static-buffer re-entry |
| `call_limited(function, eval_cost, …)` | — | a sub-budget for a handler that fans out to many subscribers | must clamp to and charge the caller's budget, or it is an eval-limit escape |
| USDT probes at each join point | — | zero-cost external tracing with `bpftrace` | Linux only |

## Appendix C: reproducing the probes

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

## Appendix D: review log

Section numbers in rows before v10 refer to the draft current at the time.
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
| Maintainer (v4) | A designated hook daemon is an unnecessary second trust root and an extra hop on every advised call | Authorization through master `valid_hook()`, like `valid_shadow`/`valid_socket`; Layer 1 events became ordinary join points; ownership, re-attach and revocation rules in §5.1; security model §5.5 rewritten |
| Maintainer (v5) | Keep everything in one document | Auto object as Layer 0 (§5.8; probes show inherited `protected` overrides replace efuns and `inherit` via `#include` works); destruct veto via the auto object, none in the driver; `HOOK_AROUND` behind a prototype gate (§11) |
| Real mudlibs (v6: English libs; Chinese libs from `fluffos/mudlibs`) | 11 valid cases with file:line evidence (§2B), incl. shipped pkuxkx/es2 bugs a miss report would have caught and a disabled exp/money audit; 7 honest "no" cases. Gaps: simul_efun exemption would blind `efun:*` probes on every lib that wraps efuns; fixed efun list; no argument-value filter; `nomask` policy; efun subject; batch shape | Simul_efun wrappers transparent (§5.1); any efun hookable at dispatch with type re-check (§5.3); `args` filter; `HOOK_OVERRIDE_NOMASK`; subject rule and batch shape (§5.9); non-goal "adding functions" (§2A); API reference §5.9; full plan §8 |
| Maintainer (v7): cover efun, simul_efun, apply, call_other, call_out, heart_beat, destruct, move, etc. | Two read-only surveys mapped every candidate to its site. Found: `(: ob, "fn" :)` is not a call route; `function` has no free flag bit (per-program bitmap instead); master applies take two routes; deny on `reset`/`clean_up`/`process_input` would permanently disable them; `object:destruct` placement double-fired; `move_object()` must re-validate recursion after LPC; `user:command` must snapshot the sentence; inline output hooks would clobber a static buffer | §5.10 normative catalogue (calls, objects, users, other, rejected) with sites, kinds, args, frequency, gates and rules; eight extra primitive rules; §8.1 phases regrouped into tracks covering every point |
| Maintainer (v9): add hooks for "net" | A read-only survey of `src/net`, `comm.cc`, `telnet.cc`, `ws_*.cc`, `dns_libevent.cc`, `wasm/comm_wasm.cc` | `net:accept` (refuse before allocation), `net:ws_upgrade`, `net:tls`, `net:telnet`, `net:read`/`net:write` (counting only), `net:resolve`; transport cause on `user:disconnect`; rules for lws callbacks, pre-logon identity, deferred read-path disconnects and bounded address tables; all in phase 3 |
| Maintainer (v8) | Too many phases | Four phases |
| Second review round (v10): consistency, scope and value, security of the `valid_hook()` model, implementer dry run | 31 contradictions left by nine revisions (efun timing vs `valid_*`, destruct placement, deny semantics per point, examples using deny-listed points, phase-1 examples needing phase-4 features). 36 point families of which the validated cases use four; most bespoke points duplicate an efun or function entry. Without the daemon, observation scoping had regressed to "the master should be careful": a naive `valid_hook()` exposed typed passwords, simul_efun transparency undid `valid_override()`, a handler's own extent hid foreign writes, observers could mutate arguments, hooked calls were free of eval cost, and a fail-closed hook on input or accept could lock the mud with no way back. The primitive missed state the error path clobbers (`restrict_destruct`, the heart_beat object) and phase 1 depended on phase-2/4 features | Document rewritten as one design with a single catalogue; seven points plus `origin` filter and a coverage map, the rest in Appendix B; driver-computed tiers; secret redaction; tier-3 delivery from simul frames; per-cause re-entry; one-level argument copies; one decision protocol (refuse / answer); handler time charged to the caller; skip status and kill switch; primitive rewritten against the source; phases reordered by validated value (core, function, efuns and connections, around) with a PR split and a harness mapping for tests |

