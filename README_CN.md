[![CI 状态](https://github.com/fluffos/fluffos/actions/workflows/ci.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![Docker 状态](https://github.com/fluffos/fluffos/actions/workflows/docker-publish.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)

[![Open Collective 支持者](https://opencollective.com/fluffos-579/backers/badge.svg)](#支持者)
[![Open Collective 赞助商](https://opencollective.com/fluffos-579/sponsors/badge.svg)](#赞助商)

欢迎使用 FluffOS
================

FluffOS 是一款高性能**游戏引擎**，专为构建持久性多人虚拟世界而设计。它是 **MudOS** 的现代继任者，也是网络游戏史上最具影响力的引擎之一，目前仍在积极维护。

如果您仍在运行 MudOS，现在是升级的时候了。FluffOS 与现有的 MudOS mudlib **完全向后兼容**，同时新增了十余年的性能优化、现代协议（WebSocket、TLS）、数据库集成（SQLite3、MySQL、PostgreSQL）和 UTF-8 支持。

> **请使用 GitHub 上的最新版本或 `master` 分支。** 旧版（尤其是 v2017）已不再受支持。

---

### 工作原理 — 三层架构

一个 LPMUD 系统由三个层次构成，各司其职：

```
┌─────────────────────────────────────────────────────────┐
│  Mudlib（游戏框架）                                      │
│  一组 LPC 文件 — 定义房间、物品、NPC、战斗、命令、登录。  │
│  （如 Dead Souls、Discworld、Lima、Nightmare）            │
├─────────────────────────────────────────────────────────┤
│  驱动程序（FluffOS）                                     │
│  LPC 虚拟机 + 网络层 + Efun                              │
│  将 LPC 编译为字节码并执行，管理连接                      │
│  （Telnet/WebSocket/TLS）、数据库、I/O。                 │
├─────────────────────────────────────────────────────────┤
│  LPC（Lars Pensjö C）                                   │
│  编程语言。                                              │
│  面向对象、类 C 语法、垃圾回收、                         │
│  热重载、事件驱动。                                      │
└─────────────────────────────────────────────────────────┘
```

| 层次 | 是什么 | 类比 |
|------|--------|------|
| **LPC** | 一种面向对象、类 C 语法的编程语言，专为构建交互式虚拟世界而设计。 | 编程语言（如 C# 或 Lua） |
| **驱动程序** | 包含 LPC 字节码编译器与 VM、网络栈（Telnet、WebSocket、TLS）、数据库驱动（SQLite3、MySQL、PostgreSQL）以及数百个内置函数（**efun**）的 C++ 运行时。这就是 FluffOS。 | 引擎 / 操作系统（如 Unity 或 JVM） |
| **Mudlib** | 一组 LPC 文件，共同构成完整的游戏编程框架——包含房间、物品、玩家、NPC 的基类，命令解析器，登录系统等。一切皆对象，对象之间可以相互交互。 | 框架 / SDK（如 .NET 或游戏模板） |

### 编程语言 — LPC

**LPC**（Lars Pensjö C）是一种专为持久性虚拟世界设计的面向对象语言。游戏世界中的一切都是 LPC 对象。

- **克隆对象** — 一个 LPC 源文件就是蓝图。驱动程序通过克隆来创建实例：一个 `/obj/sword.c` → 世界中数千把独立的剑对象。
- **继承** — 对象可继承并覆盖父类行为：`/std/weapon.c` → `/obj/sword.c` → `/obj/cursed_sword.c`。
- **垃圾回收** — 驱动程序通过引用计数自动管理内存；LPC 代码无需调用 `free()`。
- **热重载** — 可在服务器运行期间更新并重载对象，无需重启或断开玩家连接。
- **事件驱动** — 无线程、无阻塞。游戏逻辑响应玩家命令、网络事件和 `call_out()` 定时器。
- **沙盒执行** — 驱动程序追踪评估消耗，在无限循环等失控代码导致崩溃前将其终止。

FluffOS 在完全兼容 MudOS LPC 方言的基础上，新增了 UTF-8 / EGC 感知字符串操作、引用参数、新类型修饰符和更多 efun，同时**完全向后兼容**现有的 MudOS LPC 代码。

### 驱动程序 — FluffOS

FluffOS 是引擎，位于 LPC 源文件与操作系统之间，提供运行 MUD 所需的一切：

- **LPC 编译器与字节码 VM** — 按需解析并编译 LPC 源码，然后执行字节码。
- **600+ efun**（外部函数）— 可从 LPC 调用的内置 C++ 函数：`clone_object()`、`call_out()`、`write()`、`regexp()`、`crypt()`、`db_exec()` 等数百个。
- **Simul-efun** — mudlib 可以用同名 LPC 函数替换任意 efun，为所有对象透明地扩展语言能力。
- **Applies（回调）** — 驱动程序在生命周期事件时调用对象的标准 LPC 函数：对象创建时调用 `create()`，定时器触发时调用 `heart_beat()`，玩家进入房间时调用 `init()`，垃圾回收时调用 `clean_up()`。
- **网络** — 同时支持 Telnet、WebSocket 和 TLS 连接；IAC 选项协商；MXP/MSP 支持。
- **异步数据库 I/O** — SQLite3、MySQL 和 PostgreSQL 查询在线程池中运行，游戏循环不会阻塞。
- **现代运行时** — jemalloc 分配器、异步 DNS、基于 libevent 的事件循环，跨平台（Linux、macOS、Windows/MSYS2）。

### Mudlib — 您的游戏世界

**Mudlib** 就是游戏本身：一棵 LPC 文件树，定义了房间、物品、NPC、战斗、法术、命令解析器、登录界面以及世界规则。驱动程序启动时加载 mudlib；一个典型 MUD 安装中 `/src/` 以上的所有内容都是 mudlib 代码。

FluffOS 内置了 `testsuite/` 目录下的测试用 mudlib。对于真实游戏，您需要单独选择或构建 mudlib。以下是与 FluffOS 开箱即用的流行选择：

- **[Dead Souls](https://dead-souls.net/)** — 文档完善，对新手友好，工具链现代化。
- **[Lima](https://github.com/Valdaris/lima)** — 成熟、模块化、应用广泛。
- **[Discworld](https://dwwiki.mooo.com/)** — 为长期运营的 Discworld MUD（自 1991 年起）提供支持。
- **[Nightmare](http://mud.seraph.org/)** — 最古老、最具影响力的 mudlib 家族之一。

使用同一个 FluffOS 二进制文件但不同 mudlib 的两个 MUD 可以给玩家完全不同的体验——驱动程序本身不强加任何游戏机制。

### 家族谱系

```
LPMud（Lars Pensjö，1989 年）── 发明了驱动/mudlib 分离架构
    │
    ├── LPC 语言 ────────────────── 衍生出 Pike（1994 年）
    │
    ├── DGD（Felix Croes，1993 年）── 极简主义，基于磁盘的持久化
    │
    ├── LDMud（Lars Düning，1997 年）── 延续原始 LPMud 路线
    │
    └── MudOS（1992 年）
            │
            └── FluffOS ◄── 您在这里
                  完全兼容 MudOS +
                  UTF-8、WebSocket/TLS、异步数据库、jemalloc、
                  600+ efun，积极开发与 CI 持续集成
```

这个家族中的每个驱动程序都共享同一核心理念——LPC 源文件由 C 运行时加载并执行，引擎（驱动）与游戏内容（mudlib）严格分离。不同之处在于范围和哲学。**DGD** 注重极简主义和基于磁盘的持久化。**LDMud** 忠于原始设计。**MudOS** 是"功能齐全"的分支，而 **FluffOS** 延续了这一传统，采用现代 C++、跨平台支持和持续集成。

完整的代码库架构和贡献者指南，请参阅 [AGENTS.md](AGENTS.md)。LPC 文档请访问 [FluffOS 官方文档](https://www.fluffos.info)。

---

构建与安装指南
-------------
FluffOS 支持多个目标平台。请根据您的部署目标选择对应的方案。

| 目标 | 环境 | 二进制类型 |
|------|------|-----------|
| **Linux (Ubuntu/Debian)** | 原生 Linux 或 WSL | Linux ELF |
| **macOS** | 原生 macOS | Mach-O |
| **Windows** | MSYS2 / MinGW64 | Windows PE |
| **Alpine Linux** | Docker 或原生 Alpine | 静态 Linux ELF |

> [!TIP]
> 在 **Windows 主机**上，您有两种选择：
> - 构建 **Windows 原生**二进制文件 → 使用 [MSYS2 / MinGW64](#为-windows-构建msys2) 工作流。
> - 构建 **Linux 原生**二进制文件 → 使用 [WSL](#通过-wsl-构建-linux-版本) 工作流。

---

### 为 Linux 构建（Ubuntu/Debian）

这是主要支持的平台（Ubuntu 24.04 LTS）。

**1. 安装依赖：**
```bash
sudo apt update
sudo apt install -y build-essential autoconf automake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libtool libz-dev telnet libgtest-dev libjemalloc-dev \
  libdw-dev libbz2-dev
```

**2. 编译：**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc) install
```

---

### 为 macOS 构建

**1. 安装依赖（Homebrew）：**
```bash
brew install cmake pkg-config pcre libgcrypt openssl jemalloc icu4c \
  mysql sqlite3 googletest
```

**2. 编译：**
```bash
mkdir build && cd build
OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/opt/homebrew/opt/icu4c" cmake ..
make -j$(nproc) install
```

---

### 为 Windows 构建（MSYS2）

构建 **Windows 原生** FluffOS 二进制文件必须使用 **MSYS2 / MinGW64** 环境。

**1. 安装依赖（在 MINGW64 终端中执行）：**
```bash
pacman --noconfirm -S --needed \
  git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-zlib mingw-w64-x86_64-pcre \
  mingw-w64-x86_64-icu mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-jemalloc mingw-w64-x86_64-gtest \
  bison make
```

**2. 编译：**
```bash
mkdir build && cd build
cmake -G "MSYS Makefiles" -DMARCH_NATIVE=OFF -DPACKAGE_CRYPTO=OFF -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1 ..
make -j$(nproc) install
```

**编辑器设置：** 项目根目录中的 `open-editor-msys2.bat` 脚本可自动检测 MSYS2 安装位置，设置 `MSYS2_ROOT`，并将 MinGW64 添加到 `PATH`，然后启动您选择的 **Antigravity IDE** 或 **VS Code**。直接双击即可运行。

---

### 通过 WSL 构建 Linux 版本

在 Windows 主机上构建 **Linux 原生** FluffOS 二进制文件，请使用 WSL（Windows Subsystem for Linux）。

> [!IMPORTANT]
> **性能**：代码库**必须**位于 WSL 发行版的原生文件系统上（如 `/home/user/fluffos`），**不能**放在映射的 Windows 驱动器（`/mnt/c/...`）上。跨文件系统的 I/O 会严重影响构建速度。

**1. 设置 WSL 并克隆仓库（在 WSL 终端中）：**
```bash
# 如果尚未安装 WSL 发行版（在 PowerShell/cmd 中运行）：
#   wsl --install -d Ubuntu
# 然后打开 WSL 终端，将仓库克隆到原生 Linux 文件系统：
git clone https://github.com/fluffos/fluffos ~/fluffos
```

**2. 安装依赖并编译：** 在 WSL 终端中按照上述 [Ubuntu/Debian](#为-linux-构建ubuntudebian) 步骤操作。

**3. 打开编辑器：** 在 Windows 资源管理器中，通过 WSL UNC 路径导航到项目（如 `\\wsl.localhost\Ubuntu\home\<用户>\fluffos`），双击 **`open-editor-wsl.bat`**。该脚本从 UNC 路径读取发行版名称，并以 `--remote wsl+<distro>` 模式启动您选择的 **Antigravity IDE** 或 **VS Code**。

---

### 为 Alpine Linux 构建（Docker / 静态）

**1. 安装依赖：**
```bash
apk add --no-cache linux-headers gcc g++ clang-dev make cmake bash \
  mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static \
  openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
  pcre-dev bison git musl-dev libelf-static elfutils-dev \
  zstd-static bzip2-static xz-static
```

**2. 编译（静态构建）：**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DMARCH_NATIVE=OFF
make -j$(nproc) install
```

---

### 高级构建选项

以下 CMake 标志可与任意平台组合使用：

| 标志 | 说明 |
|------|------|
| `-DCMAKE_BUILD_TYPE=Debug` | 带符号信息的调试构建 |
| `-DCMAKE_BUILD_TYPE=Release` | 优化的生产构建 |
| `-DENABLE_SANITIZER=ON` | 启用地址与未定义行为 Sanitizer（Clang） |
| `-DSTATIC=ON` | 完全静态二进制文件 |
| `-DMARCH_NATIVE=OFF` | 禁用原生 CPU 优化（提高可移植性） |

---

## 测试 FluffOS

FluffOS 拥有完善的单元测试和 LPC 层面的测试套件。

### 单元测试
在 `build` 目录下使用 GTest 运行标准的二进制级单元测试：
```bash
cd build
make test

# 或者手动执行特定测试
./src/lpc_tests
./src/ofile_tests
```

### LPC 测试与集成
```bash
# 使用测试套件配置运行驱动
./build/bin/driver testsuite/etc/config.test

# 或者直接运行 LPC 测试套件
cd testsuite
../build/bin/driver etc/config.test -ftest
```

> [!NOTE]
> **测试套件贡献指南**：LPC 测试用例均位于 `testsuite/` 目录中。当您添加新的 package efun 或修改现有的驱动程序特性时，应当在 `testsuite/single/tests/efuns/`（或通常的 `testsuite/single/tests/`）下添加对应的 LPC 测试文件，以确保所做更改能通过 CI 工作流的持续验证。

---

## 目录结构与核心组件

- **`src/`**：核心驱动程序源代码。
  - `main.cc`：入口点。
  - `backend.cc`：主游戏循环与网络/事件派发器。
  - `comm.cc`：网络套接字与数据包处理。
  - `user.cc`：连接与会话管理。
- **`src/vm/`**：LPC 执行虚拟机。
  - `interpret.cc`：字节码解释器循环。
  - `simulate.cc`：游戏对象生命周期与模拟函数。
- **`src/compiler/`**：LPC 编译引擎 (`grammar.y`、`lex.cc`、`generate.cc`)。
- **`src/packages/`**：模块化的 efun 功能（数学、数据库、加密、套接字等）。
- **`testsuite/`**：包含 LPC 测试和配置的官方测试套件。
- **`docs/`**：Markdown 文档。

---

## 特性

### LPC 语言与 UTF-8 支持
- LPC 字符串操作支持 UTF-8 EGS，范围操作符支持 emoji 和其他 Unicode 字符。
- 提供了各种新的 EFUN 用于透明的输入/输出转码。

### 驱动运行时特性
- 支持 Jemalloc 以优化内存管理。
- 默认使用 SHA512 加密。
- 支持 LPC 追踪。
- 集成了 MySQL、PostgreSQL、SQLite。
- 异步 I/O 操作。
- 外部程序集成。

### 网络特性
- 支持 TLS。
- 支持 WebSocket 协议（附带一个 Web 客户端的最小示例）。

---

## 支持与社区
- **网站/文档**：[https://www.fluffos.info](https://www.fluffos.info)
- **Discord 支持**：LPC 的 #fluffos 频道 [https://discord.gg/2a6qqG6Cdj](https://discord.gg/2a6qqG6Cdj)
- **论坛**：[https://forum.fluffos.info](https://forum.fluffos.info)
- **QQ 支持群**：451819151 [![451819151](https://pub.idqqimg.com/wpa/images/group.png)](https://shang.qq.com/wpa/qunwpa?idkey=3fd552adb8ace1a8e3ae3a712e5d314c7caf49af8b87449473c595b7e1f1ddf9)

---

## 历史与遗留版本

### V2017 版本
v2017 是旧版，使用 autoconf 构建系统，支持在 CentOS/Ubuntu 和 Windows（使用 CYGWIN）上编译。此版本不再受支持，仅出于历史兴趣保留。

所有之前的 MudOS 和 FluffOS 版本也以标签形式保留在代码库中作为历史参考。

---

捆绑的第三方依赖
----------------------
- libwebsockets: <https://libwebsockets.org/>
- libevent: <https://libwebsockets.org/>
- backward-cpp: <https://github.com/bombela/backward-cpp>
- musl 的 crypt: <https://www.musl-libc.org/>
- ghc::filesystem: <https://github.com/gulrak/filesystem>
- nlohmann::json: <https://github.com/nlohmann/json>
- scope_guard: <https://github.com/Neargye/scope_guard>
- utfcpp: <https://github.com/nemtrif/utfcpp>
- utf8_decoder: <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
- libtelnet，基于 <https://github.com/seanmiddleditch/libtelnet> 并做了本地修改

非捆绑的平台依赖包括：libevent、ICU4C、OpenSSL、Zlib 等。

使用 FluffOS 的项目
----------------------
[添加您自己的项目](https://github.com/fluffos/fluffos/edit/master/README.md)

- 几乎所有中文 MUD
- [ThresholdRPG](https://wiki.thresholdrpg.com/)
- [SWmud](http://www.swmud.org/)
- [Merentha](https://www.merentha.com/)

捐赠
---------
我要特别感谢所有赞助商和贡献者给予的支持。
所有捐赠 100% 用于购买 FluffOS 开发和网站及论坛托管的工具、设备和托管费用。

列表按收到捐赠的时间降序排列。

### 2019 年 1 月收到

- 逍遥山人, qq1102907881
- lostsnow
- 小瓶盖
- 星星 qq 55833173
- 胜华 gon***@126.com

### 2018 年 11 月收到

- felchoin@sjever
- 朝亮 wuc***@163.com
- 羽天邪
- 风清扬 832***@qq.com
- Mok say***@139.com
- 春龙 lon***@gmail.com
- 碎梦 lih***@163.com

## 贡献者

这个项目得以存在要感谢所有贡献者。
<a href="https://github.com/fluffos/fluffos/graphs/contributors"><img src="https://opencollective.com/fluffos-579/contributors.svg?width=890&button=false" /></a>

## 支持者

感谢所有的支持者！🙏 [成为支持者](https://opencollective.com/fluffos-579#backer)

<a href="https://opencollective.com/fluffos-579#backers" target="_blank"><img src="https://opencollective.com/fluffos-579/backers.svg?width=890"></a>

## 赞助商

通过成为赞助商来支持这个项目。您的标志将显示在这里并链接到您的网站。[成为赞助商](https://opencollective.com/fluffos-579#sponsor)

<a href="https://opencollective.com/fluffos-579/sponsor/0/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/0/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/1/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/1/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/2/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/2/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/3/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/3/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/4/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/4/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/5/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/5/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/6/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/6/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/7/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/7/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/8/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/8/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/9/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/9/avatar.svg"></a>