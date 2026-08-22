---
title: promises / promise_result
---
# promise_result

### NAME
    promise_result - read a settled promise's value or rejection reason

### SYNOPSIS
    mixed promise_result(promise p);

### DESCRIPTION
    Returns the fulfillment value or rejection reason of the settled
    promise `p`. It is an error to call this on a pending promise (check
    promise_status(3) first, or use promise_then(3) / `await` instead).

    Reading a rejected promise's result counts as observing the rejection:
    the unhandled-rejection report is suppressed for it.

### SEE ALSO
    promise_status(3), promise_then(3)
