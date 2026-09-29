---
title: promises / async_yield
---
# async_yield

### NAME
    async_yield - give the event loop a turn from inside an async function

### SYNOPSIS
    promise async_yield();

### DESCRIPTION
    Returns a promise fulfilled with 0 on the next pass of the driver's
    event loop -- after the driver has read pending network input, queued
    commands and fired due timers.

    `await async_yield();` is therefore a cooperative preemption point: it
    parks the async function at a suspension point that is safe by
    construction and resumes it on a later microtask-drain turn, with the
    loop having run in between. Use it to break a long computation into
    pieces the driver can serve players around:

    ```c
    async void reindex(mixed *rows) {
        int i;

        foreach (mixed row in rows) {
            index(row);
            if (++i % 500 == 0) {
                await async_yield();
            }
        }
    }
    ```

    Awaiting an ordinary settled promise does NOT do this. It parks, but the
    resume is re-queued into the *same* drain turn, which is deliberate: it
    is what lets a sequential `await` loop run at full speed instead of
    paying an event-loop turn per iteration. `await call_out(0)` does not do
    it either -- a `call_out(0)` runs on the same gametick, and
    `call_out(0) nest level` will refuse one used as a yield inside a loop.
    `await call_out(1)` does reach the loop, but costs a whole gametick.

    The promise is an ordinary promise: it can be stored, chained with
    promise_then(3), or awaited from more than one place. Two calls made
    before the loop next runs return two distinct promises that settle at
    the same moment, sharing a single wake-up.

    On the WebAssembly build the loop is the page's: the promise settles on
    the host's next call into the driver, which is the same guarantee (the
    page has had its turn) at whatever rate the host drives it.

    async_yield() does not reset the evaluation budget. A delivery is armed
    with a whole `maximum evaluation cost` when it starts, and a resumed
    frame is a new delivery, so a function that yields periodically is
    metered per resumption rather than as one long run.

### RETURN VALUE
    A pending promise, fulfilled with 0 once the event loop has run.

### SEE ALSO
    async_info(3), promise_then(3), call_out(3)

### NOTE
    Unrelated to the async_read(3)/async_write(3) family from the async
    package, which offloads file and database I/O to worker threads.
