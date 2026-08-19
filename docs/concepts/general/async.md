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
compiler's type checking. It is **parameterized by the type it will
eventually deliver**, written `promise<T>`:

```c
promise<mapping> fetch(string uid);   // return type
void handle(promise<int> p);          // parameter
promise<int> *pending = ({ });        // array of promise<int>
promise<string *> names;              // promise of an array of strings
promise anything;                     // bare `promise` == promise<mixed>
```

`promise<T> *` and `promise<T *>` are different types and compose
independently: the first is an *array of promises*, the second is *one
promise that delivers an array*. Any type may be a payload — including
`class` types — except `void` and `promise` itself: resolving a promise
with a promise adopts it, so a promise value is never itself a promise, and
a nested payload is rejected at compile time. (Spelled `promise<promise<int> >`
you get the explicit diagnostic; spelled `promise<promise<int>>` you get a
plain syntax error instead, because `>>` lexes as the shift operator.)

Assignment and argument passing compare payloads, so `promise<int>` and
`promise<string>` are incompatible while `promise` (i.e. `promise<mixed>`)
accepts either. `await` yields the payload type, so `int n = await
fetch_count();` type-checks and `string s = await fetch_count();` does not.

`typeof()` returns `"promise"` — it reports the value's kind, not its
payload. An `async` function's promise does carry the payload its function
**declared**, as a plain runtime type tag, and `sprintf("%O", p)` names it
the way the driver names any runtime type: `PROMISE<int>( fulfilled: 42 )`,
`PROMISE<array>` for a `promise<string *>`, `PROMISE<class>` for a
`promise<class point>`. A promise from `promise_create()` or
`promise_then()` declares nothing and renders bare, `PROMISE( pending )`.

Two things that tag is not. It is not finer-grained than the runtime's own
types: the compiler tells `promise<string *>` from `promise<int *>`, the
value only says "array". And it is a **declaration, not a measurement** —
exactly as trustworthy as any other declared LPC type, which is to say an
`async int f()` whose body returns a `mixed` that happens to hold a string
still produces a promise tagged `int`. Read it as documentation of intent,
the way you would read the function's signature, not as a checked property
of the settled value.

Promises compare by identity, work as mapping keys, deep-copy shallowly
(identity-preserving, like objects), and are not serialized by
`save_object()`.

Settlement delivery is never synchronous: handlers attached with
`promise_then()` and suspended `await` expressions always run from the
microtask drain, in attachment order, each with a fresh evaluation-cost
budget (the same model as `call_out(0)` callbacks). The drain is a
zero-delay gametick event, so it runs after the current execution finishes
but still within the *same* gametick -- not on a later one. A drain with more
work than fits in one turn re-posts itself to the event loop and continues
there, so later deliveries of a large batch can land after that gametick (see
"Interaction with the rest of the driver").

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

A function declared `async` always returns a **promise** for its result:
an `async T f()` is typed `promise<T>` at every call site, while `T` stays
what `return` statements inside the body are checked against. Calling one
runs its body
**synchronously until the first `await` of a promise** — any promise:
awaiting a promise always parks, even one that has already settled, and
only awaiting a non-promise value continues synchronously (see `await`
below). The caller receives the promise immediately — already fulfilled
only if the body completed without awaiting any promise, pending
otherwise.

The one exception to `async T f()` being typed `promise<T>` is `T` being an
**array of promises**. A type word carries a single promise bit, so
`promise<promise<int> *>` is unspellable; the call site is typed
`promise<mixed *>` instead — sound (it really is a promise of an array) but
weaker, so `promise<int> *a = f();` does not compile. Only the payload
loses its element type: `async int *g()` is still typed `promise<int *>`.

`return value` fulfills the promise (a returned promise is adopted). An
uncaught error inside the body rejects it — an async body behaves as if
wrapped in an implicit `catch`, so the error is reported like a caught
error and becomes the rejection reason. `throw(value)` inside an async
body rejects with `value`.

The modifier propagates through inheritance and applies on every call
path: direct local calls, `::`-qualified calls, `call_other()`, and
function pointers.

An override must **agree with the inherited function about `async`**, in
both directions, and the compiler rejects a mismatch. The inherited program
is already compiled, and every call to the function inside it was typed
against its own declaration: making an override `async` when the base is not
would hand those calls a promise where they expect the declared type (`if
(f())` becomes unconditionally true, and the first arithmetic on the result
errors at runtime far from the cause), and dropping `async` in an override
does the reverse. Changing any other part of a return type across an
override merely warns; this one is an error because nothing checks it at
runtime.

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
timer: a sequential loop of awaits runs at full speed, all of it inside the
drain, with no wall-clock delay per iteration. Only when a drain exceeds
`async drain batch` deliveries does it yield to the event loop and pick up
again a moment later — the price of the driver staying responsive under
unbounded async work, paid once per batch rather than once per `await`.

While suspended, the object remains fully live: incoming calls run
normally (there are no re-entrancy locks), and each `async` call has its
own suspension state, so concurrent invocations don't interfere. The
suspension holds real references on the object and its program.
`this_player()` across a suspension is governed by the same driver option
as `call_out()` callbacks, **`this_player in call_out`**: with the option
enabled (the default), `this_player()` at suspension time is restored on
resume; with it disabled, `this_player()` is 0 after resuming.

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

One rejection does not reach an `acatch` region: if the awaited promise is
garbage-collected while still pending (its last reference dropped, so it can
never settle), the parked frame is abandoned rather than resumed, and the
async function's own promise is rejected with `*awaited promise was
collected before settling`. There is no resume, so no `acatch` inside the
function runs -- observe that case on the returned promise instead.

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
   stack. In practice this means **any `foreach` loop** (over arrays,
   mappings, strings or buffers — the loop keeps an lvalue slot live for its
   variable for the whole body) and a `ref` argument. It is a runtime error
   rather than a compile error because it depends on what the awaited
   promise turns out to be; use an indexed `for` loop when the body needs to
   suspend. Compound assignment is *not* affected — `arr[i] += await p`,
   `s += await p` and friends evaluate the right-hand side before pinning
   the target, so they park and resume normally.
4. If the object is destructed, recompiled by `recompile_object()`, or has
   its program swapped by `replace_program()` while a function is
   suspended, the resume is abandoned and the function's promise rejects
   with a descriptive error.
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

`call_out(delay)` — a delay with **no callback** — returns a promise
fulfilled (with `0`) when the delay elapses, and rejected if the call_out
is removed (`remove_call_out()` with no argument sweeps it with the rest)
or its object destructed first. `await call_out(2)` is the non-blocking
pause idiom. The classic `call_out(fn, delay, ...)` form is unchanged and
still returns the handle; note the promise form returns no handle, so a
timer you may need to cancel individually should use the classic form.

The worker-thread I/O efuns of the async *package* follow the same
pattern: `async_read(path)`, `async_write(path, str, flag)` and
`async_getdir(path)` with the trailing callback **omitted** return a
promise fulfilled with the value the callback would have received, or
rejected with the failure value (e.g. `async_read`'s negative int) —
`string s = await async_read(path);`.

`async_info()` lists every currently suspended frame — what it is, where
it is parked, and what it awaits — the async counterpart of
`call_out_info()`. A frame stops being listed the moment its object is
destructed, even in the case where the driver cannot free it yet (its
delivery was already queued, so the queue owns it until it arrives and is
abandoned there). Such a frame can never resume, so it is not reported as
suspended and does not hold one of the `max suspended async functions`
slots against live frames.

`async`, `await`, `acatch` and `promise` are reserved words. Existing
mudlib code using them as identifiers must be renamed.

## Interaction with the rest of the driver

- Delivery rides the gametick event queue, and a drain that cannot finish in
  one go **re-posts itself to the event loop** rather than running on. Each
  turn makes at most `async drain batch` deliveries — counting work created
  *during* the turn, so a sequential chain of awaits runs at full speed
  rather than paying a loop turn per link. Between turns the driver reads
  pending network input, schedules commands and fires timers.

  The effect is that async work proceeds continuously, using time the driver
  would otherwise spend idle between gameticks, without holding the event
  loop for more than one batch. A large backlog is delivered across as many
  turns as it needs instead of in one stall, and a self-feeding
  `promise_then()` chain yields every batch rather than wedging the driver.

  Because the batch is the only bound, its size is a responsiveness setting.
  The default keeps a turn to roughly a millisecond of trivial deliveries;
  raising it far above that lets a self-feeding chain hold the driver for
  correspondingly longer, and a large enough value will stall network I/O
  outright.

  The re-post is a true yield to I/O, not a timer: it rides libevent's
  `active_later` queue, which the loop promotes at the top of its next
  iteration, *before* polling — and keeps the loop from blocking while one is
  pending. So the loop polls sockets between every turn, with no fixed delay
  to pay. A zero-delay timer would not do this (libevent dispatches a zero
  timeout immediately, from the same pass, so a self-re-posting drain never
  lets the loop poll at all — measured: a `call_out` six seconds out did not
  run in ninety seconds), and a one-millisecond timer does yield but caps
  delivery at one turn per millisecond. Measured against that 1 ms
  predecessor: 9.0M deliveries in 5s versus 635k, i.e. 14×, with login
  latency under sustained promise load still around a millisecond.

  `async_info(1)` reports the scheduler: `pending_deliveries`, the monotonic
  `drain_yields`, and the effective `drain_batch`. A rising `drain_yields`
  with a non-zero `pending_deliveries` is backpressure — async work arriving
  faster than it is delivered.

  A settle that happens *outside* gametick dispatch — which is how every
  `package/async` I/O completion arrives (`async_read`, `async_write`,
  `async_getdir`, `async_db_exec`) — arms the drain through the event loop
  rather than the tick queue, so it is delivered on the loop's next pass
  instead of waiting for the next gametick.

  A consequence worth knowing when writing tests: a batch of settlements
  larger than one turn is **not** all delivered in the same gametick, even
  though each individual settle queues immediately.

- On a Debug build, suspended coroutines and promise reactions are fully
  visible to the `check_memory()` ref-count checker.
- `defer()` handlers registered before an `await` survive the suspension
  and run when the function finally finishes, including when it is abandoned
  because its object was recompiled. Two cases where they do **not** run:

  - **The object was destructed.** Frames parked inside an object are
    abandoned by `destruct_object()` itself, and that happens partway through
    tearing the object down, where running mudlib code is not safe — so their
    `defer()` handlers are discarded rather than run. Even reached by the
    other route (a delivery arriving for a destructed owner, which runs at a
    safe point), most handlers still could not run: a `defer()` handler is a
    function pointer, and the driver refuses to call one whose owner is
    destructed ("Owner of function pointer is destructed"), which is every
    handler written the natural way — `defer((: release_lock, key :))` inside
    the object's own method. Do not rely on `defer()` for cleanup that must
    survive the owner being destructed mid-`await`; put that cleanup in an
    object that outlives the operation.
  - **The awaited promise was garbage collected.** That happens on a
    deallocation path where running mudlib code is unsafe, so those handlers
    are dropped.
- Driver shutdown discards queued deliveries without running them, like
  pending `call_out()`s.
