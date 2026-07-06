# lpc-syntax — grammar-driven LPC tokenizer, highlighter, formatter

Dependency-free JavaScript (Node ≥ 16, ESM) tooling driven by
**`lpc-grammar.json`** — a machine-readable contract generated from the
driver's own `grammar.y` + `lexer.l` reserved-word/operator tables. The
tooling can never drift from the compiler: regeneration **fails loudly**
if `grammar.y` grows a token the spec doesn't categorize.

## Files

| File | What |
|---|---|
| `lpc-grammar.json` | **Generated** — keywords, type/modifier keywords, operators (longest-match ordered), punctuation, preprocessor directives, token categories, and all grammar productions. Never hand-edit. |
| `tokenizer.mjs` | Grammar-driven scanner: strings/escapes, template literals with nested `${}` interpolation (interpolated code is recursively tokenized), `@`/`@@` text blocks, comments, `#` directive lines with `\` continuations, numbers (hex/binary/underscores/reals), keyword classification, longest-match operators. |
| `highlight.mjs` | `highlightLPC(source)` → HTML with `<span class="lpc-...">` runs; `defaultCss` export. |
| `format.mjs` | `formatLPC(source)` → basic formatter: brace-depth reindentation, statement-per-line, operator spacing, directives at column 0, comments/strings/text blocks verbatim. Deterministic and idempotent. |
| `test.mjs` | `node tools/lpc-syntax/test.mjs` — 27 assertions, no dependencies. |

## Regenerating the grammar contract

```bash
cmake --build build --target generate_ebnf
```

This regenerates **both** `src/compiler/internal/grammar.ebnf` (the
three-layer EBNF: hand-authored *Lexical* and *Preprocessor* layers from
`grammar_lexical.ebnf.in`, plus the bison-derived *Syntax* layer) and
`tools/lpc-syntax/lpc-grammar.json`. Rerun it whenever `grammar.y`, the
reserved-word table, or the lexical layers change — an uncategorized new
token aborts generation with an error naming it.

## Usage

```js
import { tokenize } from './tokenizer.mjs';
import { highlightLPC, defaultCss } from './highlight.mjs';
import { formatLPC } from './format.mjs';

const html = highlightLPC('int f() { return `v=${x}`; }');
const pretty = formatLPC('int  f(){return   1;}');
```
