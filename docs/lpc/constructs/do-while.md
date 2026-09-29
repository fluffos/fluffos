---
title: constructs / do-while
---

# do-while

LPC's `do` / `while` loop matches C. The body always runs once; the test
runs after each iteration.

```c
do
    statement;
while (expression);
```

The body may be a block. `break` leaves the loop; `continue` jumps to the
condition.

```c
int n = 0;
do {
    n++;
} while (n < 3);
// n is 3
```

Use `while` when the body should not run on a false first test. Use
[foreach](foreach) when you are walking an aggregate.
