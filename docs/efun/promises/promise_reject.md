---
title: promises / promise_reject
---
# promise_reject

### NAME
    promise_reject - reject a pending promise

### SYNOPSIS
    void promise_reject(promise p);
    void promise_reject(promise p, mixed reason);

### DESCRIPTION
    Rejects the pending promise `p` with `reason` (0 if omitted). Rejection
    handlers attached with promise_then(3)/promise_catch(3) run from the
    microtask drain — never synchronously from this call, but still within
    the same gametick. An `await` suspended on `p` raises `reason` as an
    error at the await point (catchable with `acatch`).

    It is an error to settle a promise that is already settled — including
    one whose fate is already committed to a pending adoption (see
    promise_resolve(3)).

    A rejected promise whose rejection is never observed (no handler
    attached, result never read) is reported to the debug log when it is
    deallocated. The report names where the promise was REJECTED --
    `Unhandled promise rejection (rejected by /obj/thing at
    /obj/thing.lpc:42): reason` -- because deallocation can be arbitrarily
    far from the rejection, and the reason on its own is often not enough to
    identify it (`(int) 0` is an ordinary rejection value for the promise
    forms of async_read(3) and friends).

### SEE ALSO
    promise_resolve(3), promise_catch(3), promise_then(3)
