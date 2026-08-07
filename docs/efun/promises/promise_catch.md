---
title: promises / promise_catch
---
# promise_catch

### NAME
    promise_catch - attach a rejection handler

### SYNOPSIS
    mixed promise_catch(mixed p, function on_rejected);

### DESCRIPTION
    Shorthand for `promise_then(p, 0, on_rejected)`: attaches only a
    rejection handler and returns the chained promise. Fulfillment passes
    through unchanged; a rejection runs `on_rejected(reason)`, whose return
    value fulfills the chained promise.

### SEE ALSO
    promise_then(3), promise_reject(3)
