---
title: constructs / ref
---
# ref (pass-by-reference)

The `ref` keyword enables pass-by-reference semantics for function parameters.
Normally, LPC passes arguments by value — changes to a parameter inside a function
do not affect the caller's variable. With `ref`, the function operates directly on
the caller's variable.

### Declaring ref parameters

Use `ref` after the type in a function parameter declaration:

```c
void increment(int ref value) {
    value++;
}

void append(mixed ref *arr, mixed item) {
    arr += ({ item });
}
```

### Calling with ref arguments

At the call site, you must explicitly mark the argument with `ref` or `&`:

```c
int x = 10;
increment(ref x);   // x is now 11
increment(& x);     // x is now 12 — & is syntactic sugar for ref

mixed *items = ({ "a", "b" });
append(ref items, "c");   // items is now ({ "a", "b", "c" })
append(& items, "d");     // items is now ({ "a", "b", "c", "d" })
```

The `&` operator can be used anywhere `ref` is accepted. Both forms are
identical in behavior.

### Ref in foreach

The `ref` keyword (or `&`) can also be used in `foreach` loops to modify
array elements in place:

```c
int *nums = ({ 1, 2, 3 });

foreach (int ref n in nums) {
    n *= 2;
}
// nums is now ({ 2, 4, 6 })

// Equivalent using &
foreach (int & n in nums) {
    n *= 2;
}
// nums is now ({ 4, 8, 12 })
```

Note: `ref` in `foreach` only mutates **arrays** (and mapping values) in
place. Iterating a string with a `ref` loop variable follows the same rules
as an `s[i]` char lvalue: characters that index as a single codepoint work
(reads and assignments through the loop variable succeed), while a wider
grapheme cluster — such as a flag emoji — raises the same catchable
`"Indexed character is multi-codepoint"` error when the loop reaches it.
Writes through the loop variable do not propagate to the source variable —
strings are passed into `foreach` by value. To mutate a string, assign
through an index instead (`s[i] = c`, `s[i] += 1`).

### Restrictions

- `ref` can only be used in function parameter declarations, function call
  arguments, and `foreach` variable declarations.
- The argument passed by `ref` must be an lvalue (a variable, not an expression
  like `x + 1`).
- References to array/string ranges are not allowed.
- `ref` must be used at both the declaration and the call site — it is
  intentionally explicit for safety.

### See also

`types/general` — value types vs reference types
