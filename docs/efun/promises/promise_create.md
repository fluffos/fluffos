---
title: promises / promise_create
---
# promise_create

### NAME
    promise_create - create a new pending promise

### SYNOPSIS
    promise promise_create();

### DESCRIPTION
    Returns a new promise in the pending state, with no declared payload
    type -- it is a `promise<mixed>`, assignable to any `promise<T>`
    variable. (Only an `async` function's own promise carries a declared
    payload; see the async/await concept page.)

    A promise is a first-class
    LPC value (`typeof` returns `"promise"`) holding the eventual result of
    an asynchronous operation. It settles exactly once, either fulfilled via
    promise_resolve(3) or rejected via promise_reject(3); reactions attached
    with promise_then(3) run from the microtask drain after settlement --
    never synchronously, but still within the same gametick.

    Promises compare by identity (`p == q` is true only for the same
    promise) and may be used as mapping keys. They are not saved by
    save_object(3).

### EXAMPLES
    ```c
    promise delay(int seconds) {
        promise p = promise_create();
        call_out( (: promise_resolve, p, 1 :), seconds);
        return p;
    }
    ```

### SEE ALSO
    promise_resolve(3), promise_reject(3), promise_then(3),
    promise_status(3), promise_result(3)
