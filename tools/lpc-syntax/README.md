# lpc-syntax — grammar-driven LPC tokenizer, highlighter, formatter, linter

The tokenizer/highlighter/formatter/linter core (Node ≥ 18, ESM) is
dependency-free JavaScript driven by **`lpc-grammar.json`** — a
machine-readable contract generated from the driver's own `grammar.y` +
`lexer.l` reserved-word/operator tables. The tooling can never drift from
the compiler: regeneration **fails loudly** if `grammar.y` grows a token
the spec doesn't categorize.

The supported ways to run the formatter are `testsuite/format.sh` (the
corpus CLI -- see "Formatting the testsuite corpus" below) and the VS
Code extension, which calls `formatLPC()` directly; both share the exact
same engine and `printWidth`/`indentSize` options. There is deliberately
no formatter-framework integration (an ESLint plugin was prototyped and
dropped mid-branch: a thin wrapper adding an npm dependency without using
any framework capability; an LSP server is the planned next
editor-integration surface).

## Files

| File | What |
|---|---|
| `generate_ebnf.py` | The generator: composes `grammar.ebnf`, emits `lpc-grammar.json`, and generates the declarative TextMate grammar (`lpc.tmLanguage.json`). Run via the `generate_ebnf` CMake target. |
| `grammar_lexical.ebnf.in` | Hand-authored *Lexical* + *Preprocessor* EBNF layers (terminals, escapes, templates, text blocks, directive grammar). |
| `grammar.ebnf` | **Generated** — the three-layer EBNF (hand-authored layers + bison-derived *Syntax* layer from `src/compiler/internal/grammar.y`). |
| `lpc-grammar.json` | **Generated** — keywords, type/modifier keywords, operators (longest-match ordered), punctuation, preprocessor directives, token categories, and all grammar productions. Never hand-edit. |
| `lpc.tmLanguage.json` | **Generated** — the declarative TextMate grammar for editors, emitted from the grammar contract. Never hand-edit. |
| `tokenizer.mjs` | Grammar-driven scanner: strings/escapes, template literals with nested `${}` interpolation (interpolated code is recursively tokenized), `@`/`@@` text blocks, comments, `#` directive lines with `\` continuations, numbers (hex/binary/underscores/reals), keyword classification, longest-match operators. Every token carries absolute char offsets (`start`/`end`) in addition to `line`/`col`. |
| `highlight.mjs` | `highlightLPC(source)` → HTML with `<span class="lpc-...">` runs; `defaultCss` export. |
| `format.mjs` | `formatLPC(source, options)` → the formatter: brace-depth reindentation (`options.indentSize` spaces per level, default `DEFAULT_INDENT_SIZE = 2`, also exported), operator spacing, directives at column 0, comments/strings/text blocks verbatim, and bracket/comma-aware line wrapping once a rendered line exceeds `options.printWidth` (default `DEFAULT_PRINT_WIDTH = 100`, matching `ColumnLimit` in `src/.clang-format`; also exported). Line-break decisions follow the source rather than forcing a canonical shape both ways: a `{ ... }` block the source wrote on ONE line stays on one line, whatever its statement count, if the render still fits `printWidth` -- `void event_ping(object from, int v) { origin = from; value = v + 1; }` stays exactly as written; a block that is a call's last argument keeps its `);`/`:)` glued to the closing brace (`});`, `} :), 1);`); a call, condition, or declaration parameter list already split across multiple source lines keeps that layout instead of collapsing onto one line -- including the CLOSE's own placement (a `");` glued onto the last element stays glued; a close on its own line keeps it); a genuinely empty block (nothing at all between `{`/`}`, not even a comment) always collapses to `{}` regardless of source layout; statement groups the source wrote on ONE physical line stay merged (`a &= 3;  ASSERT_EQ(2, a);` renders as one single-spaced line -- statements on separate source lines keep their own lines); a multi-line continuation at statement level keeps its source breaks, indented one level -- string-concat chains (adjacent string/template/textblock literals, `+` tails or leading `+`, and bare identifiers in the chain, i.e. string-valued macros) and declarator lists broken after a trailing comma (`int a, b,` newline `c;`); a heredoc terminator line follows the source (`TEXT;` keeps its `;` glued -- the driver rescans the rest of the terminator line as code); a trailing comment stays at the end of the line the source put it on (`if (cond)  // note`), gets at least two spaces before its `//` (clang-format's `SpacesBeforeTrailingComments: 2`) and keeps a WIDER source gap exactly, so hand-aligned trailing-comment columns stay aligned; and a CRLF source stays CRLF (at_block_crlf.lpc). a brace-less if/while/for/foreach body or else-body (`if (cond)\n  stmt;`) likewise keeps its own line instead of collapsing onto the condition, including a chained `else if`/`else` and nested brace-less bodies with no real block anywhere (`if (a)\n  if (b)\n    stmt();`), which all resolve together once the chain's one real terminating statement or block is reached. Deterministic and idempotent in all cases (including after wrapping) -- a second pass sees exactly the line grouping the first pass emitted and reproduces it exactly. Powers the VS Code extension's Format Document / format-on-save and `testsuite/format.sh`. **Known limitation:** a NON-STRING statement continuation with no enclosing bracket at all (`return a +\n    b;` with no parens) still collapses onto one line -- bracketed constructs, if/while/for/foreach/else bodies, and multi-line STRING continuations track the source's original line breaks; doing so for an arbitrary unbracketed expression continuation would need real expression parsing, which this tokenizer-driven formatter deliberately doesn't attempt. A `#define NAME(params) body` macro that stringizes one of its parameters via `#param` (not plain `##`, which is token paste and doesn't care about spelling) has that parameter's SOURCE SPELLING baked into the expansion (`#define STR(x) #x` then `STR(1+2)` must stringize to `"1+2"`, not `"1 + 2"`) -- the formatter detects this (`collectStringizeMacros`/`maskStringizeArguments`) by mirroring the driver's own preprocessing (folded `\`-continuations and stripped comments before analysis, odd-`#`-run detection so `###x` still stringizes, flags unioned across every definition in the file since `#if` truth can't be evaluated, keyword-named macros accepted at call sites, and driver-model argument boundaries that nest only the `(` character -- with a whole-call freeze fallback when a span would be bracket-unbalanced) and freezes the matching call-site argument's tokens verbatim so they're never re-spaced. `testsuite/single/tests/compiler/preprocessor_stringize.lpc` is the driver-level canary for all of these shapes. A final token-merge safety net in `renderLine` guarantees no two tokens are ever butted together whose concatenation would re-lex differently (`a - --b` must not render as `a ---b`, which re-lexes as `(a--) - b`; `- -x` must not become the pre-decrement `--x`; `f( ::g() )` must not become `f(::g())`, whose `(:` re-lexes as a functional-literal opener). An empty or whitespace-only source (a real corpus case: `testsuite/clone/inh0.lpc` is an intentionally-empty inherited object) formats to an empty string, not a manufactured newline. |
| `lint.mjs` | `lintLPC(source)` → structural diagnostics (illegal characters, unterminated literals, unbalanced brackets, mismatched conditionals) with 1-based positions. Powers the VS Code extension's as-you-type diagnostics. |
| `../../testsuite/format.sh` | Auto-formats the whole testsuite corpus (`testsuite/**/*.lpc` and `*.c`) in place, or verifies it with `--check` (exit 1 if anything is unformatted -- CI-friendly). Dependency-free (node only, no `npm install`). Owns the corpus EXCLUSION list: the two deliberately-malformed UTF-8 compiler fixtures are raw byte fixtures that must never be read/written as text (see AGENTS.md §7). Lives next to the corpus it formats; drives `bin/format-corpus.mjs` here, which refuses to write any file whose formatted output isn't token-sequence-equivalent to the input, literal-content byte-identical, and idempotent. |
| `bin/format-corpus.mjs` | The engine behind `testsuite/format.sh`: file paths on stdin, `--check` for dry-run; every write is gated on token-sequence equivalence, literal-content byte-identity, and idempotency; any violation exits nonzero. |
| `package.json` | Metadata only (no dependencies): `npm test` runs test.mjs; `npm run format`/`check` forward to `testsuite/format.sh`. |
| `test.mjs` | `node tools/lpc-syntax/test.mjs` — tokenizer/highlighter/formatter/linter/tmLanguage assertions (dependency-free). |

The VS Code extension itself lives in the
[fluffos/fluffos-vscode](https://github.com/fluffos/fluffos-vscode) repo;
it pins a fluffos commit via a git submodule and syncs this directory's
outputs (the `.mjs` tooling, `lpc-grammar.json`, `lpc.tmLanguage.json`)
into the packaged extension at build time. Extension behavior changes go
there; language-engine changes go here.

## Regenerating the grammar contract

```bash
cmake --build build --target generate_ebnf
```

This regenerates `grammar.ebnf` (the three-layer EBNF: hand-authored
*Lexical* and *Preprocessor* layers from `grammar_lexical.ebnf.in`,
plus the bison-derived *Syntax* layer), `lpc-grammar.json`, and the
TextMate grammar (`lpc.tmLanguage.json`). Rerun it whenever
`grammar.y`, the reserved-word table, the lexical layers, or
`tokenizer.mjs`/`lint.mjs`/`format.mjs` change — an uncategorized new
token aborts generation with an error naming it.

## Usage

```js
import { tokenize } from './tokenizer.mjs';
import { highlightLPC, defaultCss } from './highlight.mjs';
import { formatLPC } from './format.mjs';

const html = highlightLPC('int f() { return `v=${x}`; }');
const pretty = formatLPC('int  f(){return   1;}');
const wide = formatLPC(longSource, { printWidth: 120 }); // default is 100
const fourSpace = formatLPC(source, { indentSize: 4 }); // default is 2
```

## Relation to the C++ formatter (`src/.clang-format`)

The driver's C++ sources are formatted with clang-format (Google base,
`IndentWidth: 2`, `ColumnLimit: 100`, `PointerAlignment: Left`,
`SortIncludes: Never`). This formatter deliberately matches it on every
language-common rule so the repo reads as one style:

* **Shared**: 2-space indent, 100-column limit, attached (K&R) braces
  with cuddled `else`/`while`, `if (`-spaced vs call-tight parens,
  indented `case` labels (`IndentCaseLabels`), tight casts
  (`(string)x`, `SpaceAfterCStyleCast: false`), tight unary/`++`/`--`/
  `[]`/`->`/`::`, spaced binary/assignment/ternary operators,
  comma/semicolon spacing, no space inside parens, trailing `//`
  comments two spaces off the code (`SpacesBeforeTrailingComments: 2`)
  with hand-aligned wider gaps preserved (`AlignTrailingComments` in
  spirit, without ever moving a comment), directives at column 0, no
  include sorting.
* **LPC-specific by design**: `type *name` binds the `*` to the name
  (the opposite of `PointerAlignment: Left` -- LPC's `*` is the array
  marker, and the corpus writes `string *arr` ~5:1); array/mapping
  literals keep inner padding (`({ 1, 2 })`, `([ "a": 1 ])`) with empty
  literals tight (`({})`, `([])`, `{}`); `catch(`/`new(` are call-tight;
  functional literals space their bounds (`(: f :)`); default-argument
  colons are tight-before (`string b: (: "x" :)`).
* **Deliberately NOT adopted from clang-format**: line-reflow
  canonicalization. clang-format re-decides every line break; this
  formatter follows the source's breaks (one-liner blocks, multiline
  calls, glued closes, blank-line runs, comment reflow -- see the
  `format.mjs` row above), because the corpus is hand-shaped test code
  where layout carries intent.

## Formatting the testsuite corpus

```bash
testsuite/format.sh           # format testsuite/**/*.lpc,*.c in place
testsuite/format.sh --check   # exit 1 if anything is unformatted (CI)
```

Dependency-free (node only). Skips the two malformed-UTF8 fixtures
(raw byte fixtures -- see the exclusion note in the script and
AGENTS.md §7) and refuses to write any file whose formatted output
isn't token-sequence-equivalent and idempotent. After reformatting,
still run the real driver testsuite (see "Validating a formatter
change" below) before committing.

## Validating a formatter change

`test.mjs` plus a corpus-wide reformat with token-sequence-equivalence
checking (tokenize the "before" and "after" text, compare `kind:text`
pairs ignoring whitespace) catches most regressions, but it has a real
blind spot: **it cannot catch a bug that lives in the tokenizer itself**.
If the tokenizer misclassifies some span the same (wrong) way on both
sides of a reformat, the two token sequences trivially match even though
the rendered *text* changed something semantically real — a comment's
`*/` gaining an internal space and no longer closing the comment to a
real lexer, or a `#define STR(x) #x`-style stringize macro's argument
gaining a space and stringizing to a different string. Both of these were
real bugs found in this tool that the JS-level self-checks reported as
clean.

The only way to catch that class of bug is to validate against an
independent ground truth: **build the actual FluffOS driver and run the
real LPC testsuite against the reformatted corpus**:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo   # from the repo root
cmake --build build --target driver -j$(nproc)
cd testsuite && ../build/src/driver etc/config.test -ftest
```

A clean run prints `Checks succeeded.` and exits 0 (see AGENTS.md §7 for
the pass/fail signal and randomized-order flakiness notes — run it 2–3×
after a tokenizer/format.mjs change touching comment/directive/macro-
adjacent handling). This doesn't require `flex` (the pinned, committed
lexer/parser sources are used if it's missing) but does need the usual
driver dependencies (`libicu-dev`, `libjemalloc-dev`, a MySQL/MariaDB
client dev package, `libpcre3-dev`) if they aren't already installed.
