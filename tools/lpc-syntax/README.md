# lpc-syntax — grammar-driven LPC tokenizer, highlighter, formatter

Dependency-free JavaScript (Node ≥ 16, ESM) tooling driven by
**`lpc-grammar.json`** — a machine-readable contract generated from the
driver's own `grammar.y` + `lexer.l` reserved-word/operator tables. The
tooling can never drift from the compiler: regeneration **fails loudly**
if `grammar.y` grows a token the spec doesn't categorize.

## Files

| File | What |
|---|---|
| `generate_ebnf.py` | The generator: composes `grammar.ebnf`, emits `lpc-grammar.json`, and generates the VS Code extension's assets (`vscode/syntaxes/lpc.tmLanguage.json`, `vscode/lib/*`). Run via the `generate_ebnf` CMake target. |
| `grammar_lexical.ebnf.in` | Hand-authored *Lexical* + *Preprocessor* EBNF layers (terminals, escapes, templates, text blocks, directive grammar). |
| `grammar.ebnf` | **Generated** — the three-layer EBNF (hand-authored layers + bison-derived *Syntax* layer from `src/compiler/internal/grammar.y`). |
| `lpc-grammar.json` | **Generated** — keywords, type/modifier keywords, operators (longest-match ordered), punctuation, preprocessor directives, token categories, and all grammar productions. Never hand-edit. |
| `tokenizer.mjs` | Grammar-driven scanner: strings/escapes, template literals with nested `${}` interpolation (interpolated code is recursively tokenized), `@`/`@@` text blocks, comments, `#` directive lines with `\` continuations, numbers (hex/binary/underscores/reals), keyword classification, longest-match operators. |
| `highlight.mjs` | `highlightLPC(source)` → HTML with `<span class="lpc-...">` runs; `defaultCss` export. |
| `format.mjs` | `formatLPC(source)` → basic formatter: brace-depth reindentation, statement-per-line, operator spacing, directives at column 0, comments/strings/text blocks verbatim. Deterministic and idempotent. Powers the VS Code extension's Format Document / format-on-save. |
| `lint.mjs` | `lintLPC(source)` → structural diagnostics (illegal characters, unterminated literals, unbalanced brackets, mismatched conditionals) with 1-based positions. Powers the VS Code extension's as-you-type diagnostics. |
| `vscode/` | VS Code extension: declarative highlighting (generated TextMate grammar), structural diagnostics as you type, Format Document / format-on-save, real `lpcc` compiler errors on save. See `vscode/README.md`. |
| `test.mjs` | `node tools/lpc-syntax/test.mjs` — 49 assertions, no dependencies (tokenizer, highlighter, formatter, linter, generated VS Code assets). |

## Regenerating the grammar contract

```bash
cmake --build build --target generate_ebnf
```

This regenerates `grammar.ebnf` (the three-layer EBNF: hand-authored
*Lexical* and *Preprocessor* layers from `grammar_lexical.ebnf.in`,
plus the bison-derived *Syntax* layer), `lpc-grammar.json`, and the VS
Code extension's generated assets (`vscode/syntaxes/lpc.tmLanguage.json`
plus the self-contained copies under `vscode/lib/`). Rerun it whenever
`grammar.y`, the reserved-word table, the lexical layers, or
`tokenizer.mjs`/`lint.mjs` change — an uncategorized new token aborts
generation with an error naming it.

## Usage

```js
import { tokenize } from './tokenizer.mjs';
import { highlightLPC, defaultCss } from './highlight.mjs';
import { formatLPC } from './format.mjs';

const html = highlightLPC('int f() { return `v=${x}`; }');
const pretty = formatLPC('int  f(){return   1;}');
```
