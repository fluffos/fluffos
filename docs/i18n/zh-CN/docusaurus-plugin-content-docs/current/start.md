---
title: 从零到一个能连上的 MUD
slug: /start
---

# 从零到一个能连上的 MUD

从空机器到一个能打字的 FluffOS 进程，走这条最短路径。把同一套步骤交给助手时，请用 **[https://www.fluffos.info/llm](https://www.fluffos.info/llm)**。

## 三层

LPMUD 不是单个程序，永远是三块：

| 层 | 是什么 | 本仓库里 |
|---|---|---|
| **LPC** | 写房间、NPC、命令的语言 | [语言参考](lpc/) |
| **驱动** | 编译器、VM、efun、Telnet / WebSocket / TLS | `src/` — 这就是 FluffOS |
| **Mudlib** | 游戏本身：驱动加载的 LPC 文件树 | 第一份是 `testsuite/` |

驱动不带可上线的游戏世界。`testsuite/` 是一份最小 mudlib（*Lil* 的后裔）加上 LPC 回归套件。它是正确的第一份 MUD：编译引擎、启动真实 mudlib、得到提示符。

要完整游戏时，从 [FluffOS 生态](ecosystem) 选一份 mudlib，再把生成的配置指过去。

## 1. 编译驱动

Ubuntu 22.04+ / Debian / WSL（仓库必须在 Linux 文件系统上，不要放 `/mnt/c/...`）：

```bash
sudo apt update
sudo apt install -y build-essential cmake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libz-dev telnet libjemalloc-dev libicu-dev \
  libgtest-dev pkg-config libffi-dev

git clone https://github.com/fluffos/fluffos.git
cd fluffos
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j"$(nproc)" install
```

安装后的二进制是 `build/bin/driver`。其他平台见 [构建指南](build)。

或拉取镜像：`docker pull ghcr.io/fluffos/fluffos:master`。

## 2. 启动 testsuite

`config.test` 里 `mudlib directory` 是 `./`，相对于**进程当前目录**。必须在 `testsuite/` 下启动：

```bash
cd testsuite
../build/bin/driver etc/config.test
```

进程在前台保持运行，打印版本和 `Execution root:`。不要关掉。

`etc/config.test` 的端口：

| 端口 | 协议 |
|---|---|
| 4000 | Telnet |
| 4001 | WebSocket（同时提供内置网页客户端） |
| 4002 | WebSocket + TLS |
| 4003 | Telnet + TLS |

这些端口被占用时不要再启第二个驱动。

## 3. 连接

**Telnet：**

```bash
telnet localhost 4000
```

应看到 `Welcome to Lil!` 和 `>`。试 `who`、`eval return 2 + 2;`，然后 `quit`。

**浏览器：**打开 [http://127.0.0.1:4001/](http://127.0.0.1:4001/)。驱动在 websocket 端口上提供 `src/www`。见 [WebSocket](concepts/general/websocket)。

## 4. 接下来

1. [LPC 语言](lpc/) — 类型、`foreach`、函数、[async](lpc/constructs/async)
2. [Apply](/apply/) — `create`、`logon`、`connect`、`valid_*`
3. [Efun](/efun/)
4. [概念](/concepts/) · [生态](ecosystem)

失败时见 [排障](bug)。
