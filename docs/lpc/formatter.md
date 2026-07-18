---
title: formatter
---

# The LPC formatter

FluffOS ships a dependency-free LPC code formatter in
`tools/lpc-syntax/` — the same grammar-driven engine that powers the
VS Code extension's *Format Document* and the testsuite's CI format
check. It needs only Node.js (≥ 18); there is no `npm install`.

It enforces the layout half of the [LPC style guide](style-guide):
spacing, indentation, brace placement, and wrapping — while treating
your line-break decisions, blank lines, and comments as meaningful and
leaving them where you put them.

## Running it

Over the testsuite corpus (from the repo root):

```bash
testsuite/format.sh            # format testsuite/**/*.lpc,*.c in place
testsuite/format.sh --check    # verify only, exit 1 if unformatted (CI)
```

From your own scripts, on any file set:

```bash
find lib -name '*.lpc' | node tools/lpc-syntax/bin/format-corpus.mjs           # in place
find lib -name '*.lpc' | node tools/lpc-syntax/bin/format-corpus.mjs --check  # verify
```

As a library:

```js
import { formatLPC } from './tools/lpc-syntax/format.mjs';

const pretty = formatLPC(source);                       // defaults
const wide = formatLPC(source, { printWidth: 120 });    // default 100
const four = formatLPC(source, { indentSize: 4 });      // default 2
```

In VS Code, the extension under `tools/lpc-syntax/vscode/` provides
*Format Document* and format-on-save with the same engine, configured
by `lpc.format.printWidth` (default 100) and `lpc.format.indentSize`
(default 2).

## Options

| Option | Default | Meaning |
|---|---|---|
| `printWidth` | `100` | Preferred maximum line length. Matches `ColumnLimit` in `src/.clang-format`, so LPC and the driver's C++ wrap at the same column. |
| `indentSize` | `2` | Spaces per indent level. Matches the C++ `IndentWidth`. |

## What it normalizes

* Indentation (brace depth, `case` labels, wrapped continuations) and
  trailing whitespace.
* Token spacing per the [style guide's spacing table](style-guide#spaces):
  control-flow vs call parens, operators, casts, unary tightness,
  literal padding — `({ 1, 2 })`, `([ "a": 1 ])`, `(: f :)` — index
  and mapping colons, label colons, and the rest.
* Empty blocks collapse to `{}`.
* Trailing `//` comments get at least two spaces before them; a wider
  hand-aligned gap is kept exactly.
* Preprocessor directives move to column 0.
* A rendered line longer than `printWidth` is split at its outermost
  bracket group (call arguments, array/mapping elements), one element
  per line, recursively as needed.

## What it preserves

The formatter follows the source instead of canonicalizing both ways —
the opposite of clang-format's full reflow:

* **Your line breaks.** A one-line block stays one line (if it fits);
  a multi-line call or declaration keeps its layout, including whether
  the closing `)` / `");` / `});` is glued to the last element or on
  its own line; a brace-less `if` body on its own line stays there.
* **Blank lines**, exactly as written — including runs of more than
  one.
* **Comments.** Never moved to another line, never re-wrapped, never
  allowed to force the code before them to wrap. Content is untouched.
* **Literal content, byte for byte.** Strings, `` ` ``-templates,
  `@TEXT` heredocs, and char literals are never split, re-indented, or
  re-spaced internally — a heredoc's body and terminator lines are
  reproduced verbatim (only its `;` placement on the terminator line
  follows the source).
* **Stringized macro arguments.** `#define STR(x) #x` bakes the
  argument's source spelling into a string at compile time, so the
  formatter detects stringizing macros (mirroring the driver's own
  preprocessing rules, including `#`-run parity and `\`-continuations)
  and freezes the matching call-site arguments verbatim —
  `STR(1+2)` keeps stringizing to `"1+2"`, never `"1 + 2"`.
* **CRLF sources stay CRLF.**

One known limitation: an unbracketed, non-string expression
continuation (`return a +` newline `b;` with no parentheses) collapses
onto one line — tracking that break for arbitrary expressions would
need a full expression parser, which the token-driven design
deliberately avoids. Parenthesize the continuation if the break
matters.

## Safety guarantees

Every write through `format.sh` / `format-corpus.mjs` is gated; a file
that would violate any of these is reported, left untouched, and the
run exits nonzero:

1. **Token-sequence equivalence** — the output re-tokenizes to exactly
   the input's token kinds and spellings; the formatter cannot change
   what the compiler sees.
2. **Literal byte-identity** — every string/template/heredoc/char/
   comment/directive token's content is byte-identical.
3. **Idempotency** — formatting the output again reproduces it
   exactly.

Deliberately malformed fixtures that are not valid text (the two
raw-byte bad-UTF-8 compiler fixtures under
`testsuite/single/tests/compiler/fail/`) are excluded by
`testsuite/format.sh`; anything the tokenizer cannot fully understand
is refused rather than guessed at.

Beyond the built-in gates, formatter changes are validated against the
real driver: the whole reformatted testsuite must still pass
`driver etc/config.test -ftest`. That end-to-end check exists because
a token-level self-check cannot catch a bug that lives in the
tokenizer itself — see `tools/lpc-syntax/README.md` for the full
methodology.

## Relation to the C++ style

The driver's C++ is formatted by clang-format (`src/.clang-format`:
Google base, 2-space indent, 100 columns). The LPC formatter matches
it on every language-common rule — indent, column limit, brace
attachment, keyword/call paren spacing, operator spacing, tight casts,
indented case labels, two-space trailing comments — and diverges only
where LPC is genuinely different (`string *arr` array markers, padded
`({ })` / `([ ])` literals, call-tight `catch(` / `new(`, spaced
`(: :)` bounds) or where hand-shaped layout carries intent (no line
reflow). The full comparison lives in `tools/lpc-syntax/README.md`.
