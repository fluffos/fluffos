---
title: constructs / foreach
---

# foreach

`foreach` walks an array, mapping, string, or buffer. Prefer it over an
indexed `for` when you do not need the index.

```c
foreach (type variable in aggregate)
    statement;
```

The body may be a block. `break` and `continue` work as in `for` / `while`.

## Arrays

```c
string *names = ({ "ada", "bek", "cy" });

foreach (string name in names) {
    write(name + "\n");
}
```

Declare the loop variable in the `foreach` header, or reuse an existing
local:

```c
string name;
foreach (name in names)
    write(name + "\n");
```

## Mappings

One variable receives **values**. Two variables receive **key then value**.
The key must not be a `ref`.

```c
mapping scores = ([ "ada": 10, "bek": 7 ]);

foreach (string name, int score in scores) {
    write(name + " => " + score + "\n");
}

foreach (int score in scores) {
    write(score + "\n");
}
```

## Strings and buffers

A string iterates characters (codepoint-oriented; a multi-codepoint
grapheme cluster can error if you treat it as a single indexed char — see
[ref](ref)). A buffer iterates bytes as `int` values 0–255.

```c
foreach (int b in buf) {
    write(sprintf("%02x ", b));
}
```

## Mutating in place (`ref` / `&`)

`ref` (or `&`) on the loop variable writes back into **arrays**, **buffers**,
and **mapping values**. It does not rewrite the source string variable.

```c
int *nums = ({ 1, 2, 3 });
foreach (int ref n in nums)
    n *= 2;
// nums is ({ 2, 4, 6 })
```

Details and restrictions: [ref](ref).

## `await` inside `foreach`

An `async` function may `await` inside a `foreach` over a **local**
aggregate. See [async](async) and the
[async concept guide](../../concepts/general/async).
