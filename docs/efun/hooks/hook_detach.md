---
title: hooks / hook_detach
---
# hook_detach

### NAME
    hook_detach - remove a previously attached hook

### SYNOPSIS
    void hook_detach(int id);

### DESCRIPTION
    Detaches the attachment identified by `id` (from
    [hook_attach](/efun/hooks/hook_attach)). A missing or already-detached
    id is a no-op. Only the owner or the master may detach. Ownership also
    ends when the owning object is destructed; the driver re-asks
    `valid_hook()` after the master is recompiled and drops attachments
    that are no longer granted.

    See [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_query(3), hook_list(3)
