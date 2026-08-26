---
title: promises / promise_cancel
---
# promise_cancel

### NAME

    promise_cancel - ask an async function to give up

### SYNOPSIS

    int promise_cancel(promise p);

### DESCRIPTION

    Requests cancellation of the `async` function body that owns `p`. The
    body's **next `await` raises** a catchable error whose value is the
    string `"*async function cancelled"`.

    Cancellation is **cooperative, not preemptive**. A body part-way through
    a stretch of straight-line code finishes that stretch first; a body that
    never awaits again runs to completion and its cancellation is never
    delivered at all. Nothing is torn down mid-expression.

    The raise behaves like any other rejection arriving at that `await`: it
    unwinds through enclosing `acatch` regions, runs `defer()` handlers in
    order, and — if nothing catches it — rejects `p` with the same reason.

    A body parked on a promise that will never settle is still cancelled
    promptly: it is detached from that promise and its rejection is
    scheduled directly, so cancellation is never hostage to the thing being
    awaited.

### RETURN VALUE

    1 if a cancellation was armed, 0 if there was nothing left to cancel —
    the body already finished, or it returned a still-pending promise and is
    gone. A body racing its canceller to completion is a normal outcome, not
    an error.

### THE REQUEST IS CONSUMED

    The raise **clears** the request. A body that catches its own
    cancellation may go on to `await` cleanup work and even return a value,
    in which case `p` *fulfills* normally:

```c
async int worker() {
    mixed err = acatch(await slow_thing());

    if (err) {
        await write_log("gave up");   // does NOT re-raise
        return 0;
    }
    return 1;
}
```

    So cancellation is a request a body may decline, not a verdict. Cancel
    again if you mean it again. The alternative — a sticky flag — would make
    every cleanup `await` throw, leaving a body no way to release what it
    holds.

### ERRORS

    It is an error to cancel a promise that is not an `async` function's.
    Only a body has a "next await" for the cancellation to arrive at:

    * a `promise_create()` promise is already settleable by whoever owns it;
    * an async_read(3)/async_write(3)/async_getdir(3) promise cannot stop the
      worker thread that is already doing the I/O;
    * a `call_out(delay)` promise is not cancellable — use the classic
      `call_out()` form and remove_call_out(3);
    * rejecting a promise_then(3) chain link cannot stop its upstream.

    To stop *waiting* for any of those without stopping the work, race them
    against a timer — see promise_race(3).

### NOTES

    Cancellation does **not** propagate. If a cancelled body was awaiting
    another async function's promise, that inner body keeps running: its
    promise is first-class and may have other awaiters, `then` handlers, or
    simply be stored somewhere, and rejecting it would settle it for all of
    them. A body that wants the inner work stopped too can catch its own
    cancellation and cancel the inner promise it holds.

### SEE ALSO

    promise_race(3), promise_status(3), promise_reject(3), async_info(3)
