---
title: compiler diagnostics
---
# Reading compiler diagnostics

The LPC compiler reports errors and warnings clang-style: a
`file:line:column` header, the offending source line with a gutter, a
caret marking the position (with `~~~` ranges for operands where they
apply), and follow-up notes.

```text
/cmds/ftpd.lpc:12:5: error: Undefined variable 'mudinfo'
   12 |     mudinfo = query_mud_info();
      |     ^~~~~~~
```

## Macro expansion notes

When the error position is inside a macro expansion, the diagnostic
carries one `note: expanded from macro '...'` per expansion level,
each with the macro's definition site and its definition line:

```text
/obj/board.lpc:44:13: error: Illegal type to +
   44 |     total = SUM_OF(a, b);
      |             ^~~~~~~~~~~~
/include/board.h:7:9: note: expanded from macro 'SUM_OF'
    7 | #define SUM_OF(x, y) ((x) + (y))
      |         ^
```

## Include chains and load reasons

Errors inside an `#include`d file print the chain of includes that got
there, and errors while loading an inherited file record why the file
was being compiled:

```text
In file included from /cmds/ftpd.lpc:3:
/include/ftpdconf.h:15:1: error: ...
```

```text
note: while loading '/std/container' inherited by '/std/chest'
```

## Warnings

Warnings use the same format with `warning:` severity. They do not
fail the compile. `#pragma no_warnings` suppresses them for the rest
of the file (see [pragma](preprocessor/pragma)). Notable warnings:

* **Macro redefinition** — `#define`-ing an existing macro with a
  different body warns (with a note pointing at the previous
  definition) and the new definition takes effect. Redefining with an
  identical body is silent. See [define](preprocessor/define).

## Fix-it hints

Some diagnostics carry a replacement suggestion rendered under the
caret when the fix is unambiguous — for example an unknown escape
sequence in a string literal, where the character stands for itself
and the backslash should be dropped:

```text
/obj/sign.lpc:9:20: warning: Unknown escape sequence '\m'.
    9 |     write("Read \me!");
      |                 ^~
      |                 m
```

## Efuns from packages this driver lacks

Packages are selected when the driver is compiled, so a driver built
without one has no such efun at all and a call to it is an ordinary
undefined function. Since that says nothing about the name being a real
efun, the compiler names the package it would have come from:

```text
/feature/user_gmcp.lpc:112:47: error: Undefined function hash (an efun of PACKAGE_CRYPTO, which this driver was not built with)
```

The note appears only for names this build genuinely lacks. An efun the
driver does have never reaches this diagnostic, and a name that no
package declares gets the plain message, so the note is never a guess.

From LPC, the same question is answered by the per-package predefines:

```c
#ifdef __PACKAGE_CRYPTO__
    // hash() exists
#endif
```

## Legacy error context

`#pragma show_error_context` is a legacy flag that predates these
clang-style diagnostics. The output shown above always includes the
source snippet and caret, so the pragma no longer changes how
**compiler** diagnostics are rendered — it survives only for the
driver's older runtime error-logging path.
