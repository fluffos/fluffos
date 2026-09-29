---
title: promises / promise_any
---
# promise_any

### NAME

    promise_any - take the first promise that succeeds

### SYNOPSIS

    promise promise_any(mixed *promises);

### DESCRIPTION

    Returns a promise that fulfills with the value of the first input to
    **fulfill**. Rejections are tolerated: they are collected rather than
    propagated, so one failing source does not spoil the result.

    Only if EVERY input rejects does the returned promise reject, with an
    array of the reasons, positionally (entry `i` is `promises[i]`'s reason).

    Contrast promise_race(3), which is settled by the first input to settle
    either way -- a rejection there wins the race.

    An element that is not a promise counts as already fulfilled with
    itself. An empty array rejects: there is nothing that could ever
    satisfy it.

### EXAMPLE
```c
async string first_reachable(string *mirrors) {
    // whichever mirror answers first; failures are ignored unless all fail
    return await promise_any(map(mirrors, (: fetch($1) :)));
}
```

### SEE ALSO

    promise_all(3), promise_race(3), promise_all_settled(3)
