---
title: promises / promise_status
---
# promise_status

### NAME
    promise_status - query a promise's state

### SYNOPSIS
    int promise_status(promise p);

### DESCRIPTION
    Returns the current state of promise `p`:

        0   pending
        1   fulfilled
        2   rejected
        3   cancelled

    Cancelled is a negative settlement -- `await`, `promise_then` /
    `promise_catch`, and the fail-fast combinators treat it like a
    rejection -- but it is not a fault. `acatch` still yields the reason
    string (`"*async function cancelled"`); use this code, not that
    string, to tell a cancellation from a rejection. A mudlib can forge
    the string with `throw()`.

### SEE ALSO
    promise_result(3), promise_cancel(3), promise_create(3)
