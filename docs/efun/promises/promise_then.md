---
title: promises / promise_then
---
# promise_then

### NAME
    promise_then - attach settlement handlers, chaining a new promise

### SYNOPSIS
    mixed promise_then(mixed p);
    mixed promise_then(mixed p, function on_fulfilled);
    mixed promise_then(mixed p, function on_fulfilled, function on_rejected);

### DESCRIPTION
    Attaches handlers to `p` and returns a new promise for the handler's
    result. When `p` settles (or on the next gametick, if already settled):

    - fulfilled: `on_fulfilled(result)` runs; its return value fulfills the
      chained promise (a returned promise is adopted). If omitted, the
      fulfillment passes through unchanged.
    - rejected: `on_rejected(reason)` runs; its return value FULFILLS the
      chained promise (the rejection is handled). If omitted, the rejection
      passes through to the chained promise.

    An error inside a handler rejects the chained promise with the error
    text (reported like a caught error).

    Like call_out(0), handlers run without a command context by default;
    with the "this player in call_out" driver option, this_player() at
    attach time is restored during the handler.

### EXAMPLES
    ```c
    promise_then(fetch_account(uid),
        function(mapping acc) { return acc["balance"]; },
        function(mixed err)   { log_file("bank", err + "\n"); return -1; });
    ```

### SEE ALSO
    promise_catch(3), promise_create(3), promise_resolve(3),
    promise_reject(3)
