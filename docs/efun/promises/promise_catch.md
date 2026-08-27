---
title: promises / promise_catch
---
# promise_catch

### NAME
    promise_catch - attach a rejection handler

### SYNOPSIS
    promise promise_catch(promise p, function on_rejected);

### DESCRIPTION
    Attaches only a rejection handler and returns the chained promise.
    Fulfillment passes through unchanged; a rejection runs
    `on_rejected(reason)`, whose return value fulfills the chained promise.

    This is the rejection-only half of `promise_then()`, which cannot be
    spelled with `promise_then()` itself: its second argument must be a
    function whenever a third is given, so `promise_then(p, 0, f)` is a
    runtime error ("Bad argument 2 to promise_then()").

### SEE ALSO
    promise_then(3), promise_reject(3)
