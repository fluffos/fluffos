---
title: Development environment
slug: /lpc/dev-environment
description: Set up VS Code / Cursor, the LPC highlighter, and the formatter so LPC is highlighted as LPC.
---

# LPC development environment

FluffOS ships its own language tooling. The grammar is generated from the
driver compiler (`tools/lpc-syntax/`), so highlighting, format, and lint
cannot drift from what the VM actually accepts.

Use this page when you are writing LPC — in an editor, on this docs site,
or in another Docusaurus site.

## 1. Editor: LPC (FluffOS)

The [fluffos/fluffos-vscode](https://github.com/fluffos/fluffos-vscode)
extension is the supported editor plugin. It works in **VS Code**,
**Cursor**, **VSCodium**, and other Code OSS forks.

It packages the grammar, highlighter, formatter, and linter from
`tools/lpc-syntax/` at a pinned fluffos commit. Language-engine changes
go in **this** repo; extension UI and packaging go in fluffos-vscode.

**Install** from the [latest GitHub release](https://github.com/fluffos/fluffos-vscode/releases)
(`.vsix`):

```bash
code --install-extension fluffos-lpc-<version>.vsix
# Cursor:
cursor --install-extension fluffos-lpc-<version>.vsix
```

Marketplace / Open VSX: install **LPC (FluffOS)** when the publish job
has run (same extension id).

What you get:

| Feature | Notes |
|---|---|
| Syntax highlighting | TextMate grammar generated from `lpc-grammar.json` |
| Format Document / format-on-save | Same engine as `testsuite/format.sh` |
| As-you-type diagnostics | Structural lint (unbalanced brackets, unterminated literals, …) |
| Compiler errors on save | Optional: point at `lpcc` + a mudlib config |
| Compiler Explorer | Source / tokens / AST / bytecode views |
| Language server | Other editors can run the same LSP over stdio |

Useful settings (workspace or user):

```json
{
  "lpc.format.printWidth": 100,
  "lpc.format.indentSize": 2,
  "lpc.lpcc.path": "/path/to/fluffos/build/bin/lpcc",
  "lpc.lpcc.configFile": "/path/to/mudlib/config.cfg"
}
```

In a mudlib workspace, run **LPC: Initialize compiler config** once so
save-time diagnostics and the Explorer can find `master file` and include
dirs. Do not point that config at `fluffos/testsuite/` unless you are
changing the driver.

Standalone LSP (Neovim, Helix, …), after cloning fluffos-vscode and
`npm install` in `extension/`:

```bash
node extension/server/main.js --stdio
```

## 2. This documentation site

The docs site uses the same grammar via a first-party **Prism** plugin
(`tools/lpc-syntax/prism-lpc.cjs`). Fence LPC as `lpc`:

```lpc
foreach (string name, int weight in ([ "sword": 4, "lamp": 1 ])) {
    write(`${name} weighs ${weight}\n`);
}

string *lines = @@HELP
A shop. Dust. A counter.
Type "list" to see wares.
HELP;
```

Existing ` ```c ` fences on language, efun, apply, stdlib, and concept
pages are treated as LPC at build time. The remark plugin leaves
`docs/driver/` and `build-wasm` alone, so LPC samples there must use
` ```lpc `. Real C stays ` ```c ` / ` ```cpp `.

Prefer ` ```lpc ` on new pages.

## 3. Use the highlighter on another Docusaurus 3 site

The plugin lives in the driver repo (not a separate npm package yet).
From a fluffos checkout:

1. Register the language (swizzle `prism-include-languages` and
   `require` the file while `globalThis.Prism` is the Prism instance):

```js
require('/path/to/fluffos/tools/lpc-syntax/prism-lpc.cjs');
```

2. Optional — remap leftover ` ```c ` fences to LPC, except paths you
   pass in `keepCAsC` (this site keeps `docs/driver/` and `build-wasm`).
   Use ` ```lpc ` for LPC on those pages, and ` ```cpp ` for C samples
   elsewhere.

```js
// docusaurus.config.js
presets: [
  ['classic', {
    docs: {
      remarkPlugins: [
        require('/path/to/fluffos/tools/lpc-syntax/docusaurus-plugin.cjs'),
      ],
    },
  }],
];
```

The remark plugin does not rewrite `docs/driver/**` or `build-wasm`.

HTML highlighter (no Prism) for static pages or a mudlib help site:

```js
import { highlightLPC, defaultCss } from './tools/lpc-syntax/highlight.mjs';

const html = highlightLPC('int f() { return `v=${x}`; }');
```

## 4. Formatter without an editor

Same engine as the extension. Node ≥ 18; no `npm install`.

```bash
testsuite/format.sh            # format testsuite/**/*.lpc,*.c
testsuite/format.sh --check    # CI
find lib -name '*.lpc' | node tools/lpc-syntax/bin/format-corpus.mjs
```

Details: [LPC formatter](formatter) and the [style guide](style-guide).

## 5. Efun metadata for your own tools

After a driver build:

```bash
cd build && ./generate_keywords   # writes keywords.json
```

That file is the efun list for the **compiled** binary (packages you
actually enabled). See [generate_keywords](../cli/generate_keywords).

## 6. Suggested mudlib workspace

```text
your-mudlib/
  AGENTS.md          # copy from /mudlib-agents
  config.cfg         # or whatever that lib already ships
  .vscode/settings.json
```

`.vscode/settings.json` (optional):

```json
{
  "files.associations": {
    "*.c": "lpc",
    "*.h": "lpc",
    "*.lpc": "lpc"
  },
  "[lpc]": {
    "editor.formatOnSave": true
  }
}
```

Many mudlibs still use `.c` for LPC. Associating those files with the
LPC language is how the highlighter actually runs.

**Do not** put this `files.associations` block in the `fluffos/fluffos`
driver repo — that tree’s `.c` / `.h` files are C++. Only use it in a
mudlib workspace.

Then fill [Mudlib AGENTS.md](../mudlib-agents) so the next session knows
ports and the master path.

## Related

- [From zero to a running mud](../start)
- [Ecosystem](../ecosystem) — fluffos-vscode and the rest of the org
- [Build from source](../build) — `lpcc` lives next to `driver`
