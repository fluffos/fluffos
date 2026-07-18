---
title: style guide
---

# LPC style guide

This is the house style for LPC code in the FluffOS tree (the
`testsuite/` mudlib and example code), and a sane default for any
mudlib. It follows the same philosophy as the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
that the driver's own C++ sources are formatted to (`src/.clang-format`:
Google base, 2-space indent, 100-column limit), with widely-used
community C rules (cf. [MaJerle/c-code-style](https://github.com/MaJerle/c-code-style))
adapted where LPC differs from C.

Two principles come before every specific rule:

1. **Optimize for readers, not writers.** Code is read far more often
   than it is written; a rule earns its place by making the next read
   cheaper.
2. **Imitate the surrounding code.** When editing an existing file,
   consistency with what is already there beats this document. (This is
   also the single most important rule of MaJerle's C guide.)

The *layout* rules below are machine-enforced by the
[LPC formatter](formatter) — run `testsuite/format.sh` and they are
applied for you; CI rejects unformatted testsuite code. The *naming and
practice* rules are judgment calls the formatter deliberately does not
touch.

## Layout (formatter-enforced)

### Indentation and line length

* **2 spaces** per indent level. Never tabs.
* **100 columns** maximum, matching the driver's C++ `ColumnLimit`.
  Long statements are wrapped at the outermost bracket group, one
  element per line. Three things are exempt because splitting them
  changes program behavior or meaning: string/template/heredoc
  literals, comments, and a trailing comment's line.
* Preprocessor directives start at **column 0**, never indented.

### Braces

K&R style everywhere — the opening brace shares the line of its
declaration or keyword, `else` and a `do`-loop's `while` cuddle the
closing brace:

```c
int find(string name) {
  if (!name) {
    return -1;
  } else {
    return lookup(name);
  }
}

do {
  step();
} while (more());
```

* A genuinely empty block collapses to `{}` — `void create() {}`.
* Braces are *recommended* for every control-flow body, but a
  single-statement body on its own line may omit them:

```c
if (!raw)
  return mask[0];
```

### Spaces

| Context | Rule | Example |
|---|---|---|
| Control-flow keyword before `(` | one space | `if (x)`, `while (x)`, `for (;;)`, `foreach (v in arr)`, `return (x)` |
| Call before `(` | tight | `write(msg)`, `funcs[0](5, 5)`, `f(a)(b)` |
| Call-like keywords | tight | `catch(err)`, `new("/obj")` |
| Inside `( )` | no padding | `f(a, b)`, not `f( a, b )` |
| Binary operators, assignments | one space each side | `a = b + c * d`, `a >>= 2` |
| Ternary | spaced | `a ? b : c` |
| Unary operators | tight to operand | `-a`, `!ok`, `~bits`, `++i`, `i++` |
| Comma / semicolon | tight before, space after | `f(a, b)`; `for (i = 0; i < n; i++)` |
| Cast | tight to operand | `(string)x`, `(mixed *)arr`, `return (int)v` |
| Index | tight | `arr[0]`, `m["key"]`, `s[1..<2]` |
| `->`, qualified `::` | tight | `ob->query()`, `efun::write()` |
| Array declaration `*` | binds to the name | `string *names`, `int *fn(mixed *args)` |
| Array literal | inner padding | `({ 1, 2, 3 })`, empty `({})` |
| Mapping literal | inner padding, tight key colon | `([ "a": 1, "b": 2 ])`, empty `([])` |
| Functional literal | spaced bounds | `(: f :)`, `(: $1 + $2 :)` |
| Default-argument colon | tight before | `void f(int a, string b: (: "x" :))` |
| Spread | tight after the expression | `f(args...)`, but bare `f(int a, ...)` |
| Label colons | tight | `case 1:`, `default:`, `private:` |
| Trailing `//` comment | at least two spaces off the code | `return 1;  // why` |

Two of these are deliberate departures from the C++ config, because the
constructs mean something different in LPC:

* `string *names` — LPC's `*` is the **array marker**, not a pointer,
  and mudlib code overwhelmingly binds it to the name (the driver's C++
  uses `PointerAlignment: Left`, `type* name`).
* `({ ... })` / `([ ... ])` keep inner padding where C++ brace-init
  lists are tight — the delimiters are two characters wide and the
  padding keeps the pairs legible.

### Case labels

`case` and `default` indent one level inside the `switch`, bodies one
more (Google `IndentCaseLabels`):

```c
switch (x) {
  case 1:
    handle_one();
    break;
  case LOW..HIGH:  // LPC range label
    handle_range();
    break;
  default:
    handle_rest();
}
```

### Line breaks follow the source

Unlike clang-format, the formatter does **not** re-decide your line
breaks; it preserves the shape you wrote (and only splits a line when
it exceeds the column limit):

* A block you wrote on one line stays on one line if it fits:
  `void event_ping(object from, int v) { origin = from; value = v + 1; }`
* A call or declaration you split across lines keeps that layout,
  including where you put the close — a `");` glued to the last
  argument stays glued; `});` sticks to the closing brace of a
  functional or inline block argument.
* Blank-line runs are preserved exactly; use them to group related
  statements (and one blank line between functions).
* A comment never moves to another line, and a trailing comment never
  causes the code before it to wrap.

Layout carries intent in hand-written test code; the formatter treats
it as meaningful.

## Naming

Measured against the testsuite corpus, LPC's dominant conventions are
C-flavored (they differ from Google C++'s `CapitalizedWords`
deliberately — LPC identifiers are historically lowercase):

| Entity | Convention | Example |
|---|---|---|
| Functions, applies, efun overrides | `lower_snake_case` | `query_name()`, `do_tests()` |
| Local & global variables | `lower_snake_case` | `save_file`, `num_users` |
| Macros / `#define` constants | `ALL_CAPS_WITH_UNDERSCORES` | `MAX_USERS`, `TUI_ANSI` |
| Source files | `lower_snake_case.lpc` | `hot_reload.lpc` |
| Classes | lowercase or `Capitalized` — be consistent per file | `class point`, `class Counter` |
| Include files | `lower_snake_case.h` | `tests.h` |

* Don't invent Hungarian prefixes or `m_`-style member markers; LPC
  objects are small enough that plain names with `private` do the job.
* Name a thing for what it is, not its type: `users`, not
  `user_array`.

## Visibility and structure

* **`private` by default.** Global variables and helper functions that
  aren't part of the object's interface should be `private` (the
  corpus uses `private` over `public` more than 20:1). Reserve
  `nomask` for security-critical functions that inheritors must not
  override.
* Declare locals in the **narrowest scope** that works, and initialize
  at the point of declaration. FluffOS allows declarations after
  statements — prefer declaring where the value first exists over a
  C89-style block of declarations at the top.
* One declaration per line when the initializers matter; grouping
  uninitialized same-type locals on one line (`int i, j, n;`) is fine.
* Prefer `foreach (member in aggregate)` over an indexed `for` loop
  when you don't need the index.
* Always give a `switch` a `default:` when it isn't exhaustive by
  construction.
* Prefer returning values over writing through `ref` parameters; use
  `ref` when the caller genuinely needs multiple results mutated in
  place.

## Comments

* `//` for line comments and trailing notes; `/* ... */` for longer
  block comments. (Unlike MaJerle's C rule — which bans `//` for C89
  compatibility — LPC has always had both; use whichever reads best.)
* Comment the **why**, not the what. A comment that restates the next
  line is noise; a comment that records a constraint the code can't
  express is gold.
* Trailing comments sit at least two spaces off the code. If you align
  a column of trailing comments by hand, the formatter preserves your
  alignment.

## Preprocessor practice

These C rules apply to LPC's preprocessor unchanged:

* Parenthesize macro parameters and the whole expansion:
  `#define SQUARE(x) ((x) * (x))`.
* Wrap multi-statement macros in `do { ... } while (0)` so they behave
  as one statement under `if`/`else`.
* Guard include files with `#ifndef NAME_H` / `#define NAME_H` /
  `#endif`.
* Prefer `#if defined(XYZ)` over `#ifdef XYZ` when the condition may
  grow more terms.
* Tests and optional-package code guard themselves with the package
  conditionals: `#ifdef __PACKAGE_ASYNC__`.

One LPC-specific caution: a macro that *stringizes* a parameter
(`#define STR(x) #x`) bakes the argument's exact source spelling into
program output — `STR(1+2)` is the string `"1+2"`. The formatter
detects this and never re-spaces such arguments, but be aware of it
when writing them by hand.

## Error handling

* `catch(...)` returns the error or `0` — always check or consciously
  discard it. Use `error("message")` (not raw `throw`) to raise.
* Never build a format string from untrusted input; pass it as an
  argument instead: `printf("%s", player_text)`.

## Enforcement

```bash
testsuite/format.sh            # format the corpus in place
testsuite/format.sh --check    # verify only; CI runs this
```

The formatter refuses to write any file whose output isn't
token-equivalent to the input, literal-byte-identical, and idempotent,
so running it is always safe. See the [formatter documentation](formatter)
for options, editor integration, and exactly what it does and does not
change.
