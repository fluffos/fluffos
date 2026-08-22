---
title: types / promise
---
# promise

A `promise` is a first-class value standing for a result that is not
available yet. It is in one of three states — **pending**, **fulfilled**
with a value, or **rejected** with a reason — and it settles at most once.

```c
promise p = promise_create();   // pending
promise_resolve(p, 42);         // fulfilled with 42
promise_status(p);              // 1
promise_result(p);              // 42
```

Promises are produced by [`async` functions](../constructs/async), by the
promise efuns, and by the promise forms of the callback efuns
(`call_out(delay)`, `async_read(path)`, ...). They are consumed with
`await`, or with `promise_then()` / `promise_catch()` handlers.

## Value semantics

Like objects and functions, a promise is a **reference with identity**, not
a container:

| operation | behaviour |
|---|---|
| `p == q` | true only for the *same* promise |
| mapping key | keyed by identity |
| `copy(p)` | the same promise, not a copy |
| `save_object()` | not serialisable; saves as `0` |
| `typeof(p)` | `"promise"` |
| `sprintf("%O", p)` | `PROMISE( fulfilled: 42 )` |

`promisep(x)` tests the type. A promise is reference-counted: when the last
reference goes away it is reclaimed, and anything awaiting it simply never
resumes — the same as an abandoned callback.

## Typed promises: `promise<T>`

The type may name its payload, which is what a call site of an `async`
function is given:

```c
async int fetch();              // returns promise<int>
promise<int> p = fetch();
int n = await p;                // await yields the payload type
```

`promise<int> *` is an *array of* promises; `promise<int *>` is a promise of
an array — `*` keeps its ordinary meaning and the two compose. A promise of
a promise is not representable, matching the runtime, where resolving a
promise with a promise **adopts** it rather than nesting.

Bare `promise` is `promise<mixed>`: it accepts any payload, which is what
`promise_create()` and the promise-form efuns return.

The payload type is a **declaration, not a measurement**. An `async int f()`
whose body returns a `mixed` holding a string still produces a promise
tagged `int`; nothing re-checks it at settle time.

## Errors

* Settling a promise that is already settled is an error — including one
  whose fate is already committed to a pending adoption.
* `promise_result()` on a pending promise is an error.
* A rejection nothing ever observes is reported to the debug log when the
  promise is deallocated, naming where it was rejected.

## See also

* [`async` functions, `await` and `acatch`](../constructs/async) — the syntax
* [The async/await execution model](../../concepts/general/async) — scheduling,
  suspension and resource limits
* `promise_create`, `promise_resolve`, `promise_reject`, `promise_then`,
  `promise_catch`, `promise_status`, `promise_result`, `async_yield`,
  `async_info`
