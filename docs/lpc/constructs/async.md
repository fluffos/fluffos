---
title: constructs / async, await, acatch
---
# `async`, `await` and `acatch`

`async` is a function modifier. `await` and `acatch` are expressions usable
only inside an `async` function body. Together they let a function wait for a
[`promise`](../types/promise) without blocking the driver.

```c
async int transfer(string from, string to, int amount) {
    mapping acc = await fetch_account(from);
    if (acc["balance"] < amount) {
        return 0;
    }
    mixed err = acatch(await update_account(from, -amount));
    return err ? 0 : 1;
}
```

## `async`

`async` goes where any other function modifier goes, and does not change the
declared return type — `return 1;` in an `async int f()` still type-checks
against `int`. What changes is the **call site**: calling `f()` yields
`promise<int>`, never an `int`.

```c
private async string load(string path);   // prototype
private async string load(string path) { return await async_read(path); }

promise<string> p = load("/etc/motd");    // the call yields a promise
string s = await load("/etc/motd");       // ... which await unwraps
```

Rules the compiler enforces:

* A prototype and its definition must agree about `async`, and so must an
  override and the inherited declaration it overrides. Disagreeing would make
  the same call site yield a promise in one program and a plain value in
  another, with nothing at runtime to catch it.
* `async` is a **function** modifier: not variables, not `modifier_change`
  blocks (`async:`).
* **An apply cannot be `async`** — every name the driver itself calls, master
  applies (`valid_read`, `error_handler`, ...) and object applies (`create`,
  `init`, `id`, ...) alike. The driver reads the return value immediately and
  has nowhere to await; for the value-reading applies an async version is a
  security hole rather than a curiosity. Have the apply call an async
  function instead.

An `async` body runs **synchronously** until its first `await` of a pending
promise, so anything before that has already happened by the time the caller
receives the promise.

## `await`

`await expr` is a prefix expression at unary precedence: `await a + b` is
`(await a) + b`.

* `await` of a **non-promise** yields the value unchanged — no suspension.
* `await` of a **promise** always suspends, even one that has already
  settled. The function resumes from the microtask drain with the fulfilled
  value, or raises the rejection reason as an error at the await point.

Suspension has a price the compiler and runtime enforce:

* `await` is legal only **directly inside** an `async` function body — not in
  a `(: :)` functional or an anonymous function, even one written inside an
  async function.
* `await` is not allowed inside `catch(...)` or `time_expression(...)`. Both
  re-enter the interpreter on the C++ stack, which cannot be suspended; use
  `acatch` instead.
* An `await` cannot suspend while a transient reference sits on the value
  stack — `arr[i] += await p` raises a clean runtime error. Split it:
  `x = await p; arr[i] += x;`.

Resuming does **not** hand the loop a turn: the resume is delivered in the
same drain turn, which is what lets a sequential `await` loop run at full
speed. To let the driver serve players mid-computation, `await async_yield()`.

## `acatch`

`acatch(expr)` is `catch` for code that may suspend. It has the same shape
and the same value convention — `0` on success, the error value on failure —
and it is implemented as a marker on the LPC control stack rather than a
recursive interpreter call, which is why an `await` may park inside it.

```c
async void run() {
    mixed err = acatch {
        mixed data = await async_read("/missing");
        process(data);
    };
    if (err) {
        log("failed: " + err);
    }
}
```

* `acatch` is legal only inside an `async` function.
* `acatch` inside a plain `catch` is a compile error; a plain `catch` inside
  an `acatch` is fine, as long as no `await` sits inside the inner `catch`.
* `break`/`continue` may not jump out of an `acatch` region, the same rule
  `catch` has. `return` works normally.
* "Too deep recursion" is catchable, as with `catch`. The eval-cost error is
  not.

## See also

* [the `promise` type](../types/promise)
* [The async/await execution model](../../concepts/general/async) — delivery,
  scheduling, hot reload and resource limits
