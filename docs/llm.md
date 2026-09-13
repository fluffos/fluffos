---
title: LLM onboarding
slug: /llm
description: Point an LLM at this URL to build FluffOS and boot a running mud.
---

# LLM onboarding

**Canonical URL:** [https://www.fluffos.info/llm](https://www.fluffos.info/llm)

This page is the FluffOS onboarding contract for language models and for
anyone who pastes this URL into an assistant. Follow it in order. Do not
invent efuns, apply names, config keys, CMake flags, or mudlib layout.

A machine-readable site map lives at
[https://www.fluffos.info/llms.txt](https://www.fluffos.info/llms.txt).
The human-facing twin of this page is [From zero to a running mud](start).
The full GitHub org map is [the FluffOS ecosystem](ecosystem).

## What you are doing

FluffOS is an LPMUD **driver** (game engine): an LPC compiler + VM + network
server. A **mudlib** is a tree of LPC files that *is* the game. The driver
does not ship a production game world.

```
Mudlib  — LPC files: rooms, objects, login, commands
Driver  — this repo: compiles LPC, runs it, talks Telnet / WebSocket / TLS
LPC     — the language those files are written in
```

**Goal of this page:** clone `fluffos/fluffos` on `master`, build the
driver, boot the bundled `testsuite/` mudlib, and connect. That *is* a
running mud (the historic *Lil* bootstrap plus the driver's LPC test
harness). It is not a full game; it is the correct first mud.

## Success criteria

Stop when all of these are true:

1. `build/bin/driver` exists (after `make install`) or `build/src/driver` exists (incremental build without install).
2. The driver process is running `etc/config.test` from `testsuite/`.
3. A telnet client to `localhost:4000` shows `Welcome to Lil!` and a `>` prompt.
4. Typing `who` and `eval return 1 + 1;` both produce output (do not require `tests` unless asked — that run takes minutes).

If the user only asked to *build*, stop after step 1 and report the binary path.

## Hard rules

- Use branch **`master`** or a `v2026.*` / `v2025.*` release tag. Do **not** use v2017, CYGWIN, or autoconf.
- Run the driver **from `testsuite/`**. `mudlib directory` in `config.test` is `./`, resolved against the process cwd.
- After `cmake && make install`, the binary is **`build/bin/driver`**. Incremental trees without `install` use **`build/src/driver`**. There is no `build/bin/` until install.
- Do **not** invent efuns. If a function is not on [the efun index](/efun/) or in a `src/packages/*/*.spec` file, it does not exist.
- Do **not** start a second driver on the same ports. `config.test` binds **4000** (telnet), **4001** (websocket), **4002** (websocket TLS), **4003** (telnet TLS).
- LPC sources in `testsuite/` use the **`.lpc`** extension. Extension-less names prefer `.lpc`, then `.c`.
- Prefer Ubuntu 22.04+ / Debian (or WSL with the repo on the Linux filesystem, never `/mnt/c/...`).
- When editing the driver C++ itself, also read the repo-root `AGENTS.md`. This page is for *getting a mud running*, not for changing the VM.

## Procedure (Ubuntu / Debian / WSL)

### 1. Dependencies

```bash
sudo apt update
sudo apt install -y build-essential cmake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libz-dev telnet libjemalloc-dev libicu-dev \
  libgtest-dev pkg-config libffi-dev
```

`flex` is only required if you edit `src/compiler/internal/lexer.l`.

### 2. Clone and build

```bash
git clone https://github.com/fluffos/fluffos.git
cd fluffos
git checkout master
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j"$(nproc)" install
```

Expect `bin/driver` under `build/`. Other platforms: [Build from Source](build).
WebAssembly (browser): [Build for WebAssembly](build-wasm).

**Docker-only binary** (still needs a mudlib directory and config):

```bash
docker pull ghcr.io/fluffos/fluffos:master
```

### 3. Boot the testsuite mudlib

From the **repo root**:

```bash
cd testsuite
../build/bin/driver etc/config.test
```

A healthy boot prints the FluffOS version, `Execution root:`, `Initializing internal stuff`, then listens. It does **not** exit.

One-shot LPC suite (no interactive server; minutes long):

```bash
cd testsuite
../build/bin/driver etc/config.test -ftest
```

A clean run prints `Checks succeeded.` and exits 0.

### 4. Connect

**Telnet** (primary):

```bash
telnet localhost 4000
```

You should see `Welcome to Lil!`, the motd, and `>`.

Useful verbs (full pathnames where noted): `who`, `say hello`, `eval return sizeof(({1,2,3}));`, `ed`, `update`, `dest`, `quit`, `shutdown`. `tests` runs the LPC suite from inside the mud.

**Browser / WebSocket:** open `http://127.0.0.1:4001/` — the driver serves `src/www` on the websocket port (`websocket http dir` in `config.test`). The TLS twin is `https://127.0.0.1:4002/` with the self-signed test certs in `testsuite/etc/` (the browser will warn; do not use `http://` on 4002).

### 5. If something fails

| Symptom | Likely cause |
|---|---|
| `Bad mudlib directory` | Cwd is not `testsuite/` |
| `Address already in use` / port bind failure | Another driver (or process) owns 4000–4003 |
| Missing `libicu` / `jemalloc` / `ssl` at cmake time | Install the `-dev` packages above |
| `Welcome to Lil!` never appears | Driver died; read the console and `testsuite/log/debug.log` |
| Telnet connects then immediately closes | Master `connect()` failed; same log |

More: [Troubleshooting](bug). File issues at
[github.com/fluffos/fluffos/issues](https://github.com/fluffos/fluffos/issues)
with the version line the driver printed at start, the full console, and a
minimal LPC repro if you have one.

## After it is running — what to tell the user

1. `testsuite/` is a **reference mudlib**, not a game to ship. For a real world, pick a mudlib from [the ecosystem](ecosystem) (Dead Souls, Lima, Nightmare, or a Chinese lib such as 泥潭 / 侠客行) and point a generated config at it.
2. Learn LPC next: [language index](lpc/), especially [source files](lpc/source-files), [foreach](lpc/constructs/foreach), [async](lpc/constructs/async), and [concepts / async](concepts/general/async).
3. Driver↔LPC contract: [applies](/apply/) (`create`, `logon`, `connect`, `valid_*`) and [efuns](/efun/).
4. Runtime knobs: from the **repo root**, `./build/bin/driver --generate-config > my.cfg` then [config reference](driver/config). Never invent option names; they come from `src/base/internal/rc.cc`.

## Generate a config for a different mudlib

From the **repo root**:

```bash
./build/bin/driver --generate-config > /path/to/mudlib/config.cfg
```

Then edit at least:

- `name`
- `mudlib directory` (absolute path to that mudlib)
- `master file`
- `include directories`
- `external_port_1 : telnet <port>` (the template already emits this; do **not** add a legacy `port number` line next to it — the driver will ignore `external_port_1`)

Boot with cwd or an absolute `mudlib directory` that actually contains the
master object. Do not copy `config.test` blindly onto a third-party mudlib —
port lines, master path, and include paths differ.

## Repo map (this repository)

| Path | Role |
|---|---|
| `src/` | Driver (VM, compiler, network, packages) |
| `src/packages/*/*.spec` | Efun signatures — source of truth |
| `src/vm/internal/applies` | Apply names — source of truth |
| `src/base/internal/rc.cc` | Config keys — source of truth |
| `testsuite/` | First mudlib + LPC regression suite |
| `docs/` | This site (`https://www.fluffos.info`) |
| `tools/lpc-syntax/` | LPC formatter / grammar (Node ≥ 18, no npm install) |
| `src/www/` | Built-in websocket web client |
| `AGENTS.md` | Contributor / coding-agent guide for the C++ driver |

## Ecosystem (other `fluffos/*` repos)

Do not clone these to *boot* the testsuite. Use them when the user wants a
full game, an editor, or a conversion tool. Details: [ecosystem](ecosystem).

| Repo | Use |
|---|---|
| [fluffos/fluffos](https://github.com/fluffos/fluffos) | Driver + docs + testsuite (this project) |
| [fluffos/fluffos-vscode](https://github.com/fluffos/fluffos-vscode) | VS Code / editor extension (pins this repo) |
| [fluffos/dead-souls](https://github.com/fluffos/dead-souls) | Dead Souls mudlib snapshot |
| [fluffos/lima](https://github.com/fluffos/lima) | Lima mudlib snapshot (archived repo; still the org copy) |
| [fluffos/nightmare3](https://github.com/fluffos/nightmare3) | Nightmare 3 mudlib snapshot |
| [fluffos/nt7](https://github.com/fluffos/nt7) | 泥潭 7 UTF-8 |
| [fluffos/xkx100](https://github.com/fluffos/xkx100) | 侠客行 100 UTF-8 |
| [fluffos/sanguozhi](https://github.com/fluffos/sanguozhi) | 三国志 MUD (older driver) |
| [fluffos/mudlibs](https://github.com/fluffos/mudlibs) | Archive of historic Chinese mudlibs |
| [fluffos/lpc-test](https://github.com/fluffos/lpc-test) | Additional LPC test lib |
| [fluffos/gbk2utf8](https://github.com/fluffos/gbk2utf8) | GB2312/GBK/GB18030 ↔ UTF-8 |
| [fluffos/imud](https://github.com/fluffos/imud) | Source for imud.fluffos.info |
| [fluffos/libtelnet](https://github.com/fluffos/libtelnet) | Telnet library (also vendored in the driver) |
| [fluffos/widecharwidth](https://github.com/fluffos/widecharwidth) | wcwidth helper (also vendored) |

Community (not GitHub repos): [forum.fluffos.info](https://forum.fluffos.info),
Discord `#fluffos` on the [LPC server](https://discord.gg/E5ycwE8NCc),
QQ group 451819151.

## Next documents (read only what the task needs)

- [From zero to a running mud](start) — same procedure, written for humans
- [LPC language](lpc/) · [Efuns](/efun/) · [Applies](/apply/) · [Concepts](/concepts/)
- [Stdlib](/stdlib/) (LPC helpers that ship *in the testsuite*, not as efuns)
- [Async / promises](concepts/general/async) · [WebSocket](concepts/general/websocket) · [TLS](concepts/general/tls)
- [WASM cookbook](driver/wasm) · [CLI `driver`](cli/driver)
- [License](license)

If you are an LLM writing LPC: match existing testsuite style (2-space
indent, `foreach`, `#pragma strict_types` where the surrounding file uses
it). Format with `testsuite/format.sh` from the repo root when you touch
`testsuite/**/*.lpc`.
