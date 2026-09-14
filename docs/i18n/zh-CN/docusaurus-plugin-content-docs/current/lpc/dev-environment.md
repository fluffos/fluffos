---
title: 开发环境
slug: /lpc/dev-environment
description: 配置 VS Code / Cursor、LPC 高亮和格式化，让 LPC 按 LPC 高亮。
---

# LPC 开发环境

FluffOS 自带语言工具。语法从驱动编译器生成（`tools/lpc-syntax/`），所以高亮、格式化和检查不会和 VM 实际接受的语言分叉。

写 LPC 时用这一页：编辑器、本文档站、或其他 Docusaurus 站点。

## 1. 编辑器：LPC (FluffOS)

[fluffos/fluffos-vscode](https://github.com/fluffos/fluffos-vscode)
是官方扩展。适用于 **VS Code**、**Cursor**、**VSCodium** 以及其他 Code OSS 分支。

它按钉住的 fluffos 提交打包 `tools/lpc-syntax/` 里的语法、高亮、格式化和 linter。语言引擎改动进**本仓库**；扩展 UI 进 fluffos-vscode。

从 [GitHub Release](https://github.com/fluffos/fluffos-vscode/releases) 装 `.vsix`：

```bash
code --install-extension fluffos-lpc-<version>.vsix
# Cursor：
cursor --install-extension fluffos-lpc-<version>.vsix
```

Marketplace / Open VSX：发布完成后搜 **LPC (FluffOS)**。

| 功能 | 说明 |
|---|---|
| 语法高亮 | 由 `lpc-grammar.json` 生成的 TextMate 语法 |
| Format Document / 保存时格式化 | 与 `testsuite/format.sh` 同一引擎 |
| 键入时诊断 | 括号不配、未闭合字面量等 |
| 保存时编译错误 | 可选：指向 `lpcc` + mudlib 配置 |
| Compiler Explorer | 源码 / token / AST / 字节码 |
| Language server | 其他编辑器可用同一 LSP（stdio） |

常用设置：

```json
{
  "lpc.format.printWidth": 100,
  "lpc.format.indentSize": 2,
  "lpc.lpcc.path": "/path/to/fluffos/build/bin/lpcc",
  "lpc.lpcc.configFile": "/path/to/mudlib/config.cfg"
}
```

在 mudlib 工作区先跑一次 **LPC: Initialize compiler config**。不要把配置指到 `fluffos/testsuite/`，除非你在改驱动。

独立 LSP（Neovim、Helix 等），克隆 fluffos-vscode 并在 `extension/` 里 `npm install` 后：

```bash
node extension/server/main.js --stdio
```

## 2. 本文档站

文档站用同一份语法，通过第一方 **Prism** 插件
（`tools/lpc-syntax/prism-lpc.cjs`）。LPC 代码块标成 `lpc`：

```lpc
foreach (string name, int weight in ([ "sword": 4, "lamp": 1 ])) {
    write(`${name} weighs ${weight}\n`);
}
```

语言 / efun / apply / stdlib / 概念页上已有的 ` ```c ` 在构建时按 LPC 处理。remark 插件不改 `docs/driver/` 和 `build-wasm`，那些页上的 LPC 请写 ` ```lpc `。真正的 C 仍用 `c` / `cpp`。

新页面请写 ` ```lpc `。

## 3. 在其他 Docusaurus 3 站点里用

插件在驱动仓库里（尚未单独发 npm）。从 fluffos 检出：

1. 注册语言（swizzle `prism-include-languages`，在 `globalThis.Prism` 指向 Prism 实例时 `require`）：

```js
require('/path/to/fluffos/tools/lpc-syntax/prism-lpc.cjs');
```

2. 可选：把遗留的 ` ```c ` 改成 LPC（本站的 `keepCAsC` 含 `docs/driver/` 和 `build-wasm`）。
   那些页上的 LPC 请写 ` ```lpc `，别处真正的 C 请写 ` ```cpp `。

```js
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

不经过 Prism 的 HTML 高亮：

```js
import { highlightLPC, defaultCss } from './tools/lpc-syntax/highlight.mjs';
const html = highlightLPC('int f() { return `v=${x}`; }');
```

## 4. 不用编辑器也能格式化

```bash
testsuite/format.sh            # 格式化 testsuite/**/*.lpc,*.c
testsuite/format.sh --check    # 只检查，CI
find lib -name '*.lpc' | node tools/lpc-syntax/bin/format-corpus.mjs
```

详见 [格式化工具](formatter) 和 [风格指南](style-guide)。

## 5. 给自己的工具用的 efun 元数据

```bash
cd build && ./generate_keywords
```

见 [generate_keywords](../cli/generate_keywords)。

## 6. 建议的 mudlib 工作区

```text
your-mudlib/
  AGENTS.md
  config.cfg
  .vscode/settings.json
```

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

很多 mudlib 仍用 `.c` 当 LPC。把这些文件关联到 LPC 语言，高亮才会生效。

**不要**把这段 `files.associations` 写进 `fluffos/fluffos` 驱动仓库——那里的 `.c` / `.h` 是 C++。只用于 mudlib 工作区。

然后填好 [Mudlib AGENTS.md](../mudlib-agents)。

## 相关

- [从零到一个能连上的 MUD](../start)
- [生态](../ecosystem)
- [从源码构建](../build)
