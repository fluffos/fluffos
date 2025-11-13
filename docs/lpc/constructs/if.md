---
layout: doc
title: constructs / if
---
# if

### The if else statement

LPC's if statement is identical to that provided by C. Syntax is as follows:

```c
if (expression)
    statement;
```

Alternately:

```c
if (expression) {
    statements;
}
```

Alternately:

```c
if (expression0) {
    statements;
} else {
    statements1;
}
```

Alternately:

```c
if (expression0) {
    statements0;
} else if (expression1) {
    statements1;
}
```

The number of else clauses is not explicitly limited.

---

Another favorite programming construct is the ? : operator, which also
operates identical to C. The syntax is:

    expression0 ? expression1_if_true : expression2_if_false

In some cases, ? : is an shorter way of expression constructs such as:

```c
if (expression0)
    var = expression1;
else
    var = expression2;
```

which can be equivalently translated to:

    var = expression0 ? expression1 : expression;

---

### Nullish coalescing operator (`??`)

LPC also supports `expr_left ?? expr_right`, which returns `expr_left` when it is
*defined* and evaluates/returns `expr_right` only when the left-hand side is
`undefined` (see `undefinedp()` for what counts as undefined). It short-circuits
like `||`, but falsy values such as `0`, `0.0`, `""`, or `({ })` do **not** trigger
the fallback.

```c
int timeout = cfg["timeout"] ?? 60;          // use default only if key is undefined
string name = player->query_name() ?? "guest";  // RHS runs only when call returns undefined
```

Operator precedence sits between `?:` and `||`, mirroring the common pattern
`lookup ?? fallback` without extra parentheses. Use `||` when you want to treat
all falsy values as missing; use `??` when only truly undefined values should
fall back.

### Logical assignment operators (`||=`, `&&=`, `??=`)

Short-circuit evaluation is also available in assignment form:

```c
int connected = 0;
object sock = find_connection();

connected ||= open_default();       // assign only if connected is falsy
sock &&= reconnect(sock);           // assign only if sock is truthy
config["timeout"] ??= 60;           // assign only when key is undefined
```

`||=` evaluates the right-hand side only when the left-hand side is falsy (0 or
0.0). `&&=` does the reverse, running the right-hand side only when the left is
truthy. `??=` mirrors `??`: the right-hand side runs only when the left-hand
side is truly undefined (useful for lazily populating mappings). The lvalue is
evaluated just once in every case.
