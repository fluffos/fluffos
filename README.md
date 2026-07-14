[![CI Status](https://github.com/fluffos/fluffos/actions/workflows/ci.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![Docker Status](https://github.com/fluffos/fluffos/actions/workflows/docker-publish.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)


[![Backers on Open Collective](https://opencollective.com/fluffos-579/backers/badge.svg)](#backers)
[![Sponsors on Open Collective](https://opencollective.com/fluffos-579/sponsors/badge.svg)](#sponsors)

Welcome to FluffOS
==================

FluffOS is a high-performance **game engine** for building persistent, multiplayer virtual worlds. It is the modern, actively maintained successor to **MudOS** — one of the most influential engines in the history of online gaming.

If you are still running MudOS, it is time to upgrade. FluffOS is fully backward-compatible with existing MudOS mudlibs and adds over a decade of performance optimizations, modern protocols (WebSockets, TLS), database integrations (SQLite3, MySQL, PostgreSQL), and UTF-8 support.

> **Use the latest release on GitHub or the `master` branch.** Legacy versions (v2017 in particular) are no longer supported.

---

### How It Works — Three Layers

An LPMUD system is built from three distinct layers, each with a clear responsibility:

```
┌─────────────────────────────────────────────────────────┐
│  Mudlib                                                 │
│  A distribution of LPC files — the game framework.      │
│  Defines rooms, items, NPCs, combat, commands, login.   │
│  (e.g. Dead Souls, Discworld, Lima, Nightmare)          │
├─────────────────────────────────────────────────────────┤
│  Driver  (FluffOS)                                      │
│  LPC Virtual Machine + Networking + Efuns               │
│  Compiles LPC to bytecode, executes it, manages         │
│  connections (Telnet/WebSocket/TLS), databases, I/O.    │
├─────────────────────────────────────────────────────────┤
│  LPC  (Lars Pensjö C)                                   │
│  The programming language.                              │
│  Object-oriented, C-like, garbage-collected,            │
│  hot-reloadable, event-driven.                          │
└─────────────────────────────────────────────────────────┘
```

| Layer | What it is | Analogy |
|-------|-----------|---------|
| **LPC** | An object-oriented, C-like programming language designed for building interactive virtual worlds. | The programming language (like C# or Lua) |
| **The Driver** | A C++ runtime that includes the LPC bytecode compiler and VM, the network stack (Telnet, WebSocket, TLS), database drivers (SQLite3, MySQL, PostgreSQL), and hundreds of built-in functions (**efuns**) exposed to LPC code. This is FluffOS. | The engine / OS (like Unity or the JVM) |
| **The Mudlib** | A distribution of LPC files that together form a complete game programming framework — base classes for rooms, objects, players, NPCs, a command parser, a login system, and more. Everything is an object and everything can interact with everything else. | The framework / SDK (like .NET or a game template) |

### The Language — LPC

**LPC** (Lars Pensjö C) is an object-oriented, C-like language purpose-built for persistent virtual worlds. Everything in the game world is an LPC object.

- **Clone-based objects** — an LPC source file is a blueprint. The driver clones it to create instances: one `/obj/sword.c` file → thousands of individual sword objects in the world.
- **Inheritance** — objects inherit and override behavior: `/std/weapon.c` → `/obj/sword.c` → `/obj/cursed_sword.c`.
- **Garbage collected** — the driver handles memory automatically via reference counting; LPC code never calls `free()`.
- **Live hot-reload** — `recompile_object()` swaps a recompiled program into a running object *and all its clones* without restarting the server or disconnecting players; variables carry over by name, so state survives the update.
- **Event-driven** — no threads, no blocking. Game logic runs in response to player commands, network events, and `call_out()` timers.
- **Sandboxed** — the driver tracks evaluation cost and kills runaway code before it can crash the server.

FluffOS extends the original MudOS LPC dialect with UTF-8 / EGC-aware string operations, reference parameters, new type modifiers, and additional efuns — while remaining **fully backward-compatible** with existing MudOS LPC code.

### The Driver — FluffOS

FluffOS is the engine. It sits between your LPC source files and the operating system and provides everything needed to run a MUD:

- **LPC compiler & bytecode VM** — parses and compiles LPC source on demand, then executes the bytecode.
- **600+ efuns** (external functions) — built-in C++ functions callable from LPC: `clone_object()`, `call_out()`, `write()`, `regexp()`, `crypt()`, `db_exec()`, and hundreds more.
- **Simul-efuns** — your mudlib can wrap or replace any efun with an LPC function of the same name, transparently extending the language for all objects.
- **Applies** — the driver calls standard LPC functions on objects at lifecycle events: `create()` when an object is born, `heart_beat()` on a timer, `init()` when a player enters a room, `clean_up()` during garbage collection.
- **Networking** — simultaneous Telnet, WebSocket, and TLS connections; IAC option negotiation; MXP/MSP support.
- **Async database I/O** — SQLite3, MySQL, and PostgreSQL queries run in a thread pool so the game loop never blocks.
- **Modern runtime** — jemalloc allocator, async DNS, event loop based on libevent, cross-platform (Linux, macOS, Windows/MSYS2).
- **Runs in the browser** — the driver cross-compiles to WebAssembly: a full mudlib boots inside a webpage (the page is the telnet client), with an LPC ↔ JavaScript bridge for `fetch()`, canvas/WebGL and page-driven UIs. See [Building for WebAssembly](#building-for-webassembly-browser).

### The Mudlib — Your Game World

A **mudlib** is the game itself: a tree of LPC files that defines rooms, items, NPCs, combat, spells, the command parser, the login screen, and the rules of the world. The driver loads the mudlib at startup; everything above `/src/` in a typical MUD installation is mudlib code.

FluffOS ships with a built-in testsuite mudlib under `testsuite/`. For real games, you choose or build a mudlib separately. Popular choices that work with FluffOS out of the box include:

- **[Dead Souls](https://dead-souls.net/)** — well-documented, beginner-friendly, modern tooling.
- **[Lima](https://github.com/Valdaris/lima)** — mature, modular, widely used.
- **[Discworld](https://dwwiki.mooo.com/)** — powers the long-running Discworld MUD (since 1991).
- **[Nightmare](http://mud.seraph.org/)** — one of the oldest and most influential mudlib families.

Two MUDs sharing the same FluffOS binary but different mudlibs can feel entirely different — the driver imposes no game mechanics.

### The Family Tree

```
LPMud (Lars Pensjö, 1989) ── invented the driver/mudlib split
    │
    ├── LPC language ──────────────── also spawned Pike (1994)
    │
    ├── DGD (Felix Croes, 1993) ───── minimalist, disk-based persistence
    │
    ├── LDMud (Lars Düning, 1997) ─── continues the original LPMud line
    │
    └── MudOS (1992)
            │
            └── FluffOS ◄── you are here
                  Full MudOS backward compatibility +
                  UTF-8, WebSocket/TLS, async DB, jemalloc,
                  600+ efuns, active development & CI
```

Every driver in this family shares the same core idea — LPC source files loaded and executed by a C runtime, with a hard separation between the engine (driver) and the game content (mudlib). What differs is scope and philosophy. **DGD** prioritises minimalism and disk-based persistence. **LDMud** stays close to the original design. **MudOS** was the "batteries-included" branch, and **FluffOS** carries that tradition forward with modern C++, cross-platform support, and continuous integration.

For the full codebase architecture and contributor guide, see [AGENTS.md](AGENTS.md). For LPC documentation, visit the [Official FluffOS Documentation](https://www.fluffos.info).


Build & Setup Guide
-------------------
FluffOS supports multiple target platforms. Choose the one that matches your deployment target and follow the steps below.

| Target | Environment | Binary Type |
|--------|-------------|-------------|
| **Linux (Ubuntu/Debian)** | Native Linux or WSL | Linux ELF |
| **macOS** | Native macOS | Mach-O |
| **Windows** | MSYS2 / MinGW64 | Windows PE |
| **Alpine Linux** | Docker or native Alpine | Static Linux ELF |

> [!TIP]
> On a **Windows host**, you have two choices:
> - Build a **Windows-native** binary → use the [MSYS2 / MinGW64](#building-for-windows-msys2) workflow.
> - Build a **Linux-native** binary → use the [WSL](#building-for-linux-via-wsl) workflow.

---

### Building for Linux (Ubuntu/Debian)

This is the primary supported platform (Ubuntu 24.04 LTS).

**1. Install dependencies:**
```bash
sudo apt update
sudo apt install -y build-essential autoconf automake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libtool libz-dev telnet libgtest-dev libjemalloc-dev \
  pkg-config libffi-dev libdw-dev libbz2-dev
```

> [!NOTE]
> `flex` is only needed if you modify the LPC lexer (`src/compiler/internal/lexer.l`). Otherwise the build uses the pre-committed generated lexer.

**2. Compile:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc) install
```

---

### Building for macOS

**1. Install dependencies (Homebrew):**
```bash
brew install cmake pkg-config pcre libgcrypt openssl jemalloc icu4c \
  mysql sqlite3 googletest libffi
```

**2. Compile:**
```bash
mkdir build && cd build
OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/opt/homebrew/opt/icu4c" cmake ..
make -j$(nproc) install
```

---

### Building for Windows (MSYS2)

To build a **Windows-native** FluffOS binary, you must use the **MSYS2 / MinGW64** environment.

**1. Install dependencies (from a MINGW64 shell):**
```bash
pacman --noconfirm -S --needed \
  git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-zlib mingw-w64-x86_64-pcre \
  mingw-w64-x86_64-icu mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-jemalloc mingw-w64-x86_64-gtest \
  mingw-w64-x86_64-pkgconf mingw-w64-x86_64-libffi \
  bison make
```

**2. Compile:**
```bash
mkdir build && cd build
cmake -G "MSYS Makefiles" -DMARCH_NATIVE=OFF -DPACKAGE_CRYPTO=OFF -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1 ..
make -j$(nproc) install
```

**Editor setup:** The `open-editor-msys2.bat` script (in the project root) auto-detects your MSYS2 installation, sets `MSYS2_ROOT`, prepends MinGW64 to `PATH`, and launches your choice of **Antigravity IDE** or **VS Code** with the correct environment. Simply double-click it from the project root.

---

### Building for Linux via WSL

To build a **Linux-native** FluffOS binary from a Windows host, use WSL (Windows Subsystem for Linux). FluffOS compiles natively under WSL distributions such as Ubuntu and Alpine using the standard Linux build toolchain — not MSYS2.

> [!IMPORTANT]
> **Performance**: The codebase **must** reside on the WSL distribution's native filesystem (e.g. `/home/user/fluffos`), **not** on a mapped Windows drive (`/mnt/c/...`). Cross-filesystem I/O across the Windows/Linux boundary is extremely slow and will significantly degrade build times.

**1. Set up WSL and clone the repo (inside your WSL terminal):**
```bash
# Install a WSL distribution if you haven't already (run in PowerShell/cmd):
#   wsl --install -d Ubuntu
# Then open a WSL terminal and clone into the native Linux filesystem:
git clone https://github.com/fluffos/fluffos ~/fluffos
```

**2. Install dependencies and compile:** Follow the [Ubuntu/Debian](#building-for-linux-ubuntudebian) or Alpine instructions inside your WSL terminal.

**3. Open the editor:** In Windows Explorer, navigate to the project via its WSL UNC path (e.g. `\\wsl.localhost\Ubuntu\home\<user>\fluffos`) and double-click **`open-editor-wsl.bat`**. The script reads the distro from the UNC path and launches your choice of **Antigravity IDE** or **VS Code** connected to that WSL distribution.

---

### Building for Alpine Linux (Docker / Static)

**1. Install dependencies:**
```bash
apk add --no-cache linux-headers gcc g++ clang-dev make cmake bash \
  mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static \
  openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
  pcre-dev bison git musl-dev libelf-static elfutils-dev \
  pkgconf libffi-dev zstd-static bzip2-static xz-static
```

**2. Compile (static build):**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DMARCH_NATIVE=OFF
make -j$(nproc) install
```

---

### Building for WebAssembly (Browser)

The driver cross-compiles with Emscripten and runs a full mudlib inside a
webpage — every visitor gets their own driver instance, served as static
files:

```bash
tools/wasm/build-deps.sh     # once: cross-build ICU for wasm32
tools/wasm/build.sh          # native codegen tools + wasm driver + demo bundle
python3 -m http.server -d build-wasm/dist 8080   # open http://localhost:8080/
```

Package your own mudlib with `tools/wasm/pack-mudlib.sh --mudlib <dir>
--config <path>`. Full workflow: [docs/build-wasm.md](docs/build-wasm.md);
recipes (playable demos on GitHub Pages, page UIs calling LPC via the
`jsbridge` efuns, browser `fetch()`/canvas from LPC):
[docs/driver/wasm.md](docs/driver/wasm.md); architecture:
[src/wasm/README.md](src/wasm/README.md).

---

### Building in VS Code

Once the editor is open (via `open-editor-msys2.bat` or `open-editor-wsl.bat`), the CMake Tools extension handles configuration automatically. Use the **Tasks** menu (`Terminal → Run Task`) or `Ctrl+Shift+B` for the default build:

| Task | What it does |
|------|-------------|
| `driver: build` | Incremental build — recompiles only changed files *(default build task)* |
| `driver: clean rebuild` | Deletes all build artifacts, then rebuilds from scratch |
| `driver: testsuite` | Starts the driver with the LPC testsuite (`etc/config.test`) |
| `driver: autotest` | Runs the full automated LPC test suite and exits with a pass/fail code |

> [!TIP]
> On first open, CMake Tools will prompt to select a kit. Choose **GCC (MSYS2 MinGW64)** for Windows or the system GCC/Clang for WSL/Linux/macOS.

---

### Advanced Build Options

These CMake flags can be combined with any platform above:

| Flag | Description |
|------|-------------|
| `-DCMAKE_BUILD_TYPE=Debug` | Debug build with symbols |
| `-DCMAKE_BUILD_TYPE=Release` | Optimized production build |
| `-DENABLE_SANITIZER=ON` | Enable Address & UB sanitizers (Clang) |
| `-DSTATIC=ON` | Fully static binary |
| `-DMARCH_NATIVE=OFF` | Disable native CPU optimizations (for portability) |

-----

## Testing FluffOS

FluffOS has extensive unit tests and LPC-level test suites.

### Unit Tests
Run standard binary-level unit tests using GTest (compiled in the `build` directory):
```bash
cd build
make test

# Or execute specific tests manually
./src/lpc_tests
./src/ofile_tests
```

### LPC Tests & Integration
```bash
# Run the driver with the testsuite configuration
./build/bin/driver testsuite/etc/config.test

# Or run the LPC test suite directly
cd testsuite
../build/bin/driver etc/config.test -ftest
```

> [!NOTE]
> **Test Suite Contributions**: The LPC test cases are located in the `testsuite/` directory. When adding new package efuns or editing existing driver features, contributors should add relevant LPC test files under `testsuite/single/tests/efuns/` (or `testsuite/single/tests/` generally) to ensure the changes are continuously verified by the CI workflows.

### Memory-Safety Work

The driver processes untrusted input (mudlib code, network bytes, save files), so memory-safety review matters. For any change touching C strings, buffers, arrays, offsets, or reference counts:

* Build **both** a Debug + sanitizer tree (`-DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON`) and a `RelWithDebInfo` tree, and run the LPC suite 2–3× in each (it randomizes file order) — some defects are release-only, some only trip ASan/UBSan, and the Debug build's per-file ref-count checker (`Bad ref count …`) is a hard gate.
* Add a regression test that demonstrably fails on the *unfixed* binary.
* **[AGENTS.md](AGENTS.md) §13 is the memory-safety audit checklist** (the recurring bug classes — unbounded copies, integer overflow before alloc, `INT_MIN / -1`, unbounded recursion on nested data, tainted format strings, `error()`-path leaks, off-graph/cross-thread refs); §3 and §4 cover reference counting, the debug ref-count checker, and stack-unwinding safety in detail.

---

## Directory Structure & Core Components

- **`src/`**: Core driver source code.
  - `main.cc`: Entry point.
  - `backend.cc`: The tick/event queues (event-loop-agnostic core); `backend_libevent.cc` is the native blocking loop.
  - `comm.cc`: Transport-agnostic connection handling (users, commands, prompts).
  - `user.cc`: Connection & session management.
- **`src/net/`**: The byte-transport layer — `transport.h` (per-connection interface), `transport_libevent.cc` (sockets/TLS/websockets + listeners), telnet protocol handling.
- **`src/wasm/`**: WebAssembly target — JS-bridged transport, host-driven event loop, exported entry points (see [src/wasm/README.md](src/wasm/README.md)).
- **`src/vm/`**: LPC execution virtual machine.
  - `interpret.cc`: Bytecode interpreter loop.
  - `simulate.cc`: Game object lifecycle and simulation functions.
- **`src/compiler/`**: LPC parsing engine (`grammar.y`, `lex.cc`, `generate.cc`).
- **`src/packages/`**: Modular efun features (math, db, crypto, sockets, jsbridge, etc.).
- **`src/thirdparty/`**: Vendored third-party libraries (libwebsockets, libevent, fmt, nlohmann/json, backward-cpp, …), each a byte-exact copy of an upstream release tag plus a small set of documented FluffOS-local patches; unused test/example/doc trees are pruned. Update one dependency per commit, vendoring from a `git clone` of the upstream tag and re-applying the local patches — the full workflow, the local-patch inventory, and the platform traps (clang configure probes, musl, MinGW runtime DLLs) are documented in [AGENTS.md §14](AGENTS.md).
- **`tools/wasm/`**: WebAssembly tooling — dependency cross-build, end-to-end build, mudlib packer, node testsuite runner.
- **`testsuite/`**: Official testsuite containing LPC tests and configurations.
- **`docs/`**: Documentation site (Markdown, built with Docusaurus — see [docs/README.md](docs/README.md)).

The committed Bison/Flex outputs (`src/compiler/internal/*.autogen.*`) are pinned to the generator versions recorded inside them: hosts with older bison/flex build from the committed copies, hosts at or above the pin regenerate, and the copy-back into the source tree only happens when `grammar.y`/`lexer.l` actually changed (tracked by an input-hash stamp) — so local builds never churn these files.

---

## Features

### LPC Language & UTF-8 Support
- LPC string operations are UTF-8 EGS aware, range operator supports emoji and other unicode characters.
- Various new EFUNS for transparent input/output transcoding.
- Pass-by-reference parameters with the `ref` keyword — or its shorthand `&` — in parameter declarations, call arguments, and `foreach` loops; see the [ref guide](https://www.fluffos.info/lpc/constructs/ref).
- Buffers behave like byte arrays: `foreach` iteration (each byte an int 0–255, `ref` mutates in place), forward/reverse/open-ended indexing, range reads and assignments, `+`/`+=` concatenation, and range-checked byte writes (a value outside 0–255 errors instead of truncating). Strings (as raw UTF-8 bytes) and arrays of ints 0–255 promote to buffers in `=`, `+=`, `+`, and range assignments — explicitly via the `to_buffer()` efun.

### Driver Runtime
- Jemalloc support for optimized memory management.
- SHA512 crypt by default.
- LPC Tracing.
- MySQL, PostgreSQL, SQLite integration.
- Async IO operations.
- External program integration.

### Hot Reload
- `recompile_object()` efun: recompile a source file and swap the new program into the live master copy and every clone — no destruct, object identity and variable state preserved (works for the master object, the simul_efun object, and virtual objects too).
- Compile-time master applies `inherit_program()` and `include_file()` expose the full dependency graph (and can redirect, synthesize, or deny inherits/includes).
- A reference auto-hot-reload daemon (watch files, reload on change, dependency-ordered) ships in the testsuite; see the [hot reload guide](https://www.fluffos.info/concepts/general/hot_reload).

### Networking
- TLS support.
- WebSocket protocol support (with a minimal example for a webclient).

### WebAssembly
- The whole driver runs in a browser page (or node) — compiler, VM, efuns, telnet.
- Mudlibs ship as static bundles via `tools/wasm/pack-mudlib.sh`; no server required.
- `jsbridge` efuns: LPC calls page JavaScript (`js_eval`, `js_call` — fetch, canvas/WebGL, audio, storage) and pages call LPC (`js_export` + `fluffos.callLPC`).
- The LPC testsuite runs inside the wasm driver under node, gated in CI.

---

## Support & Community
- **Website / Documentation**: [https://www.fluffos.info](https://www.fluffos.info)
- **Discord Support**: #fluffos channel on LPC [https://discord.gg/E5ycwE8NCc](https://discord.gg/E5ycwE8NCc)
- **Forum**: [https://forum.fluffos.info](https://forum.fluffos.info)
- **QQ Support Group**: 451819151 [![451819151](https://pub.idqqimg.com/wpa/images/group.png)](https://shang.qq.com/wpa/qunwpa?idkey=3fd552adb8ace1a8e3ae3a712e5d314c7caf49af8b87449473c595b7e1f1ddf9)

---

## History & Legacy Versions

### V2017
v2017 is the legacy version, with an autoconf based build system, it supports compiling on CentOS/Ubuntu and under Windows using CYGWIN. This release is no longer supported; it is kept only for historical interest now.

All previous MudOS and FluffOS releases are also kept in the codebase as tags for historical reference.

---

Bundled Third-party Dependencies
----------------------
- libwebsockets: <https://libwebsockets.org/>
- libevent: <https://libwebsockets.org/>
- backward-cpp <https://github.com/bombela/backward-cpp>
- crypt from musl: <https://www.musl-libc.org/>
- ghc::filesystem <https://github.com/gulrak/filesystem>
- nlohmann::json <https://github.com/nlohmann/json>
- scope_guard <https://github.com/Neargye/scope_guard>
- utfcpp <https://github.com/nemtrif/utfcpp>
- utf8_decoder <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
- libtelnet, based on <https://github.com/seanmiddleditch/libtelnet> with local modifications

Non-bundled platform dependencies includes: libevent, ICU4C, OpenSSL, Zlib etc.

Projects Using FluffOS
----------------------
[Add Your Own](https://github.com/fluffos/fluffos/edit/master/README.md)

- Practically all Chinese
- [ThresholdRPG](https://wiki.thresholdrpg.com/)
- [SWmud](http://www.swmud.org/)
- [Merentha](https://www.merentha.com/)
- [Reinos de Leyenda](https://www.reinosdeleyenda.es)

Donations
---------
I would like to personal thank all the sponsors and contributors for showing their support.
All donations are 100% used towards purchasing tools, equipments and hosting cost for FluffOS development and website
 and forum hosting.

The list is in descending order by time donation received.

##### Received in 2019 Jan

- 逍遥山人, qq1102907881
- lostsnow
- 小瓶盖
- 星星 qq 55833173
- 胜华 gon***@126.com

#### Received in 2018 Nov

- felchoin@sjever
- 朝亮 wuc***@163.com
- 羽天邪
- 风清扬 832***@qq.com
- Mok say***@139.com
- 春龙 lon***@gmail.com
- 碎梦 lih***@163.com

## Contributors

This project exists thanks to all the people who contribute.
<a href="https://github.com/fluffos/fluffos/graphs/contributors"><img src="https://opencollective.com/fluffos-579/contributors.svg?width=890&button=false" /></a>

## Backers

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/fluffos-579#backer)]

<a href="https://opencollective.com/fluffos-579#backers" target="_blank"><img src="https://opencollective.com/fluffos-579/backers.svg?width=890"></a>

## Sponsors

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/fluffos-579#sponsor)]

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
