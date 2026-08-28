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

### SEE ALSO
    promise_result(3), promise_create(3)
