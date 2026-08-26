---
title: promises / promise_race
---
# promise_race

### NAME

    promise_race - settle as the first input settles, either way

### SYNOPSIS

    promise promise_race(mixed *promises);

### DESCRIPTION

    Returns a promise that settles exactly as the **first input to settle**
    does -- fulfilled with its value, or rejected with its reason. A
    rejection wins a race; that is the difference from promise_any(3),
    which ignores rejections until every input has failed.

    The losing inputs are not cancelled. They keep running and their results
    are discarded, so a race is a way to stop *waiting*, not a way to stop
    work.

    An element that is not a promise counts as already fulfilled with
    itself, which therefore wins the race outright.

    An **empty array is an error**, not a promise that never settles. This
    deliberately departs from the JavaScript behaviour: here a permanently
    pending promise that something awaits is a parked frame holding its
    object, its program and one `max suspended async functions` slot for the
    life of the driver, so the mistake is refused where it is made.

### EXAMPLE
```c
// bound a wait without touching the operation's own promise
async mixed with_timeout(promise p, int secs) {
    return await promise_race(({ p, timeout_promise(secs) }));
}
```

### SEE ALSO

    promise_any(3), promise_all(3), promise_all_settled(3), call_out(3)
