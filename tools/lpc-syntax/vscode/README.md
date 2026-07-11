# LPC (FluffOS) — VS Code extension

LPC language support generated from the FluffOS compiler's own grammar
contract (`tools/lpc-syntax/lpc-grammar.json`), so it cannot drift from
what the driver actually accepts.

## Features

* **Syntax highlighting** for `.lpc` files — keywords, types, modifiers,
  operators, preprocessor directives, strings/templates/text blocks,
  `(: ... :)` functionals, `$N` parameters. Declarative TextMate grammar
  (`syntaxes/lpc.tmLanguage.json`), **generated** from the grammar
  contract.
* **Structural diagnostics as you type** — illegal characters,
  unterminated strings/templates/block comments/text blocks, unbalanced
  brackets, mismatched `#if`/`#elif`/`#else`/`#endif` (from
  `lib/lint.mjs`, a generated copy of the grammar-driven linter).
* **Real compiler errors on save** (optional) — set the two `lpcc`
  settings and the file is compiled with the actual FluffOS front-end;
  its clang-style errors and warnings appear inline, including in
  `#include`d files.

## Settings

| Setting | Meaning |
|---|---|
| `lpc.lint.enabled` | Toggle the built-in structural lint (default on). |
| `lpc.lpcc.path` | Path to the `lpcc` binary (build target `lpcc`). |
| `lpc.lpcc.configFile` | Driver config file passed to `lpcc`. |
| `lpc.mudlibRoot` | Mudlib root; files are compiled by their path relative to it (default: the workspace folder). |

Legacy mudlibs that name LPC files `.c` can map them per-workspace:

```json
"files.associations": { "*.c": "lpc" }
```

## Install / develop

From this directory:

```bash
npx @vscode/vsce package        # produces fluffos-lpc-<version>.vsix
code --install-extension fluffos-lpc-*.vsix
```

or for development: open `tools/lpc-syntax/vscode/` in VS Code and press
F5 (Run Extension).

## Regenerating the generated files

`syntaxes/lpc.tmLanguage.json` and everything under `lib/` are emitted
by `tools/lpc-syntax/generate_ebnf.py` (CMake target `generate_ebnf`)
from the same grammar contract as the EBNF and the JS tooling — edit
the sources one level up, never these copies:

```bash
cmake --build build --target generate_ebnf
```
