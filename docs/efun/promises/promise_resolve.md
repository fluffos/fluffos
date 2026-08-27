---
title: promises / promise_resolve
---
# promise_resolve

### NAME
    promise_resolve - fulfill a pending promise

### SYNOPSIS
    void promise_resolve(promise p);
    void promise_resolve(promise p, mixed value);

### DESCRIPTION
    Fulfills the pending promise `p` with `value` (0 if omitted). Reactions
    attached with promise_then(3) and suspended `await` expressions resume
    from the microtask drain — never synchronously from this call, but
    still within the same gametick.

    If `value` is itself a promise, `p` adopts its eventual state instead
    of fulfilling immediately (flattening): `p` stays pending until `value`
    settles, then settles the same way. Resolving a promise with itself is
    an error.

    It is an error to settle a promise that is already settled — including
    one whose fate is already committed to a pending adoption: after
    `promise_resolve(p, q)` with `q` still pending, a second
    `promise_resolve(p, ...)` or `promise_reject(p, ...)` errors even
    though `p` itself has not settled yet.

    It is likewise an error to settle a promise returned by an `async`
    function — that promise belongs to the function body. See
    promise_reject(3).

### SEE ALSO
    promise_create(3), promise_reject(3), promise_then(3)
