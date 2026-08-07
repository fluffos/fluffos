---
title: promises / promise_resolve
---
# promise_resolve

### NAME
    promise_resolve - fulfill a pending promise

### SYNOPSIS
    void promise_resolve(mixed p);
    void promise_resolve(mixed p, mixed value);

### DESCRIPTION
    Fulfills the pending promise `p` with `value` (0 if omitted). Reactions
    attached with promise_then(3) and suspended `await` expressions resume
    on a later gametick — never synchronously from this call.

    If `value` is itself a promise, `p` adopts its eventual state instead
    of fulfilling immediately (flattening): `p` stays pending until `value`
    settles, then settles the same way. Resolving a promise with itself is
    an error.

    It is an error to settle a promise that is already settled.

### SEE ALSO
    promise_create(3), promise_reject(3), promise_then(3)
