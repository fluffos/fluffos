---
title: promises / promise_create
---
# promise_create

### NAME
    promise_create - create a new pending promise

### SYNOPSIS
    mixed promise_create();

### DESCRIPTION
    Returns a new promise in the pending state. A promise is a first-class
    LPC value (`typeof` returns `"promise"`) holding the eventual result of
    an asynchronous operation. It settles exactly once, either fulfilled via
    promise_resolve(3) or rejected via promise_reject(3); reactions attached
    with promise_then(3) run on a later gametick after settlement.

    Promises compare by identity (`p == q` is true only for the same
    promise) and may be used as mapping keys. They are not saved by
    save_object(3).

### EXAMPLES
    ```c
    mixed delay(int seconds) {
        mixed p = promise_create();
        call_out( (: promise_resolve, p, 1 :), seconds);
        return p;
    }
    ```

### SEE ALSO
    promise_resolve(3), promise_reject(3), promise_then(3),
    promise_status(3), promise_result(3)
