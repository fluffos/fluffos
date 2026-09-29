---
title: promises / promise_all
---
# promise_all

### NAME

    promise_all - wait for every promise, failing on the first rejection

### SYNOPSIS

    promise promise_all(mixed *promises);

### DESCRIPTION

    Returns a promise that fulfills with an array of every input's value,
    **positionally**: entry `i` of the result is the value of `promises[i]`,
    whatever order they settled in.

    If any input rejects, the returned promise rejects immediately with that
    reason and the remaining inputs are ignored (they keep running -- there
    is no cancellation implied). Only the FIRST rejection is reported; use
    promise_all_settled(3) to see every outcome.

    An element that is not a promise counts as already fulfilled with
    itself, so the output of an ordinary `map()` can be passed straight in
    without wrapping.

    An empty array fulfills immediately with an empty array.

### EXAMPLE
```c
async void load(string *names) {
    mixed *rows = await promise_all(map(names, (: fetch($1) :)));

    // rows[i] corresponds to names[i]
    write("loaded " + sizeof(rows) + "\n");
}
```

### SEE ALSO

    promise_any(3), promise_race(3), promise_all_settled(3), promise_then(3)
