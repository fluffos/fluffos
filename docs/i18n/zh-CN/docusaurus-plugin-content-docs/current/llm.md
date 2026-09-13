---
title: LLM 入门约定
slug: /llm
---

# LLM 入门约定

**规范 URL：** [https://www.fluffos.info/llm](https://www.fluffos.info/llm)

本页是给语言模型（以及把该 URL 贴给助手的人）的入门约定。按顺序执行。不要臆造 efun、apply、配置键、CMake 开关或 mudlib 目录结构。

机器可读站点地图：[https://www.fluffos.info/llms.txt](https://www.fluffos.info/llms.txt)。
人类版本：[从零到一个能连上的 MUD](start)。
组织仓库地图：[FluffOS 生态](ecosystem)。

英文页是同一约定的完整原文；若中英冲突，以 [英文 /llm](https://www.fluffos.info/llm) 为准。

## 你在做什么

FluffOS 是 LPMUD **驱动**（游戏引擎）。**Mudlib** 是 LPC 文件树，才是游戏。驱动不附带可上线的游戏世界。

**本页目标：** 克隆 `fluffos/fluffos` 的 `master`，编译驱动，启动自带的 `testsuite/` mudlib，并连上。这就是一个在跑的 MUD（历史 *Lil* 引导库 + LPC 测试框架），不是完整游戏，但是正确的第一份 MUD。

## 成功标准

同时满足即可停止：

1. 存在 `build/bin/driver`（`make install` 之后）或 `build/src/driver`（未 install 的增量编译）。
2. 驱动进程正在从 `testsuite/` 运行 `etc/config.test`。
3. `telnet localhost 4000` 出现 `Welcome to Lil!` 和 `>`。
4. 输入 `who` 与 `eval return 1 + 1;` 都有输出（除非用户要求，否则不必跑 `tests`，那要好几分钟）。

若用户只要编译，完成第 1 步并报告二进制路径即可。

## 硬性规则

- 使用 **`master`** 或 `v2026.*` / `v2025.*` 发布标签。**不要**使用 v2017、CYGWIN、autoconf。
- 必须在 **`testsuite/`** 下启动驱动。`config.test` 的 `mudlib directory` 是 `./`，相对进程 cwd。
- `cmake && make install` 之后二进制是 **`build/bin/driver`**。未 install 的树是 **`build/src/driver`**。
- **不要臆造 efun。** 不在 [efun 索引](/efun/) 或 `src/packages/*/*.spec` 里的函数不存在。
- **不要**在同一组端口上再启一个驱动。`config.test` 占用 **4000–4003**。
- `testsuite/` 里的 LPC 使用 **`.lpc`** 扩展名。
- 优先 Ubuntu 22.04+ / Debian（或 WSL，仓库在 Linux 文件系统上，不要 `/mnt/c/...`）。
- 若要改驱动 C++，另读仓库根目录 `AGENTS.md`。本页只负责把 MUD 跑起来。

## 步骤（Ubuntu / Debian / WSL）

### 1. 依赖

```bash
sudo apt update
sudo apt install -y build-essential cmake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libz-dev telnet libjemalloc-dev libicu-dev \
  libgtest-dev pkg-config libffi-dev
```

只有改 `src/compiler/internal/lexer.l` 才需要 `flex`。

### 2. 克隆并编译

```bash
git clone https://github.com/fluffos/fluffos.git
cd fluffos
git checkout master
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j"$(nproc)" install
```

其他平台：[从源码构建](build)。浏览器：[WebAssembly](build-wasm)。

### 3. 启动 testsuite

在**仓库根目录**下：

```bash
cd testsuite
../build/bin/driver etc/config.test
```

正常启动会打印版本、`Execution root:`、`Initializing internal stuff`，然后监听，**不会退出**。

一次性跑 LPC 套件（无交互，耗时数分钟）：

```bash
cd testsuite
../build/bin/driver etc/config.test -ftest
```

干净运行打印 `Checks succeeded.` 并以 0 退出。

### 4. 连接

```bash
telnet localhost 4000
```

应看到 `Welcome to Lil!`、motd 和 `>`。

浏览器：打开 `http://127.0.0.1:4001/`（驱动在 websocket 端口提供 `src/www`）。TLS 是 `https://127.0.0.1:4002/`（自签名测试证书，不要对 4002 用 `http://`）。

### 5. 失败时

| 现象 | 常见原因 |
|---|---|
| `Bad mudlib directory` | 当前目录不是 `testsuite/` |
| `Address already in use` | 4000–4003 已被占用 |
| cmake 找不到 icu / jemalloc / ssl | 先装上面的 `-dev` 包 |
| 从未出现 `Welcome to Lil!` | 驱动已退出；看控制台和 `testsuite/log/debug.log` |

更多：[排障](bug)。

## 生态（其他 `fluffos/*` 仓库）

启动 testsuite **不必**克隆这些。用户要完整游戏、编辑器或转码时再用。细节见 [生态](ecosystem)。

| 仓库 | 用途 |
|---|---|
| [fluffos/fluffos](https://github.com/fluffos/fluffos) | 驱动 + 文档 + testsuite |
| [fluffos/fluffos-vscode](https://github.com/fluffos/fluffos-vscode) | VS Code 扩展 |
| [fluffos/dead-souls](https://github.com/fluffos/dead-souls) | Dead Souls mudlib |
| [fluffos/lima](https://github.com/fluffos/lima) | Lima（仓库已 archived） |
| [fluffos/nightmare3](https://github.com/fluffos/nightmare3) | Nightmare 3 |
| [fluffos/nt7](https://github.com/fluffos/nt7) | 泥潭 7 UTF-8 |
| [fluffos/xkx100](https://github.com/fluffos/xkx100) | 侠客行 100 UTF-8 |
| [fluffos/sanguozhi](https://github.com/fluffos/sanguozhi) | 三国志（较旧驱动） |
| [fluffos/mudlibs](https://github.com/fluffos/mudlibs) | 历史中文 mudlib 归档 |
| [fluffos/gbk2utf8](https://github.com/fluffos/gbk2utf8) | GBK/GB18030 ↔ UTF-8 |
| [fluffos/lpc-test](https://github.com/fluffos/lpc-test) | 额外 LPC 测试库 |
| [fluffos/imud](https://github.com/fluffos/imud) | imud.fluffos.info |
| [fluffos/libtelnet](https://github.com/fluffos/libtelnet) | Telnet 库（驱动内也有一份） |
| [fluffos/widecharwidth](https://github.com/fluffos/widecharwidth) | wcwidth（驱动内也有一份） |

社区：[forum.fluffos.info](https://forum.fluffos.info)、LPC Discord `#fluffos`、QQ 群 451819151。
