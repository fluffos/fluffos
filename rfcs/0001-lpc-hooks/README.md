# RFC 0001: In-game hooks — eBPF-style join points for LPC

| | |
|---|---|
| Status | Draft v6 — six-angle review (v2), game-design round (v3), `valid_hook()` authorization (v4), auto object, destruct veto and `HOOK_AROUND` resolved in this document (v5), 11 validated real-mudlib use cases and the plan completed (v6); see [§12](#12-review-log). Decisions in [§11](#11-decisions) |
| Issues | #1414 (call_other miss report) is the first consumer |
| Prior art | LDMud `set_driver_hook()`, `H_DEFAULT_METHOD`, `limited()`, `trace()`, Python hooks; DGD auto object, driver-object applies, kernellib object/error managers, `call_touch()`, `rlimits`, `atomic` |
| Evidence | Probes in [`probes/`](probes/), run on master `b5714e5f` (RelWithDebInfo, gcc) |

## 1. Summary

Mudlibs keep asking for small cross-cutting driver features: report a
`call_other` to a missing function (#1414), fence a directory off from callers,
count calls, account object lifetimes, and, in gameplay, curses, protections
and world events that today are faked with `shadow()`. This RFC proposes one
small mechanism instead of one feature per request, split the way eBPF is
split:

* **The driver is the kernel.** It defines a fixed set of join points, makes
  them free when unused, runs handlers in a contained frame, and offers cheap
  C-side filtering and counting for high-frequency events.
* **Authorization follows the `valid_*` pattern.** Any object may call
  `hook_attach()`; the driver normalizes the request and asks the master's
  `valid_hook(who, point, action, spec, flags)`, exactly as `shadow()` asks
  `valid_shadow()` and sockets ask `valid_socket()`. Only a return of `1`
  allows. No `valid_hook()` in the master means no hooks.
* **Hooks observe and filter calls.** Attachments can observe or deny
  `call_other`s and selected efuns, see `call_other` misses and object
  creation/destruction, and, for gameplay, advise any function by every call
  route (local calls included) on one object instance or a whole program,
  before, after or around it (§2A, §5.7).
* **Policy is LPC.** Who may hook what is the master's decision; dedup,
  reports, status-effect bookkeeping and builder-facing rule tables are mudlib
  code. Reference policies and an example effects daemon ship with the docs.

The driver surface is two layers, each earned by scenarios the layer below
cannot serve:

| Layer | What | Earned by |
|---|---|---|
| 0 | No driver change: document the mechanisms that already cover a scenario, including a DGD-style **auto object** built from the global include file and the master's `include_file()` apply (§5.8) | S2 (dev), S3, S5 (with a base object), S8; permanent efun policy and an explicit-`destruct()` veto |
| 1 | Three rare join points, zero cost on the normal path: `call_other_miss`, `object_created`, `object_destructed` | S1, S5 (no base object, with cause), S7 |
| 2 | Call observation and filtering: `call_other`, `efun:*` (any efun) and `function` join points that observe, deny or advise, with per-program/per-function/per-object gating, pointer-keyed filters, `count`/`time` actions and positional handler arguments | S2 (runtime), S4, S6, S9–S12, G1–G15 |

Both layers use the same efuns and the same `valid_hook()` gate. Compile-time
advice on efuns (a DGD-style auto object) is part of Layer 0 and needs no
driver change (§5.8).

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

**Non-goal: adding functions.** Hooks advise functions that exist; they do
not add new functions to an object. Shadows that exist to *add* an interface
(Genesis guild shadows adding `query_guild_name_occ()` and friends,
`std/guild/guild_base.lpc:93-133`) stay shadows or become inherits.

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

**Trust.** Hooks are a system / live-ops / game-designer instrument, and the
master's `valid_hook()` decides who may use them on what. Builder- and
player-facing features are best exposed by a mudlib daemon as **rule data**
(zone prefix, program, function, verdict, expiry), so builders never need
hook code of their own.

## 2B. Validated use cases from real mudlibs

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

### 2B.1 Valid (11)

**V1. Find calls to functions that do not exist** (S1, #1414) — **Hook**, `call_other_miss`.
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
hook_attach("call_other_miss", (: log_misses :),
    ([ "label": "misses", "exclude": ([ "function": ({ "is_character", "got_fight" }) ]) ]),
    HOOK_BEFORE | HOOK_DEFERRED);
void log_misses(int id, mixed *events) {           // event: ({ actor, caller, target, fn, args, reason })
  foreach (mixed *e in events)
    log_file("misses", sprintf("%s -> %s::%s (%s)\n", base_name(e[1]), base_name(e[2]), e[3], e[5]));
}
```

**V2. Audit every exp/money change, wherever it is made** — **Hook**, `function` +
`HOOK_AFTER | HOOK_DEFERRED` + an argument filter (§5.3, added for this case).
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
hook_attach("function", (: "你穴道被封，无法装备！\n" :),
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
points). *Hook:* the follower attaches `efun:move_object` with `"object":
leader` and `HOOK_AFTER` and moves itself when the leader has arrived; only
the leader pays (§5.9: the subject of `move_object(dest)` is the moved object).

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
`object_destructed` for the destructs no wrapper sees. These libs issue the
efuns from simul_efun wrappers, which is why §5.1 makes wrappers transparent
rather than exempt.

**V11. Cleanup when a room's destruct cascades** — **Hook**,
`object_destructed`. *Evidence:* pkuxkx `adm/simul_efun/object.lpc:73-87`
wraps `destruct` to run area `move_out` and `ob->remove()` bookkeeping, but
objects destructed *with* their environment never pass through it.
*Hook:* `hook_attach("object_destructed", (: cleanup :), ([ "label":
"destruct-cascade" ]), 0)` acting on `cause == "environment"`; low rate,
nothing otherwise.

### 2B.2 Not a hook (7)

| Case | Evidence | Why not |
|---|---|---|
| Timed stat buffs, `add_temp("apply/…")` + `call_out("remove_effect")` | pkuxkx 373 files with the manual negative undo; 51 of 58 skills using `call_out("remove_effect")` never `remove_call_out` (re-cast double-applies); `kungfu/skill/xueshan-jianfa/xuejian.lpc:88-100` | The consumers (`query_skill`, `query_temp("apply/…")`) are the hottest getters in combat; the lib owns the chokepoint. Fix with a `BUFF_D` that owns expiry and cleanup |
| PK / newbie / `no_fight` gates | pkuxkx `cmds/std/kill.lpc:12-56` nine rules, copies in `hit`, `ansuan`, `touxi`; `feature/attack.lpc:116 kill_ob()` checks none | `kill_ob()` *is* the chokepoint; move the rules there (a 20-line edit). Only an expiring live-ops rule earns a hook |
| Discworld PK pair rules | `secure/simul_efun/pk_check.c:33`, 47 call sites (theft, drag, food splash, combat) | A rule about a *pair* of players asked before unrelated verbs: a simul_efun question is the right shape |
| Guard NPCs reacting to killers | `init()` attack logic in 357 pkuxkx NPC files; `d/beijing/npc/jinyiwei.lpc:46-72` | `init()` fires per move × room inventory; per-NPC data in an inherit is the right tool |
| Genesis guild shadows and combat states | `std/guild/guild_base.lpc:93-133`; `std/combat/cbase.lpc:1830-1876` (nomask) | Guild shadows *add functions* (a non-goal); combat states already have a chokepoint |
| Dead Souls damage protections | `lib/body.lpc:705-745` `AddMagicProtection` list with ordering and owner cleanup | The lib already has the modifier pipeline (the single-slot `SetProtect` at `:1964` is an LPC bug) |
| `destruct` policy (cleanup, `remove()`) | pkuxkx `adm/simul_efun/object.lpc:73-87` | Permanent efun policy already central in the simul_efun; only the cascade (V11) needs a hook |

The rule from §2A held: hooks lose wherever the mudlib already owns a single
chokepoint (4 of the 7). They win for misses, event sources over legacy code,
per-instance gates with expiry, and observation cheap enough to leave on.

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
  (`kernellib/src/doc/kernel/hook/driver:19-84`). Mudlibs can build the same
  shape on top of hooks (the example effects daemon in §7 does).
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

### 5.1 Authorization: `valid_hook()`

```lpc
// master
int valid_hook(object who, string point, mixed action, mapping spec, int flags);
```

* **Any object may call `hook_attach()`.** Before asking the master, the
  driver normalizes `spec`: unknown keys and malformed patterns are rejected;
  program and object names are canonicalized (`filename_to_obname()`, `#n`
  stripped for program keys) so `/secure/login`, `/secure/login.c` and
  `//secure/login` are one spelling; prefixes only with a trailing `/`. The
  master therefore decides on exactly what the driver will match.
* **Fail closed.** Only an exact `1` allows. An absent apply, `0`, any other
  value, a promise (AGENTS.md §13.24) or an error denies, and `hook_attach()`
  errors. Unlike `master_approved()`, there is no "master not loaded yet"
  allow: attaching before the master exists is denied.
* **The master sees everything it needs to scope.** `who` (the attaching
  object), the join point, the action (whose function pointer owner the
  handler will run as), the normalized filter and the flags (`HOOK_DECIDE`,
  `HOOK_AROUND`, `HOOK_FAIL_CLOSED` ...). A reference master (§7) allows
  wizards only filters whose target, defining program or caller lies under
  their own directory, keeps `HOOK_DECIDE`/`HOOK_AROUND` on others' code for
  admin objects, and refuses protected functions.
* **Ownership.** The attachment belongs to `who`. The handler runs as its
  function pointer's owner, like any other function pointer. The attachment is
  detached when its owner or its handler's owner is destructed; objects that
  need a hook re-attach in `create()`, the same idiom as `call_out()` and
  `set_heart_beat()`. `recompile_object()` keeps identity, so attachments
  survive it.
* **Visibility and control.** `hook_list()` / `hook_query()` /
  `hook_detach()` work on the caller's own attachments; the master sees and
  may detach all of them, so a demoted wizard's hooks can be revoked.
* **Hard exemptions, regardless of `valid_hook()`.** Nothing fires for calls
  made by the master, during compile, or inside `valid_hook()`, any other
  `valid_*` apply, or `error_handler`. No attachment may name the master or
  simul_efun program as `target`, `defined_in` or `object`. Operations started
  by the master are never denied.
* **Simul_efun wrappers are transparent, not exempt.** Real mudlibs issue
  `destruct`, `call_out`, `move_object`, `exec` and `snoop` from inside
  simul_efun wrappers (Dead Souls `secure/sefun/sefun.lpc:464-493`, Lima
  `secure/simul_efun/overrides.c`, Nightmare `secure/SimulEfun/SimulEfun.lpc:56-129`,
  Discworld `modified_efuns.c:427-462, 762-800`). Exempting the simul_efun
  object would blind every `efun:*` hook on those libs. Instead, a call made
  from simul_efun code is observed and attributed to the object that called
  the simul_efun (`caller` is that object; `caller_program` its program), and
  may be denied like any other call.
* **Cost when unused.** Each join point has a bit in `g_hook_mask`, set while
  at least one attachment exists for it; otherwise the check is one predicted
  branch.

### 5.2 Layer 1: rare join points

Attached like any other join point (`hook_attach()` + `valid_hook()`):

| Point | Fires | Site | Handler and protocol |
|---|---|---|---|
| `call_other_miss` | A `call_other` (`->`, explicit, array element, `(: ob, "fn" :)`) found nothing callable, after the shadow chain | inside `apply_low()`, only when `local_call_origin == ORIGIN_CALL_OTHER`: the one place all three dispatch routes converge, where the reason is known and the arguments are still on the stack | `h(int id, object actor, object caller, object target, string fn, mixed *args, string reason)`; `reason` is `"undefined"`, `"private"`, `"static"`, `"protected"` or `"destructed"`. With `HOOK_DECIDE` a handler may **claim** the call by returning a one-element array `({ value })`: `value` becomes the result (a plain 0 could not be told apart from "declined"). Anything else declines; the first claim in priority order wins; with no claim the caller gets `undefined` as today |
| `object_created` | After `create()` returns (in `call_create()`), not if `create()` destructed the object | `src/vm/internal/base/object.cc` `call_create()` | `h(int id, object ob)`; observe |
| `object_destructed` | At the start of `destruct_object()`, before the existing `O_DESTRUCTED` re-check | `src/vm/internal/simulate.cc` `destruct_object()`, beside `on_destruct` | `h(int id, object ob, string cause)`; observe; `cause` is `"efun"`, `"shadowed"`, `"environment"`, `"refused"` (`valid_object` denial / `creator_file` failure) |

S1 is `hook_attach("call_other_miss", "count", ([ "label": "typos" ]), 0)`
from a dev tool; it prints the rows and filters out known optional hooks
(`query_*`, `is_*`) when it reports. Misses are rare, and hits cost nothing.

Errors stay with `error_handler` (S8): it already runs at throw time with the
trace live, and a second error hook adds nothing but another way to recurse.

### 5.3 Layer 2: call probes

High-frequency join points cannot call LPC on every event without becoming
the 3.4x wrapper again. Layer 2 attachments ("probes") get C-side gating,
filtering and aggregation:

```lpc
int hook_attach(string point, mixed action, mapping spec, int flags);
                              // spec: filter keys + "label", "priority", "expires" (§5.7)
void hook_detach(int id);
mapping hook_query(int id);   // rows, events, dropped, errors, last_error
void hook_reset(int id);
mapping *hook_list(object|void ob); // own attachments, or those applying to ob (§5.7); all for the master
```

Every `hook_attach()` goes through `valid_hook()` (§5.1); the other efuns act
on the caller's own attachments, or on any attachment when called by the
master.

| Point | Fires | Site |
|---|---|---|
| `call_other` | a resolved `call_other`, before the function runs (so `HOOK_DECIDE` can deny) | `apply_low()`, after lookup, before `push_control_stack()`, `ORIGIN_CALL_OTHER` only |
| `function` | entry to a hooked function by any route (local, inherited, `call_other`, funptr, `call_out`, `heart_beat`) — added for game design, §5.7 | function entry in the interpreter, per-function gate |
| `efun:<name>` | before **any** efun runs (a small deny-list excepted: the `hook_*` efuns, `call_limited`, and efuns the hook machinery itself calls) | the efun dispatch sites (`F_EFUN0`–`F_EFUN3`, `F_EFUNV`, efun function pointers), gated by a per-efun bit in the instruction table set while an attachment exists for that efun; a reviewer measured a bit test before *every* efun dispatch as below noise. After a handler runs, the driver latches/restores `st_num_arg` and **re-runs the efun's generic argument type check**, so a handler that destructed an object argument produces a clean "bad argument" error instead of a stale pointer. `efun:*` fires before the efun's own `valid_*` checks, so observers also see attempts the efun then refuses (useful for alarms); a decision can only deny, never grant what a `valid_*` refuses |

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
| `args` | a mapping from argument position to an exact value or array of values (strings, ints, objects), compared in C; e.g. `([ 0: ({ "combat_exp", "balance" }) ])` for `add(string prop, …)` (§2B V2). Honoured by `count`/`time` too |
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

Every handler call, at any join point, goes through one
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
| Per-attachment re-entry guard, not a global "no hooks inside hooks": an attachment never observes its own handler, but everything a handler does is observed by every *other* attachment | A global guard would let anyone bypass an audit hook by doing their writes from inside their own handler |
| Layer 1 never re-enters itself (`g_in_hook` per join point) | recursion |
| Detach on the first "stale function pointer" or owner-destructed error; otherwise after `hook max errors` (default 10). `HOOK_FAIL_CLOSED` attachments never auto-detach. The owner is told via an optional `hook_detached(id, reason)` apply | §3 dangling funptrs; a fence that silently disappears |
| Attachments, handler funptrs and filters marked in `checkmemory.cc` (`mark_funp`, after `mark_call_outs()`'s pattern); count tables are C++ containers bounded by `hook max rows` with a `dropped` counter | §3 off-graph references; unbounded memory from attacker-chosen names |
| `this_player()` is 0 inside every handler, restored afterwards (the `actor` argument says who acted) | a handler acting as the victim (`input_to`, `command`) |
| Detaching inside a handler marks the attachment; it is swept after dispatch | iterator invalidation (§13.14) |

Eval limits are enforced on Linux only (`src/vm/internal/eval_limit.cc`); on
macOS, Windows and WASM the handler shares the caller's wall clock. Document
it.

### 5.5 Security model

* **The master is the trust root**, as for `shadow()`, sockets, `bind()`
  and every other privileged capability. `valid_hook()` sees the normalized
  filter, the kind and the handler owner, so it can enforce scope. The main
  threat is **observation**: an attachment sees the arguments of every call it
  matches (passwords sent to a login daemon, tells, file contents). The
  reference master (§7) therefore lets ordinary wizards attach only to events
  whose target, defining program or caller lies under their own directory,
  and reserves unscoped observation, `HOOK_DECIDE` and `HOOK_AROUND` on
  others' code, and `efun:*` decisions for admin objects.
* **Hard exemptions** regardless of `valid_hook()` (§5.1): nothing fires for
  calls made by the master, during compile, or inside `valid_*`/`error_handler`;
  nothing may target the master or simul_efun program; the master's
  operations are never denied. Simul_efun wrappers are transparent: their
  efun calls are attributed to the object that called them.
* **Decisions are narrow.** `call_other_miss` can only *supply* a result for a
  call that found nothing; `HOOK_DECIDE` elsewhere can only *deny*. Nothing can
  grant what a `valid_*` apply refused: `efun:*` probes fire after the efun's
  own `valid_*` check.
* **Revocation.** Authorization is checked at attach time; the master can
  list and detach any attachment, and `hook_detached()` tells the owner.
* **Resource abuse.** Each handler has its own eval budget, errors and
  overruns count toward `hook max errors`, count tables are bounded by
  `hook max rows`, and `valid_hook()` can cap attachments per owner using
  `hook_list()`.
* **Upgrade hazard.** A mudlib whose master already defines a function named
  `valid_hook` for its own purposes would answer the new apply. Release notes
  must say so, and the apply's documentation must tell mudlib maintainers to
  check for it before upgrading.

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

The §2A cases need four things the administrative design lacks. All go
through `hook_attach()` and `valid_hook()` like the rest of Layer 2.

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
* `nomask` is respected by default: AROUND and DECIDE attachments skip
  `nomask` functions (libraries such as Genesis make their combat pipeline
  `nomask` precisely to stop shadows, `std/combat/cbase.lpc:44,2122`). An
  attachment may cover them only with `HOOK_OVERRIDE_NOMASK`, which
  `valid_hook()` sees in `flags`. BEFORE/AFTER observation is always allowed.
* AROUND, AFTER and DECIDE are refused on driver-questioned applies (the
  generated `object_applies_table` names, `id`, `catch_tell`, every `valid_*`)
  except an allow-list (`heart_beat`, `init`), and on functions a mudlib marks
  protected (refused by the master's `valid_hook()`, after Discworld's `player.c` blacklist:
  `query_name`, `query_creator`, `save_me`, money/exp/auth functions).

**4. Lifetime, state and introspection.**

* `hook_attach(string point, mixed action, mapping spec, int flags)`: `spec`
  holds the filter keys plus `"label"` (required), `"priority"` and
  `"expires"` (seconds; driver-side expiry so 300 status effects are not 300
  `call_out`s that leak when their owner reloads; **not allowed with
  `HOOK_FAIL_CLOSED`**, so a fence cannot lapse silently). Per-effect state
  rides on the handler as bound arguments: `(: fumble, ([ "tries": 0 ]) :)`.
* `hook_list(object ob)` lists every attachment that applies to `ob` (object
  filters and program/function filters), with label, owner, expiry, hit count
  and handler eval time. The example effects daemon (§7) renders it as a visible
  **status-effects list** for players and builders, which replaces Dead Souls'
  shadow registry and Discworld's `sh_adows` tool, and answers "why did that
  rat hit me for 40?". Error traces and `call_stack()` mark advised frames
  (`[hook #12 blood_moon]`).
* Hooks are runtime state: nothing survives a reboot, and per-object effects
  do not survive the target's destruct-and-reload. Their owner re-attaches them
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

**Sub-budget calls.** A handler that fans out to several subscribers (an
effects daemon, an event bus) shares one `hook eval cost`, so one slow
subscriber starves the rest. An optional efun `call_limited(function f, int
eval_cost, mixed args...)` (LDMud's `limited()`) runs `f` under its own
budget using the §5.4 primitive's save/restore, and returns `({ result })` or
0 on overrun or error. It is useful beyond hooks, and lands in phase 4c.

**Deferred or rejected here.** Environment/room-tree filters (an LPC
`environment()` walk in the handler is ~100 ns; add only if measured hot);
output/message
transform probes and per-player phasing (rejected, see §2A.2).

### 5.8 Compile-time layer: the auto object (Layer 0)

DGD gives every program an implicit parent, the auto object, which may
redefine kfuns and reach the originals with `::kfun()`; kernellib builds its
whole security and resource model there (§4.2). FluffOS can do the same today:

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
  combination is **not yet verified**; phase 0 verifies it and falls back to
  a one-line master apply if needed (§11).

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

### 5.9 API reference (normative summary)

Everything a mudlib sees, in one place. `include/hooks.h` defines the
constants.

**Master apply**

```lpc
int valid_hook(object who, string point, mixed action, mapping spec, int flags);
// exactly 1 = allow; absent, 0, other values, promise, error = deny
```

**Optional apply on the attaching (owner) object**

```lpc
void hook_detached(int id, string reason);
// reason: "expired", "target_destructed", "handler_owner_destructed",
//         "errors", "stale_function", "master"
```

**Efuns**

| Efun | Who may call | Returns |
|---|---|---|
| `int hook_attach(string point, mixed action, mapping spec, int flags)` | anyone; gated by `valid_hook()` | attachment id (> 0); errors when refused or malformed |
| `void hook_detach(int id)` | owner or master | — |
| `mapping hook_query(int id)` | owner or master | `([ "id", "point", "owner", "label", "flags", "priority", "expires", "spec", "events", "errors", "dropped", "last_error", "eval_us", "last_fired", "rows": ({ ([ "caller_program", "target_program", "function", "count", "eval_us" ]) }) ])` |
| `void hook_reset(int id)` | owner or master | clears counters and rows |
| `mapping *hook_list(object\|void ob)` | anyone (own attachments); master (all) | one `hook_query()`-shaped mapping per attachment, without `rows`; with `ob`, only attachments that apply to `ob` |
| `mixed call_limited(function f, int eval_cost, mixed args...)` | anyone | `({ result })`, or 0 on overrun or error |

**Join points**

| Point | Kinds allowed | Handler arguments after `(int id, …)` | Site |
|---|---|---|---|
| `call_other_miss` | BEFORE, DECIDE (claim with `({ value })`) | `object actor, object caller, object target, string fn, mixed *args, string reason` | `apply_low()`, `ORIGIN_CALL_OTHER`, after shadows |
| `object_created` | BEFORE | `object ob` | `call_create()` |
| `object_destructed` | BEFORE | `object ob, string cause` | top of `destruct_object()` |
| `call_other` | BEFORE, AFTER, DECIDE, AROUND | `object actor, object caller, object target, string fn, mixed *args` | `apply_low()`, after lookup, before the frame is pushed |
| `efun:<name>` (any efun but the §5.3 deny-list) | BEFORE, AFTER, DECIDE | same; `target` is the efun's **subject** (its first object argument, else the calling object: for `move_object(dest)` the moved object, i.e. the caller), `args` are the efun's arguments | efun dispatch, per-efun bit; type check re-run after the handler |
| `function` | BEFORE, AFTER, DECIDE, AROUND | same | function entry in the interpreter, per-function gate |

`HOOK_AFTER` handlers get `mixed result` appended; `HOOK_AROUND` handlers get
`function proceed` appended. `HOOK_DEFERRED` handlers get
`(int id, mixed *events)`, in order; each event is the array of the handler
arguments after `id`, e.g. `({ actor, caller, target, fn, args, result })` for
`HOOK_AFTER` on `function`, `({ actor, caller, target, fn, args, reason })`
for `call_other_miss`, `({ ob, cause })` for `object_destructed`. Objects
destructed before delivery are 0.

**Flags**: exactly one kind (`HOOK_BEFORE` = 0 default, `HOOK_AFTER`,
`HOOK_DECIDE`, `HOOK_AROUND`), optionally `HOOK_FAIL_CLOSED` (with
`HOOK_DECIDE`), `HOOK_DEFERRED` (with `HOOK_BEFORE`/`HOOK_AFTER`) or
`HOOK_OVERRIDE_NOMASK` (with `HOOK_AROUND`/`HOOK_DECIDE`, §5.7).

**Actions**: `"count"`, `"time"`, or a function pointer.

**Spec keys**: the filter keys of §5.3 and §5.7 (`target`, `defined_in`,
`function`, `caller`, `caller_program`, `object`, `caller_object`, `args`,
`exclude`),
plus `"label"` (string, required), `"priority"` (int, default 0) and
`"expires"` (seconds; refused with `HOOK_FAIL_CLOSED`).

**Runtime config**

| Key | Default | Meaning |
|---|---|---|
| `hook eval cost` | the value of `maximum evaluation cost` | budget of one handler call |
| `hook max errors` | 10 | errors (incl. overruns) before an attachment auto-detaches; never for `HOOK_FAIL_CLOSED` |
| `hook max rows` | 10000 | rows per `count`/`time` table; overflow counted in `dropped` |
| `hook max deferred` | 1024 | ring-buffer size per `HOOK_DEFERRED` attachment |

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

## 7. Reference code (shipped with the docs and the testsuite)

* **A reference `valid_hook()`** for the testsuite master and the docs:
  admins (by directory or euid) may attach anything; wizards only to events
  whose `target`, `defined_in`, `object` or `caller` lies under their own
  directory, observe-only (`HOOK_BEFORE`/`AFTER`, `count`, `time`); protected
  functions (`query_name`, `query_creator`, `save_me`, money/exp/auth) are
  refused for everyone but admins; a per-owner cap on live attachments.
* **Examples:** S1 typo finder (`call_other_miss` + `count`, filtering
  `query_*`/`is_*` when printing); S5 lifecycle counts with cause; S4
  `/secure/` fence (`HOOK_DECIDE | HOOK_FAIL_CLOSED`).
* **An example effects daemon** (optional mudlib component, not a driver
  concept): status effects as hook attachments with labels and expiry, a
  player-visible effects list from `hook_list(ob)`, re-attach at login and
  `restore_object()`, and a rule-data API for builders (§2A trust note).

## 8. Implementation plan

Each phase is one PR, reviewed and merged before the next one starts.
Phases 0–1 deliver #1414 and the lifecycle events; phases 2–3 the
administrative probes; phase 4 the game-design layer.

### 8.1 Phases

| Phase | Scope | Driver files | Tests | Exit criteria |
|---|---|---|---|---|
| **0** | Docs and examples only: `docs/concepts/general/interposition.md` (`valid_object` + `on_destruct`, `valid_write`, the tracer, `error_handler`, `valid_override`, the transparent `call_other` wrapper and its cost) and the **auto-object recipe** (§5.8) | none (fallback: `get_auto_object()` in `simulate.cc`/compiler if the recipe fails) | testsuite: auto object injected into ordinary files only; an efun override used; `efun::` refused by a reference `valid_override()`; an explicit `destruct()` vetoed | recipe verified end to end, or the fallback apply merged |
| **1a** | Core: `valid_hook`; spec normalization; `hook_attach`/`hook_detach`/`hook_list`/`hook_query`; ownership and owner-destruct detach; `g_hook_mask`; the contained-invocation primitive (§5.4); `call_other_miss`; `hook_detached` | new `src/vm/internal/hooks.{h,cc}`; `src/packages/core/hooks.spec` + `hooks.cc`; `src/vm/internal/apply.cc` (`apply_low`); `src/vm/internal/applies` (`valid_hook`, `hook_detached`); `src/vm/internal/simulate.cc` (owner detach in `destruct_object`); `src/base/internal/rc.cc` + `runtime_config.h` (four keys); `src/packages/develop/checkmemory.cc` (`mark_hooks`) | LPC `tests/efuns/hook_attach.lpc` (authorization matrix: absent / 0 / 2 / promise / error / pre-master), `hook_miss.lpc` (each route and reason, claim, decline, promise/error declined), `hook_ownership.lpc`; GTest `test_hooks.cc` (eval-state save/restore after a forced overrun, spec normalization) | closes #1414; benchmark gate §5.6 passes detached |
| **1b** | `object_created`, `object_destructed` with cause | `object.cc` (`call_create`), `simulate.cc` (`destruct_object`, causes) | LPC: each driver destruct cause reported once; self-destructing `create()` not reported; Debug `check_memory()` clean with attachments live | merged |
| **2a** | `call_other` probes: filters, `PROG_HOOKED`, pointer-keyed set, `count`/`time`, `HOOK_DECIDE` + `HOOK_FAIL_CLOSED`, per-attachment re-entry | `hooks.cc`, `apply.cc`, `program.h` (bit), `simulate.cc` (recompute on compile) | LPC: every filter key incl. `exclude` and canonicalization; deny with message; fail-closed on error/overrun; a handler's own calls not seen by itself but seen by another attachment; detach inside a handler | gate incl. attached cases (1 and 100 non-matching filters, one matching handler) |
| **2b** | `efun:*` probes for any efun: per-efun bit at the dispatch sites, `st_num_arg` latch, type check re-run, subject rule, simul_efun attribution | `interpret.cc` (`F_EFUN*`), `function.cc` (efun funptrs), efun table generation (`make_func`) for the bit | LPC: `efun::` calls observed; calls from simul_efun wrappers attributed to the real caller; a handler destructing an efun argument gets a clean error; deny-listed efuns refused at attach; `move_object` subject is the moved object | detached gate on an efun-heavy loop |
| **3** | Reference `valid_hook()` and examples (§7); `docs/concepts/general/hooks.md`; efun and apply pages; `include/hooks.h`; `docs/driver/config.md` regenerated; release note about pre-existing `valid_hook` functions | testsuite master and `testsuite/single/hooks/` | the examples (S1, S4, S5) run in the suite | docs build; `gen_config_docs.py` / `gen_sidebar.py` clean |
| **4a** | `function` join point (per-function gate, every route); `object`/`caller_object` filters with `O_HOOKED`; `HOOK_BEFORE`/`HOOK_AFTER`; `actor`; target-destruct detach | `interpret.cc` (frame setup), `program.h`/`function_t` flags, `object.h` (`O_HOOKED`), `hooks.cc` | LPC: local, inherited, funptr, `call_out`, `heart_beat` routes all observed; unflagged instances untouched; target destruct detaches | gate incl. the §5.7 local-call cases |
| **4b** | AROUND prototype, then (if it passes the §11 gate) `HOOK_AROUND` with `proceed`, priorities, the chain, return-type check, apply allow-list, protected functions | `hooks.cc`, `interpret.cc` | LPC: curse (deny), clamp (skip `proceed`), disguise (`/secure/` excluded), stacking by priority independent of attach order, failure runs the original unmodified, attach/detach mid-call affects the next call only | prototype numbers recorded in the PR |
| **4c** | `expires`, `hook_list(ob)`, labels in traces and `call_stack()`, `HOOK_DEFERRED`, `call_limited()` | `hooks.cc`, `trace.cc`, backend tick | LPC: expiry and `hook_detached(id, "expired")`; refused with `HOOK_FAIL_CLOSED`; batches revalidate destructed objects; `call_limited` overrun returns 0 and leaves the caller's eval intact | merged |
| **4d** | Example effects daemon (mudlib code, §7) and the §2B real-world cases V1–V11 as testsuite scenarios | testsuite only | G1, G2, G7, G8 and the §2B examples end to end | merged |

### 8.2 Required on every phase

* Debug + sanitizer build: GTests and the LPC suite clean (no `check_memory()`
  report, no ASan/UBSan finding).
* RelWithDebInfo: LPC suite twice (randomized order), GTests.
* The §5.6 cachegrind gate on the detached cases, numbers pasted in the PR.
* Every regression test fails on the unfixed driver (AGENTS.md §7).
* Docs for anything user-visible land in the same PR.

### 8.3 Rollout

* **Off by default.** Without a `valid_hook()` in the master nothing can
  attach, so existing mudlibs see no behaviour change; the only cost is the
  detached branches.
* **Release notes** for phase 1a call out the `valid_hook` name collision
  (§5.5) and point to the reference policy.
* **The testsuite master** ships the reference policy so CI exercises it.

### 8.4 Risks

| Risk | Mitigation |
|---|---|
| Detached cost regresses a hot path | cachegrind gate on every phase; per-function and per-object gating |
| Handler code corrupts VM state | one contained-invocation primitive (§5.4), reused everywhere; GTests that force overruns and errors |
| Off-graph references leak or trip `check_memory()` | `mark_hooks()` from phase 1a; Debug suite with attachments live |
| `HOOK_AROUND` too slow | prototype gate in 4b; BEFORE/AFTER/DECIDE ship regardless |
| Mudlibs misuse hooks as their stat system | §2A "No" list in the docs; examples show modifier APIs where they belong |
| Pre-existing `valid_hook` in a master | release note; apply documentation |

## 9. Security invariants (each has a test)

1. No event fires when the caller is the master; during compile; inside
   `valid_hook`, any other `valid_*`, or `error_handler`. An efun called from
   a simul_efun wrapper **is** observed, with `caller` = the object that called
   the simul_efun.
2. `hook_attach()` errors unless `valid_hook()` returns exactly 1 (absent apply, 0, other values, promise, error, master not yet loaded all deny); `hook_detach`/`hook_query` on another owner's attachment errors unless called by the master.
3. A `call_other_miss` handler that returns anything but a one-element
   array (0, a promise, an error) leaves the caller with `undefined`, as if
   nothing were attached.
4. `efun:*` probes observe `efun::` calls and everything other attachments'
   handlers do; an attachment never observes its own handler.
5. `HOOK_FAIL_CLOSED`: handler error or overrun denies; the attachment
   stays; `hook_detached` is not called for it.
6. `this_player()` is 0 inside every handler, and the
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
12. Destructing an attachment's owner, or its handler's owner, detaches it;
    an attachment never outlives the code that would run.

## 10. Alternatives considered

**A. A DGD-style auto object instead of hooks.** Now part of this design as
Layer 0 (§5.8), as a complement rather than an alternative: it covers
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
in `create()`, the `call_out()` idiom (`update` dropping attachments). A
dispatcher daemon remains a good *mudlib* pattern (§7).

**E. External tracing (USDT/bpftrace).** Complementary for operators;
`hooks.cc` can emit a USDT probe at each join point at no extra cost.

## 11. Decisions

Decided by the maintainer:

1. **Authorization follows the `valid_*` pattern**: any object may call
   `hook_attach()`; the master's `valid_hook(who, point, action, spec, flags)`
   decides, fail-closed. This supersedes the earlier "master-supplied hook
   daemon" decision: the master still controls who hooks what, without a
   designated receiver object.
2. **Names stay `hook_*`**: `hook_attach`, `hook_detach`, `hook_query`,
   `hook_reset`, `hook_list`, `HOOK_DECIDE`, `HOOK_FAIL_CLOSED`, master apply
   `valid_hook`, owner callback `hook_detached`. Mudlib vocabulary such as
   Lima's `add_hook`/`call_hooks` does not collide with efuns; the one real
   collision risk is a master that already defines `valid_hook` (§5.5).
3. **Hooks must be able to observe and filter calls**: Layer 2
   (`call_other`, `efun:*` and `function` probes, observe and `HOOK_DECIDE`)
   is part of this RFC, not a later option.
4. **Game design is in scope** (§2A, §5.7): `function` join point,
   per-object targeting, `HOOK_BEFORE`/`AFTER`/`DECIDE`/`AROUND` with
   priorities.

Settled by the game-design review: driver-side `expires` is allowed (status
effects are the dominant use and 300 `call_out`s leak when their owner reloads) but
refused with `HOOK_FAIL_CLOSED`, so a fence can never lapse silently.

5. **Everything stays in this one document** (maintainer): the auto object,
   the destruct veto and the `HOOK_AROUND` decision are resolved here, not in
   follow-up RFCs.

Resolved in this document (v5), for the maintainer to confirm:

6. **Auto object**: Layer 0, built from the global include file and
   `include_file()`, no driver change (§5.8). If the end-to-end check in
   phase 0 fails, the fallback is a one-line master apply
   `string get_auto_object()` that the compiler injects as an implicit
   inherit; that is the only driver change the auto object could need.
7. **Destruct veto**: no driver mechanism. Explicit `destruct()` calls are
   vetoed by the auto object's `destruct()` override (raise an error to
   refuse), which covers LDMud's `prepare_destruct` use for mudlib code.
   Driver-initiated destructs (shadow teardown, environment contents, refused
   loads) stay unvetoable by design: refusing them would leave half-destroyed
   state. `object_destructed` observes all of them.
8. **`HOOK_AROUND`** ships behind a prototype gate (phase 4b): a prototype
   measures one matching AROUND against one matching `HOOK_AFTER` on the same
   function. If AROUND costs no more than ~1.5x AFTER per advised call (the
   `proceed` closure is the extra), it ships; otherwise phase 4 ships
   `BEFORE`/`AFTER`/`DECIDE` and AROUND waits for a cheaper `proceed`
   (for example a reusable per-chain closure instead of one per call).

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
| Maintainer (v4) | A designated hook daemon is an unnecessary second trust root and an extra hop on every advised call | Authorization through master `valid_hook()`, like `valid_shadow`/`valid_socket`; Layer 1 events became ordinary join points; ownership, re-attach and revocation rules in §5.1; security model §5.5 rewritten |
| Maintainer (v5) | Keep everything in one document | Auto object as Layer 0 (§5.8; probes show inherited `protected` overrides replace efuns and `inherit` via `#include` works); destruct veto via the auto object, none in the driver; `HOOK_AROUND` behind a prototype gate (§11) |
| Real mudlibs (v6: English libs; Chinese libs from `fluffos/mudlibs`) | 11 valid cases with file:line evidence (§2B), incl. shipped pkuxkx/es2 bugs a miss report would have caught and a disabled exp/money audit; 7 honest "no" cases. Gaps: simul_efun exemption would blind `efun:*` probes on every lib that wraps efuns; fixed efun list; no argument-value filter; `nomask` policy; efun subject; batch shape | Simul_efun wrappers transparent (§5.1); any efun hookable at dispatch with type re-check (§5.3); `args` filter; `HOOK_OVERRIDE_NOMASK`; subject rule and batch shape (§5.9); non-goal "adding functions" (§2A); API reference §5.9; full plan §8 |

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
