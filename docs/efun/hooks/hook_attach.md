---
title: hooks / hook_attach
---
# hook_attach

### NAME
    hook_attach - attach a handler at a driver join point

### SYNOPSIS
    int hook_attach(string point, mixed action, mapping spec, int flags);

### DESCRIPTION
    Registers `action` at the named join `point`. Returns a positive
    attachment id on success. Authorization goes through the master apply
    [valid_hook](/apply/master/valid_hook). See the
    [Hooks](/concepts/general/hooks) concept page for join points, handler
    signatures, tiers, and eval credit.

    Wired points: `call_other:miss`, `apply:miss`, `object:create`,
    `object:destruct`, `function`, `efun:<name>`, `net:accept`,
    `net:ws_upgrade`, `net:read`, `net:write`, `net:telnet`, `net:tls`,
    `net:resolve`. `call_other:miss`, `apply:miss`, and `function` require
    `object` or `target` in `spec` (live object or a name pattern: exact,
    `"path/"`, `"path*"`, or `"*"`). The same pattern rules apply to
    `function` and `efun:<name>` (e.g. `efun:set*`).

    `action` is a function pointer, or `"count"` / `"time"` for C-side
    aggregation. `"time"` requires `function` + `HOOK_AFTER`. `flags`
    selects the kind (`HOOK_BEFORE` / `HOOK_DECIDE` / `HOOK_AFTER` /
    `HOOK_AROUND`; event points accept `0` for the point's kind) plus
    optional `HOOK_FAIL_CLOSED`. `HOOK_DEFERRED` and
    `HOOK_OVERRIDE_NOMASK` are reserved and rejected. Constants are in
    `include/hooks.h`.

### SEE ALSO
    hook_detach(3), hook_query(3), hook_list(3), hook_proceed(3), valid_hook(4)
