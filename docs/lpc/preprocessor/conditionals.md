---
title: preprocessor / conditionals
---
# Conditional compilation

`#if`, `#ifdef`, `#ifndef`, `#elif`, `#else` and `#endif` select which
regions of a file the compiler sees.

```c
#ifdef __PACKAGE_SOCKETS__
int open_port() { return socket_create(STREAM, "cb"); }
#else
int open_port() { return -1; }
#endif
```

## #ifdef / #ifndef

Test only whether a macro is defined (user `#define`s and driver
predefines both count):

```c
#ifndef MUDLIB_H
#define MUDLIB_H
...
#endif  // classic include guard
```

## #if expressions

`#if` and `#elif` evaluate an integer constant expression. Macros in
the expression are expanded first; the expression is then evaluated
**over tokens with full C operator precedence** — parentheses, unary
`! ~ -`, arithmetic `* / % + -`, shifts, comparisons, bitwise
`& ^ |`, and logical `&& ||`:

```c
#define X 1+1
#if X*2 == 3        // expands to 1+1*2 -- true, as in C
#endif
```

Two extra operators are available:

* `defined(NAME)` (or `defined NAME`) — 1 if `NAME` is a defined
  macro.
* `efun_defined(name)` — 1 if the driver was built with the named
  efun.

```c
#if defined(__PACKAGE_CRYPTO__) && efun_defined(hash)
string h = hash("sha256", data);
#endif
```

An undefined identifier appearing in an `#if` expression evaluates to
`0`, as in C. A missing expression is a compile error.

## Nesting and skipped regions

Conditionals nest arbitrarily. Inside a false branch the compiler
still tracks `#if`/`#endif` nesting (so a skipped region may contain
its own conditionals), but nothing else in the region is evaluated —
including other directives. An unmatched `#elif`/`#else`/`#endif` is
a compile error.

Each conditional has the shape `#if` → any number of `#elif` → at most
one `#else` → `#endif`, and the compiler checks that shape in skipped
regions too: a second `#else`, or an `#elif` after the `#else`, is a
compile error. A conditional must also open and close in the same file:
a header that leaves an `#if` open is an error at the end of the header
("unterminated conditional directive"), and an `#elif`/`#else`/`#endif`
that would continue a conditional opened by the including file is an
error as well.
