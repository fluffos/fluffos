---
title: promises / promise_reject
---
# promise_reject

### NAME
    promise_reject - reject a pending promise

### SYNOPSIS
    void promise_reject(mixed p);
    void promise_reject(mixed p, mixed reason);

### DESCRIPTION
    Rejects the pending promise `p` with `reason` (0 if omitted). Rejection
    handlers attached with promise_then(3)/promise_catch(3) run on a later
    gametick; an `await` suspended on `p` raises `reason` as an error at
    the await point (catchable with `acatch`).

    It is an error to settle a promise that is already settled.

    A rejected promise whose rejection is never observed (no handler
    attached, result never read) is reported to the debug log when it is
    deallocated.

### SEE ALSO
    promise_resolve(3), promise_catch(3), promise_then(3)
