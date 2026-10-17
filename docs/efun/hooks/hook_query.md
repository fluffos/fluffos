---
title: hooks / hook_query
---
# hook_query

### NAME
    hook_query - inspect a live hook attachment

### SYNOPSIS
    mapping hook_query(int id);

### DESCRIPTION
    Returns a mapping describing the attachment `id`, or an empty mapping
    if the id is unknown. Keys include `"id"`, `"point"`, `"owner"`,
    `"label"`, `"flags"`, `"priority"`, `"expires"`, `"events"`,
    `"errors"`, `"skipped"`, `"dropped"`, `"eval_us"`, and `"last_fired"`.
    Count/time attachments may also include `"rows"`.

    See [Hooks](/concepts/general/hooks).

### SEE ALSO
    hook_attach(3), hook_list(3), hook_detach(3)
