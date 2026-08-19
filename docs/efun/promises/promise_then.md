---
title: promises / promise_then
---
# promise_then

### NAME
    promise_then - attach settlement handlers, chaining a new promise

### SYNOPSIS
    promise promise_then(promise p);
    promise promise_then(promise p, function on_fulfilled);
    promise promise_then(promise p, function on_fulfilled, function on_rejected);

### DESCRIPTION
    Attaches handlers to `p` and returns a new promise for the handler's
    result. Handlers run from the microtask drain when `p` settles — or, if
    `p` is already settled, from the drain later in the current gametick.
    Delivery is never synchronous with this call:

    - fulfilled: `on_fulfilled(result)` runs; its return value fulfills the
      chained promise (a returned promise is adopted). If omitted, the
      fulfillment passes through unchanged.
    - rejected: `on_rejected(reason)` runs; its return value FULFILLS the
      chained promise (the rejection is handled). If omitted, the rejection
      passes through to the chained promise.

    An error inside a handler rejects the chained promise with the error
    text (reported like a caught error).

    Like call_out(0) callbacks, handlers are governed by the "this_player
    in call_out" driver option: with the option enabled (the default),
    this_player() at attach time is restored during the handler; with it
    disabled, handlers run without a command context.

### EXAMPLES
    ```c
    promise_then(fetch_account(uid),
        function(mapping acc) { return acc["balance"]; },
        function(mixed err)   { log_file("bank", err + "\n"); return -1; });
    ```

### SEE ALSO
    promise_catch(3), promise_create(3), promise_resolve(3),
    promise_reject(3)
