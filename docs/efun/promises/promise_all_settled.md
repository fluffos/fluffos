---
title: promises / promise_all_settled
---
# promise_all_settled

### NAME

    promise_all_settled - wait for every promise and report each outcome

### SYNOPSIS

    promise promise_all_settled(mixed *promises);

### DESCRIPTION

    Returns a promise that fulfills once every input has settled -- it never
    rejects. The value is an array of one mapping per input, positionally,
    describing how that input ended:

        ([ "status": 1, "value":  v ])   fulfilled
        ([ "status": 2, "reason": r ])   rejected

    The status codes are promise_status(3)'s, so one vocabulary covers both.
    A fulfilled entry has no `"reason"` key and a rejected entry has no
    `"value"` key, so `undefinedp()` distinguishes them as reliably as
    `"status"` does.

    Use this instead of promise_all(3) when a partial failure is a result
    rather than an error -- fanning work out over many objects and reporting
    which succeeded, for instance.

    An element that is not a promise counts as already fulfilled with
    itself, so the output of an ordinary `map()` can be passed straight in.
    An empty array fulfills immediately with an empty array.

    The name follows JavaScript's `Promise.allSettled`. It is deliberately
    not `promise_settle`: that already names the driver's internal "settle
    this one promise" operation, and reusing it for "wait for all of these"
    would read as almost the opposite.

### EXAMPLE
```c
async void reindex(object *rooms) {
    mixed *results = await promise_all_settled(map(rooms, (: $1->rebuild() :)));
    int i;

    foreach (mixed r in results) {
        if (r["status"] == 2) {
            write("room " + i + " failed: " + r["reason"] + "\n");
        }
        i++;
    }
}
```

### SEE ALSO

    promise_all(3), promise_any(3), promise_race(3), promise_status(3)
