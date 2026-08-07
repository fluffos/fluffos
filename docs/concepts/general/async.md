---
title: Async / await
---

# Async / await

FluffOS supports native coroutine-style asynchronous programming
(issue #1319, phase 1): a first-class **promise** value type, an
**`async`** function modifier, an **`await`** expression that suspends the
function without blocking the driver, and **`acatch`**, the async-aware
form of `catch`. This page is the execution-model specification.

## Promises

A promise is a first-class LPC value holding the eventual result of an
asynchronous operation. It is created pending and settles exactly once —
**fulfilled** with a value or **rejected** with a reason. See
`promise_create()`, `promise_resolve()`, `promise_reject()`,
`promise_then()`, `promise_catch()`, `promise_status()`,
`promise_result()`, and `async_info()`.

`promise` is a declared type, like `mapping` or `buffer` — usable for
variables, parameters, return types and arrays, and enforced by the
compiler's type checking:

```c
promise fetch(string uid);        // return type
void handle(promise p);           // parameter
promise *pending = ({ });         // array of promises
```

`typeof()` returns `"promise"`. Promises compare by identity, work as
mapping keys, deep-copy shallowly (identity-preserving, like objects),
and are not serialized by `save_object()`. Calling an `async` function
yields a value of this type, so `promise p = some_async_fn();`
type-checks.

Settlement delivery is never synchronous: handlers attached with
`promise_then()` and suspended `await` expressions always run from a later
gametick, in attachment order, each with a fresh evaluation-cost budget
(the same model as `call_out(0)` callbacks).

Fulfilling a promise with another promise **adopts** it (flattening): the
outer promise stays pending until the inner one settles, then settles the
same way. A rejection that is never observed — no rejection handler, the
result never read — is reported to the debug log when the promise is
deallocated.

## `async` functions

```c
async int transfer(string from, string to, int amount) {
    mapping acc = await fetch_account(from);
    if (acc["balance"] < amount) return 0;
    await update_account(from, -amount);
    await update_account(to, amount);
    return 1;
}
```

A function declared `async` always returns a **promise** for its result,
whatever its declared return type says (the declared type is what `return`
statements inside the body are checked against). Calling one runs its body
**synchronously until the first `await` of a pending promise**; the caller
receives the promise immediately — already fulfilled if the body finished
without suspending, pending otherwise.

`return value` fulfills the promise (a returned promise is adopted). An
uncaught error inside the body rejects it — an async body behaves as if
wrapped in an implicit `catch`, so the error is reported like a caught
error and becomes the rejection reason. `throw(value)` inside an async
body rejects with `value`.

The modifier propagates through inheritance and applies on every call
path: direct local calls, `::`-qualified calls, `call_other()`, and
function pointers.

## `await`

`await expr` is a unary prefix expression (`await a + b` parses as
`(await a) + b`):

- a **non-promise** operand passes through unchanged — awaiting a plain
  value is a no-op, not a scheduling point;
- a **promise always suspends**, even one that has already settled. The
  driver keeps serving everything else; the function resumes from the
  microtask drain **with a fresh evaluation-cost budget**, receiving the
  value, or with the rejection raised at the await point.

That last rule is what makes `await` a scheduling primitive: because every
awaited promise costs a suspension and buys a fresh budget, a loop of
awaits breaks long work into separately-metered pieces instead of burning
one budget until "too long evaluation". Resuming is a microtask, not a
timer, so this costs no wall-clock delay.

While suspended, the object remains fully live: incoming calls run
normally (there are no re-entrancy locks), and each `async` call has its
own suspension state, so concurrent invocations don't interfere. The
suspension holds real references on the object and its program;
`this_player()` at suspension time is restored on resume under the same
driver option that governs `call_out()`.

## `acatch`

`await` is **not allowed inside `catch`** (see restrictions below).
`acatch(expr)` / `acatch { ... }` is the async-aware replacement: the same
value convention as `catch` — `0` on success, the error value on failure —
but `await` may suspend inside the protected region.

```c
async void pay(object who, int amount) {
    mixed err = acatch(await bank_transfer(who, amount));
    if (err) write("Transfer failed: " + err + "\n");
}
```

An error raised inside the region — synchronously, or as the rejection of
an awaited promise, even one that settles long after the function
suspended — resumes execution right after the `acatch` with the error as
its value. `acatch` is only legal directly inside an `async` function
body, and not inside a plain `catch`.

## Restrictions

These rules keep suspension sound (the VM parks exactly one frame; no C++
stack may be pinned across a suspension). Each violation is a clean
compile-time or runtime error, never silent misbehavior:

1. `await` and `acatch` are only legal **directly inside an `async`
   function body** — not in `(: :)` functionals or anonymous functions
   (those run in their own frames), and not at top level.
2. `await` is not allowed inside `catch(...)` or `time_expression(...)`
   (their implementation recurses the C++ stack). Use `acatch`.
3. An `await` cannot suspend while a transient reference sits on the value
   stack. This covers `arr[i] += await p` and **any `foreach` loop** (over
   arrays, mappings, strings or buffers — the loop keeps an lvalue slot
   live for its variable), which is a runtime error rather than a compile
   error because it depends on what the awaited promise turns out to be.
   Await into a plain variable first (`x = await p; arr[i] += x;`), and use
   an indexed `for` loop when the body needs to suspend.
4. If the object is destructed, or recompiled by `recompile_object()`,
   while a function is suspended, the resume is abandoned and the
   function's promise rejects with a descriptive error.
5. `break`/`continue` may not jump out of an `acatch` region (same rule as
   `catch`); `return` works normally.
6. An eval-cost ("too long evaluation") error can never be swallowed by an
   async body or `acatch`, matching `catch`.

## Resource limits

Each suspended function holds a heap copy of its frame, so the number of
concurrently suspended async functions is bounded by the runtime config
option **`max suspended async functions`** (default 10000; 0 disables the
limit). An `await` that would exceed it raises a clean error at the await
point instead of suspending — catchable with `acatch`, and otherwise
simply rejecting that function's promise. This is a runaway-exhaustion
guard: a loop spawning async calls that never settle hits a bounded error
rather than consuming memory without limit.

`await_callout(handle)` turns a pending `call_out` into a promise,
fulfilled with the callback's return value when it fires and rejected if
the call_out is removed first — so `await await_callout(call_out(f, 2))`
is a non-blocking pause that resumes with the callback's result.

`async_info()` lists every currently suspended frame — what it is, where
it is parked, and what it awaits — the async counterpart of
`call_out_info()`.

`async`, `await`, `acatch` and `promise` are reserved words. Existing
mudlib code using them as identifiers must be renamed.

## Interaction with the rest of the driver

- Delivery rides the gametick event queue; queued deliveries are bounded
  per tick (leftovers roll to the next tick), so a self-feeding
  `promise_then()` chain cannot wedge the driver.
- On a Debug build, suspended coroutines and promise reactions are fully
  visible to the `check_memory()` ref-count checker.
- `defer()` handlers registered before an `await` survive the suspension
  and run when the function finally finishes, including when it is
  abandoned because its object was destructed or recompiled. The one
  exception is abandonment caused by the awaited promise being garbage
  collected: that happens on a deallocation path where running mudlib code
  is unsafe, so those handlers are dropped.
- Driver shutdown discards queued deliveries without running them, like
  pending `call_out()`s.
