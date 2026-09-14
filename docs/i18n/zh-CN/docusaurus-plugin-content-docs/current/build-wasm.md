---
title: 构建 WebAssembly 版本
description: 把 FluffOS 交叉编译到浏览器：工具链、打包 mudlib、页面里启动。
---

# 在浏览器里跑 FluffOS（WebAssembly）

FluffOS 可用 [Emscripten](https://emscripten.org) 交叉编译到 WebAssembly：
编译器、VM 和 efun 栈在网页里运行，从内存文件系统启动**你选的** mudlib，
页面本身就是 telnet 客户端。

完整步骤、依赖和限制以英文 [Build for WebAssembly](/build-wasm) 为准。
下面是中文摘要。

> **不想自己编？** 每个 [GitHub Release](https://github.com/fluffos/fluffos/releases)
> 都带 `fluffos-<version>-wasm.zip`（预编译驱动、网页终端、`pack-mudlib.sh`）。
> 解压后从打包 mudlib 那一步开始（打包脚本仍需要 emsdk 在 PATH 上）。

## 先记住

1. **先选 mudlib。** 不要默认打包驱动仓库里的 `testsuite/`，除非你在改引擎。
2. 本机原生开发仍走 [入门](start) / [LLM 约定](llm)：`build/bin/driver` + 该 lib 的配置。
3. WASM 页上的 LPC 示例请用 ` ```lpc ` 围栏（remark 插件不改这一页的 ` ```c `）。

## 流程（详见英文页）

1. 安装 emsdk，并装齐英文页列出的宿主依赖。
2. 按英文页 CMake 预设编出 `build-wasm/src/fluffos.js` + `fluffos.wasm`。
3. 用 `pack-mudlib.sh` 打包**选定的** mudlib（路径、master、端口以该 lib 为准）。
4. 用英文页的静态服务器示例打开网页终端。

架构说明见仓库 [`src/wasm/README.md`](https://github.com/fluffos/fluffos/blob/master/src/wasm/README.md)
和 [WASM cookbook](driver/wasm)。
