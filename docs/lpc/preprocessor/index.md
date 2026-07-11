---
title: preprocessor
---
# The LPC preprocessor

A preprocessor directive is a line whose first non-whitespace character
is `#`. Directives are not statements: they instruct the compiler while
it reads the file. A directive owns its whole logical line; ending a
line with a backslash (`\`) continues the directive onto the next line,
and comments may appear on directive lines.

Preprocessing in FluffOS is integrated into the compiler's single scan
of the source — there is no separate preprocessing pass — but the
semantics are the conventional C-family ones described here.

## Directives

| Directive | Purpose |
|---|---|
| [`#define`](define) | Define an object-like or function-like macro. |
| [`#undef`](define) | Remove a macro definition. |
| [`#include`](include) | Splice another file into the compile. |
| [`#if` / `#elif` / `#else` / `#endif`](conditionals) | Conditional compilation over constant expressions. |
| [`#ifdef` / `#ifndef`](conditionals) | Conditional compilation on whether a macro is defined. |
| [`#pragma`](pragma) | Toggle per-file compiler flags. |
| `#error text` | Abort the compile with `text` as the error message. |
| `#warn text` | Emit `text` as a compiler warning; does not fail the compile (and is suppressed by `#pragma no_warnings`). |
| `#echo text` | Print `text` to the driver's stderr during the compile. |
| `#line N ["file"]` | Override the reported line number (and optionally file name) for subsequent diagnostics. `# N` is accepted as a synonym. |

An unknown directive is a compile error.

## Predefined macros

The driver predefines macros describing itself and its configuration:
`__FILE__`, `__LINE__`, `__DIR__`, `FLUFFOS`, `MUDOS`, `MUD_NAME`,
`__VERSION__`, `MAX_INT`, `MIN_INT`, `MAX_FLOAT`, `MIN_FLOAT`,
`__ARCH__`, plus one `__PACKAGE_NAME__`-style macro per compiled-in
package and one `__FEATURE__`-style macro per enabled option, among
others. Predefined macros are **immutable**: both `#define` and
`#undef` of a predefined name are compile errors.
