---
title: master / valid_hook
---
# valid_hook

### NAME
    valid_hook - authorize a hook attachment request

### SYNOPSIS
    int valid_hook(object who, string point, mixed action, mapping spec,
                   int flags, int tier);

### DESCRIPTION
    Called from [hook_attach](/efun/hooks/hook_attach) before an attachment
    is accepted. The driver computes `tier` from the request; return the
    tier you grant. A naive `return 1` allows self-hooks only. Return `0`
    to deny.

    Full tier rules and examples: [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_detach(3), hook_query(3), hook_list(3)
