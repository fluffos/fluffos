---
title: Hooks
---

# Hooks

FluffOS hooks are fixed join points in the driver: a small set of events LPC
can attach to, free when unused, with authorization through the master's
`valid_hook()`. Handlers run on an alternate VM stack and do not charge their
eval time to the code they observe (subject to a per-evaluation budget). This
page is the execution-model specification for what is shipped today; efun
signatures are under [efun/hooks](/efun/hooks/). Runnable demos for every use
case below live in `testsuite/single/tests/efuns/hooks_usecases.lpc`.

## Join points

One attach API (`hook_attach` / `detach` / `query` / `list`), one authorization
path (`valid_hook` tiers), one contained-invocation model (alt stack + eval
credit). Status below is what the driver actually does today.

| Point | When | Kinds | Status |
|---|---|---|---|
| `call_other:miss` | `ob->fn(...)` undefined, private, protected, or target destructed. **Requires** `object` or `target` | BEFORE, DECIDE | Wired |
| `apply:miss` | Same miss reasons via `apply()` / `safe_apply()` for every origin **except** `call_other`. **Requires** `object` or `target` | BEFORE, DECIDE | Wired |
| `object:create` | After `create()` returns, object still alive | event (`0`) | Wired |
| `object:destruct` | After destruct re-check, before stack sweep | event (`0`) | Wired |
| `function` | Named LPC function entered by any route; filter `origin`. **Requires** `object` or `target` | BEFORE, AFTER, DECIDE, AROUND | Wired |
| `efun:<name>` | At efun dispatch (`efun:*` matches all) | BEFORE, AFTER, DECIDE (refuse only) | Wired |
| `net:accept` | New connection, before session allocation | BEFORE, DECIDE | Wired |
| `net:ws_upgrade` | Websocket upgrade | BEFORE, DECIDE | Wired |
| `net:read` / `net:write` | Inbound / outbound user bytes | BEFORE, AFTER (`write`), DECIDE | Wired |
| `net:telnet` | Telnet WILL/WONT/DO/DONT/IAC/SB | BEFORE; DECIDE on `"sb"` only | Wired |
| `net:tls` | TLS handshake completed or failed | AFTER | Wired |
| `net:resolve` | Driver reverse-DNS of a user address finished | AFTER | Wired |

Miss reasons: `"undefined"`, `"private"`, `"protected"`, `"destructed"`.
Target is the object named before the shadow walk. Master-object callers never
raise `call_other:miss` or `apply:miss`. The two miss points are exclusive by
origin: LPC `ob->fn(...)` / `call_other` only raise `call_other:miss`; driver
applies (`id`, `create`, …), efun-driven applies, `call_out` named functions,
and other non-`call_other` origins raise `apply:miss`.

Simul_efun wrappers stay transparent for `efun:*` (the underlying efun still
fires). Hooked efuns still run the normal argument type checks before the
hook sees them. Efun aliases are resolved at attach time to the primary
`instrs[]` name (e.g. `efun:clone_object` matches the same dispatch as
`efun:_new`, which is what `new()` compiles to). Prefix patterns match
aliases too (`efun:clone*` hits `_new`).

`HOOK_DEFERRED` and `HOOK_OVERRIDE_NOMASK` are reserved constants:
`hook_attach()` rejects them until the driver implements delivery / nomask
bypass.

### Use cases

| Use case | Point / kind | Demo |
|---|---|---|
| **Typo / miss finder** ([#1414](https://github.com/fluffos/fluffos/issues/1414)) | `call_other:miss` BEFORE — unique `(caller, target, fn, reason)` | `demo_typo_finder` |
| **Deprecation radar** (renamed / removed names) | `call_other:miss` BEFORE, filter `function` / `target` | `demo_deprecation_radar` |
| **Access audit** | `call_other:miss` BEFORE on `"private"` / `"protected"` | `demo_access_audit` |
| **Destructed-call noise** | `call_other:miss` BEFORE on `"destructed"` | `demo_destructed_noise` |
| **Default method / proxy** | `call_other:miss` DECIDE → `({ value })` | `demo_default_proxy` |
| **Compat shim** for one renamed API | `call_other:miss` DECIDE | `demo_compat_shim` |
| **Virtual / generated objects** | `call_other:miss` DECIDE, `object`/`target` prefix (`"path/"` or `"path*"`) | `demo_virtual_proxy` |
| **Hot miss profile** | `call_other:miss` action `"count"` | `demo_hot_miss_profile` |
| **Missing apply finder** (silent driver / efun applies) | `apply:miss` BEFORE — pin `object` / `target` (+ optional `function` / `origin`) | `demo_apply_miss_finder` |
| **Apply-side default / proxy** | `apply:miss` DECIDE on a pinned `object` / `target` | `demo_apply_miss_proxy` |
| **Live object census** | `object:create` / `destruct` (or `"count"`) | `demo_object_census` |
| **Leak / clone storms** | `object:create` + `"count"` rate signal | `demo_clone_storm` |
| **Destruct cause telemetry** | `object:destruct` (`efun` / `environment` / `shadowed` / `refused`) | `demo_destruct_cause` |
| **Session / instance bookkeeping** | create / destruct on a path prefix | `demo_session_bookkeeping` |
| **Domain quotas (soft)** | create tracking + alert threshold | `demo_quota_soft` |
| **Domain quotas (hard)** | `efun:clone_object` (alias of `_new`) DECIDE refuse | `demo_quota_hard` |
| **Capability fence** (`/domains/` ↛ `/secure/`) | `function` DECIDE, `origin: "call_other"` | `demo_capability_fence` |
| **Live-function deprecation** | `function` BEFORE + `"count"` | `demo_live_deprecation` |
| **Status effects** (silence, invuln) without `shadow()` | `function` DECIDE on `object` = victim | `demo_status_effect` |
| **Quest / achievement / gossip sources** | `function` AFTER | `demo_quest_after` |
| **Hot-function profile** | `function` `"count"` / `"time"` (AFTER) | `demo_hot_function_profile` |
| **Privilege efun alarms** | `efun:shutdown` / `seteuid` / … BEFORE | `demo_privilege_alarm` |
| **`call_out` leak finder** | `efun:call_out` + `"count"` | `demo_call_out_leak` |
| **Pre-accept connection ban** | `net:accept` DECIDE | `demo_net_accept_api` |
| **Protocol / rate audits** | `net:read` / `net:write` / `net:telnet` / `net:tls` / `net:resolve` | `demo_net_rw_api` |
| **Polymorph / disguise** (exclude secure callers) | AROUND + `exclude_caller_program` | `demo_polymorph` |
| **Damage scale / shop pricing / A/B** | AROUND on the relevant `function` | `demo_around_rewrite` |

**Mudlib auto object** (every clone inherits a common base): recipe in the
mudlib only — inherit list / simul_efun / `compile_object`. No driver
change. Fallback if needed: master `string get_auto_object()`. Explicit
`destruct()` veto is a mudlib override (or `efun:destruct` DECIDE);
driver-initiated destructs are never vetoable.

### Out of scope

- Hot-patching bytecode (`recompile_object()`)
- Per-opcode tracing / debugger (#1286)
- Command / message auditing already covered by `process_input`,
  `receive_message`, `catch_tell`

### Suggested mudlib adoption order

1. Miss reporter in master or a secure daemon (`call_other:miss` and
   `apply:miss`).
2. Create/destruct census for packages you reload often.
3. Replace `shadow()` sites that only need refuse/observe (`function` DECIDE).
4. Privilege `efun:` alarms, then `net:accept` refuses.
5. AROUND only where rewriting is actually required.

## Attaching

```c
#include <hooks.h>

int id = hook_attach(
    "call_other:miss",
    (: on_miss :),
    ([ "label": "typo-finder", "object": this_object() ]),
    HOOK_BEFORE
);
```

- **`hook_attach(point, action, spec, flags)`** — returns a positive id, or
  errors on denial / bad arguments. `action` is a function pointer, or the
  string `"count"` / `"time"` for C-side aggregation (no LPC handler).
  `"time"` is only valid with `function` + `HOOK_AFTER` (body duration in
  microseconds, reported in `hook_query` rows as `"eval_us"`).
- **`hook_detach(id)`** — remove an attachment (no-op if already gone).
- **`hook_query(id)`** / **`hook_list(owner | void)`** — inspect live
  attachments.

Constants (`HOOK_BEFORE`, `HOOK_DECIDE`, `HOOK_AFTER`, `HOOK_AROUND`,
`HOOK_FAIL_CLOSED`, `HOOK_DEFERRED`, `HOOK_OVERRIDE_NOMASK`) live in the
driver's `include/hooks.h`.

### Spec keys

| Key | Meaning |
|---|---|
| `label` | String shown in `hook_query` / `hook_list` |
| `priority` | Higher runs first within a kind (default 0) |
| `expires` | TTL in wall-clock seconds from attach; `0` = never. `hook_query` reports the absolute expiry time |
| `object` | Target filter: a live **object**, or a **string** name pattern (same rules as `target`). A master object pin also matches its clones; a clone pin stays instance-exact |
| `caller_object` | Caller filter: a live **object**, or a **string** name pattern (same rules as `caller`) |
| `function` | Match this function name (same pattern rules) |
| `target` / `caller` | Match canonical object name (no leading `/`). Matching ignores a live object's `#clone` suffix |
| `caller_program` | Match caller program filename (`prog->filename`, no leading `/`) |
| `exclude_caller_program` | Skip when the caller's program matches |
| `origin` | Match call origin (`"call_other"`, `"local"`, `"driver"`, `"simul_efun"`, `"function_pointer"`, `"functional"`, `"efun"`, `"internal"`). `function` and `apply:miss` only |

**Name patterns** (shared by `object`/`target`/`caller`/`caller_object` strings, `function`, `caller_program`, `exclude_caller_program`, and `efun:<name>`):

| Pattern | Matches |
|---|---|
| `"foo"` | Exact name |
| `"foo/"` | Prefix (path convention; the `/` is part of the prefix) |
| `"foo*"` | Prefix (single trailing `*`) |
| `"*"` | Any name |

Only one `*`, and only alone or at the end — `"a*b"` is rejected at attach. Do not combine a string `object` with `target` (or string `caller_object` with `caller`).

Attaching or detaching inside a handler affects the **next** event. The
list for the current dispatch is snapshotted when the event starts.

### Handler signatures

**`call_other:miss`**

```c
mixed handler(int id, object actor, object caller, object target,
              string fn, mixed *args, string reason);
```

- `HOOK_BEFORE`: return value ignored (observe only).
- `HOOK_DECIDE`: return `({ value })` to answer the miss (that value becomes
  the `call_other` result); any other return is no opinion. A string or `1`
  refuse is reserved for decide points that can block; on a miss it does not
  change the outcome. `HOOK_FAIL_CLOSED` is **not** allowed on this point.
  Miss points have no AROUND — do not use `hook_proceed` here.
- **Requires** `object` or `target` in the attach spec (same as `apply:miss`
  / `function`).

**`apply:miss`**

```c
mixed handler(int id, object actor, object caller, object target,
              string fn, mixed *args, string reason, string origin);
```

Same BEFORE / DECIDE rules as `call_other:miss`, but `origin` is
`origin_to_name()` for the apply (`"driver"`, `"efun"`, `"internal"`, … — never
`"call_other"`). Answering with `({ value })` becomes the apply result.
There is no AROUND / `hook_proceed` on miss points — DECIDE is the answer path.

**Scope is required** (same rule as `call_other:miss` / `function`):
`object` or `target` (live object, exact name, `"path/"` / `"path*"` prefix,
or `"*"`). Optional `function` / `origin` filters narrow further. Exact
object pins skip unrelated targets before walking handlers (a master pin
also admits its clones). `HOOK_FAIL_CLOSED` is not allowed.

**`object:create`**

```c
void handler(int id, object ob);
```

**`object:destruct`**

```c
void handler(int id, object ob, string cause);
```

`cause` is one of `"efun"`, `"shadowed"`, `"environment"`, `"refused"`.

**`function`** (and the same positional shape for `efun:<name>`):

```c
mixed handler(int id, object actor, object caller, object target,
              string fn, mixed *args);
// AFTER appends: mixed result
```

- `HOOK_BEFORE`: observe only.
- `HOOK_DECIDE`: return `({ value })` to answer (skip the body); a string or
  `1` to refuse; `0` for no opinion. `efun:*` DECIDE is refuse-only (no
  answer); a refuse skips the efun and yields `0`.
- `HOOK_AFTER`: sees the return value (`0` for an efun that returns
  nothing; call args are not preserved — the `args` slot is an empty array).
- `HOOK_AROUND` (`function` only): highest-priority match replaces the body.
  Call `hook_proceed()` or `hook_proceed(mixed *args)` once to run the real
  call; the handler's return value becomes the call result. Returning
  without `proceed` still supplies the result (no body).
- **`function` requires** `object` or `target` (a bare `function` name
  filter is not enough). `efun:<name>` does not.
- **`function` filters use the named target** (before the shadow walk),
  same as miss points, so a pin on the victim still matches under
  `shadow()`.

**`net:accept` / `net:ws_upgrade`**: `handler(int id, string addr)` — DECIDE
refuse closes the fd / rejects the upgrade.

**`net:read` / `net:write`**: `handler(int id, object user, int len, string data)`
— all BEFORE observers run, then DECIDE (refuse drops the payload).
`data` is a length-`len` byte string (embedded NULs are preserved; `len`
is authoritative). `net:write` AFTER runs after the payload is handed to
the telnet/transport layer (past DECIDE and the driver's write call); it
is not a kernel-level send-completion callback.

**`net:telnet`**:
`handler(int id, object user, string event, string option, string payload)`
— `event` is `"will"` / `"wont"` / `"do"` / `"dont"` / `"iac"` / `"sb"`;
`option` is a TELOPT name (`"GMCP"`, `"NAWS"`, …) or a numeric string;
`payload` is the SB body (empty otherwise). BEFORE always runs; DECIDE on
`"sb"` only (refuse skips the driver's SB handler / applies). Negotiation
cannot be refused (libtelnet has already replied). `user` may be `0`
pre-logon while the session is still bound to master. DATA/SEND stay on
`net:read` / `net:write`.

**`net:tls`**:
`handler(int id, object user, int port, string version, string cipher, string outcome)`
— AFTER when a TLS accept handshake finishes (`outcome` `"ok"`) or errors
(`"error"`). `version` / `cipher` are filled on success.

**`net:resolve`**:
`handler(int id, string address, string name, object user)` — AFTER the
driver's reverse-DNS of a connected user's address succeeds (the lookup
started from `connect()`). This is not the mudlib `resolve()` efun
completion; that remains `efun:resolve` plus the LPC callback.

## Authorization: `valid_hook`

```c
int valid_hook(object who, string point, mixed action, mapping spec,
               int flags, int tier);
```

The driver computes `tier` and asks the master to grant **at least** that
tier. Return `0` to deny; return a positive tier to grant. A naive
`return 1` therefore allows only self-hooks.

| Tier | Meaning (shipped rules) |
|---|---|
| 1 | Self-pinned: filters name `who` as `object` or `caller_object`, handler owned by `who`, not fail-closed |
| 2 | Observe (`BEFORE`/`AFTER`) on a shipped point, handler owned by `who`, with at least one scope key |
| 3 | Everything else |

After the master is recompiled, every live attachment is re-validated and
detached if no longer granted. Destroying the owner, the handler's owner,
or a pinned `object` / `caller_object` also detaches automatically. There
is no detach callback — track ids yourself if you need to know when an
attachment is gone.

## Performance

When nothing is attached, `g_hook_mask` is zero. Hot paths (function call,
efun dispatch, `add_message`, user input, create/destruct) test that mask
with one predicted branch and do **not** call into the hook engine. Point
bits stay separate (`HOOK_MASK_LPC_FUNC` vs `HOOK_MASK_EFUN`) so an
efun-only attach does not wake the LPC call path (and vice versa). Fine
bits on `HOOK_MASK_LPC_FUNC_*` further gate work by kind: an AFTER-only
attachment does not walk BEFORE/AROUND lists or take timestamps.
`call_other:miss`, `apply:miss`, and `function` require `object` or
`target` at attach time. When every attachment for one of those points
uses a live-object pin (no name-pattern string), events on other objects
skip dispatch entirely. A master pin's basename is indexed too, so its
clones still reach handlers. Name patterns (`"*"`, `"foo*"`, `"foo/"`)
force a full scan. Eval-timeout continuation uses a separate flag
(`g_hook_eval_active`), not the join-point mask.

## Contained invocation

Handlers run on a **separate VM stack** so they do not steal the caller's
control-stack depth. Their wall time is **not charged** to the observed
evaluation: the driver keeps a lazy eval credit and restores the caller's
timer afterward.

That exemption is capped by the config key `hook eval cost` (default equal
to `maximum evaluation cost`). A handler that burns past the budget is
stopped; a runaway pack of handlers cannot stall the mud unboundedly.

Other config keys: `hooks enabled` (master switch; `0` makes
`hook_attach` fail), `hook max errors`, `hook max attachments`,
`hook max rows`, `hook max deferred` (reserved). See
[Runtime configuration](/driver/config).

## Example: miss reporter

```c
#include <hooks.h>

nosave mapping seen = ([ ]);

void on_miss(int id, object actor, object caller, object target,
             string fn, mixed *args, string reason) {
    string key = sprintf("%O|%O|%s|%s",
                         caller ? file_name(caller) : 0,
                         target ? file_name(target) : 0,
                         fn, reason);
    if (seen[key]) return;
    seen[key] = 1;
    debug_message(sprintf("call_other miss: %s\n", key));
}

void setup() {
    hook_attach("call_other:miss", (: on_miss :),
                ([ "label": "typo-finder", "object": this_object() ]),
                HOOK_BEFORE);
}
```

## Example: DECIDE proxy

```c
mixed on_miss(int id, object actor, object caller, object target,
              string fn, mixed *args, string reason) {
    if (fn == "legacy_alias")
        return ({ "ok" });   // call_other result becomes "ok"
    return 0;                // no opinion
}

void setup() {
    hook_attach("call_other:miss", (: on_miss :),
                ([ "object": this_object(), "function": "legacy_alias" ]),
                HOOK_DECIDE);
}
```

## See also

- [hook_attach](/efun/hooks/hook_attach), [hook_detach](/efun/hooks/hook_detach),
  [hook_query](/efun/hooks/hook_query), [hook_list](/efun/hooks/hook_list),
  [hook_proceed](/efun/hooks/hook_proceed)
- [valid_hook](/apply/master/valid_hook)
- [shadow](/efun/calls/shadow) — the older per-object interposition mechanism
  hooks are meant to complement (and, for many gameplay cases, replace)
