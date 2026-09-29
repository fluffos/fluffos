---
title: constructs / modifiers
---

# Modifiers

Type modifiers attach to functions and (most of them) to variables. More
than one may appear. The basic type is still `void`, `int`, `float`,
`string`, `object`, `mapping`, `function`, `mixed`, `buffer`, a `class`
name, or an array of those (`int *`, `mixed **`, …).

## Visibility

| Modifier | Meaning |
|---|---|
| `public` | Callable / visible from other objects (the default you should write). |
| `protected` | Visible to this program and its inheritors, not via `call_other` from outside. |
| `private` | Visible only in this program file. |
| `nomask` | Inheritors may not override this function (or hide this variable). |

`static` is the historic spelling: it means `protected` + `nosave` unless
the driver was built with sensible-modifier aliases. Prefer `protected`
and `nosave` in new code.

## Persistence

| Modifier | Meaning |
|---|---|
| `nosave` | Not written by `save_object()` / not restored. Use on runtime caches, sockets, and UIDs. |

## Functions

| Modifier | Meaning |
|---|---|
| `varargs` | Callers may pass fewer arguments than declared; omitted trailing args are `0`. The last declared parameter may be a catch-all with `...`. |
| `async` | The function body may `await`. The **call site** yields a `promise`, not the declared return type. See [async](async). |

```c
public nomask varargs void log(string msg, mixed extra) {
    if (undefinedp(extra))
        write(msg + "\n");
    else
        write(msg + " " + extra + "\n");
}

private nosave object conn;
```

## `async` is not a variable modifier

`async` applies only to functions (including prototypes). It is not legal
on variables or in a `modifier_change` block (`async:`). Applies such as
`create` / `logon` must not be `async` — the driver reads the return
value immediately. See [async](async).

## `ref` is not a storage class

`ref` (and `&`) mark pass-by-reference **parameters**, call-site
arguments, and `foreach` loop variables. They are not `private`-style
modifiers. See [ref](ref).
