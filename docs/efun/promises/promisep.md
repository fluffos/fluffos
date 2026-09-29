---
title: promises / promisep
---
# promisep

### NAME

    promisep - test whether a value is a promise

### SYNOPSIS

    int promisep(mixed arg);

### DESCRIPTION

    Returns 1 if `arg` is a promise, 0 otherwise. The `*p()` type test for
    the `promise` type, equivalent to `typeof(arg) == "promise"`.

    The argument is `mixed` on purpose: the question is only interesting for
    a value whose type is not known statically. Under `strict_types` a
    variable already declared `promise` needs no test.

### EXAMPLE
```c
void runit() {
    mixed p = promise_create();

    printf("promisep(p) = %d\n", promisep(p));    // 1
    printf("promisep(0) = %d\n", promisep(0));    // 0
    printf("promisep(\"x\") = %d\n", promisep("x"));  // 0
}
```

    A promise is never a valid *fulfilled* value -- resolving a promise with
    a promise adopts it (see promise_resolve(3)) -- so `promisep()` on the
    value an `await` yields, or on promise_result(3) of a FULFILLED promise,
    is always 0. A rejection reason is not restricted that way: any value may
    be one, including a promise, so `promisep()` on promise_result(3) of a
    rejected promise (or on what `acatch` yields) can be 1.

### SEE ALSO

    promise_create(3), promise_status(3), promise_result(3), typeof(3)
