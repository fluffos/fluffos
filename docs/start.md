---
title: From zero to a running mud
slug: /start
description: Clone, build, boot the testsuite mudlib, and connect over telnet or a browser.
---

# From zero to a running mud

This is the shortest path from an empty machine to a FluffOS process you
can type into. For a paste-into-an-assistant version of the same path, use
**[https://www.fluffos.info/llm](https://www.fluffos.info/llm)**.

## The three layers

An LPMUD is not a single program. Three pieces always show up:

| Layer | What it is | In this repo |
|---|---|---|
| **LPC** | The language rooms, NPCs, and commands are written in | [Language reference](lpc/) |
| **Driver** | Compiler, VM, efuns, Telnet / WebSocket / TLS | `src/` — this is FluffOS |
| **Mudlib** | The game: a tree of LPC files the driver loads | `testsuite/` is the first one |

The driver does not include a shippable game world. `testsuite/` is a
minimal mudlib (a descendant of *Lil*) plus the LPC regression suite. It
is the right first mud: you compile the engine, boot a real mudlib, and
get a prompt.

When you want a full game, pick a mudlib from the
[FluffOS ecosystem](ecosystem) and point a generated config at it.

## 1. Build the driver

On Ubuntu 22.04+ / Debian / WSL (repo on the Linux filesystem, not `/mnt/c`):

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

The installed binary is `build/bin/driver`. macOS, Windows (MSYS2), Alpine,
and WebAssembly are on the [build guide](build).

Or pull a prebuilt image: `docker pull ghcr.io/fluffos/fluffos:master`.

## 2. Boot the testsuite

`config.test` sets `mudlib directory` to `./`, which is the process
working directory. Start the driver **from `testsuite/`**:

```bash
cd testsuite
../build/bin/driver etc/config.test
```

The process stays in the foreground and prints the version plus
`Execution root:`. Leave it running.

Ports from `etc/config.test`:

| Port | Protocol |
|---|---|
| 4000 | Telnet |
| 4001 | WebSocket (serves the built-in web client) |
| 4002 | WebSocket + TLS |
| 4003 | Telnet + TLS |

Do not start a second driver while these ports are taken.

## 3. Connect

**Telnet:**

```bash
telnet localhost 4000
```

You should see `Welcome to Lil!` and a `>` prompt. Try `who`,
`eval return 2 + 2;`, then `quit`.

**Browser:** open [http://127.0.0.1:4001/](http://127.0.0.1:4001/). The
driver serves `src/www` from the websocket port. See
[WebSocket](concepts/general/websocket).

## 4. What you can type

The motd lists the core verbs. Common ones:

- `who`, `say`, `eval`, `ed`, `update`, `dest`, `rm`, `quit`, `shutdown`
- `tests` — full LPC suite (long)
- `tuidemo` — terminal UI demo

`update`, `dest`, `ed`, and `rm` want full mudlib pathnames
(`/single/master`, not `master.lpc`).

## 5. Where to go next

1. [LPC language](lpc/) — types, `foreach`, functions, [async](lpc/constructs/async)
2. [Applies](/apply/) — `create`, `logon`, `connect`, `valid_*` (the driver calls you)
3. [Efuns](/efun/) — `clone_object`, `call_out`, `write`, …
4. [Concepts](/concepts/) — objects, simul_efuns, [hot reload](concepts/general/hot_reload), [async](concepts/general/async)
5. [Stdlib](/stdlib/) — LPC helpers that live in the testsuite, not in the driver
6. [Runtime config](driver/config) — or `driver --generate-config` for a starter file
7. [Ecosystem](ecosystem) — other `fluffos/*` repos (mudlibs, VS Code, converters)

If the build or the boot fails, see [Troubleshooting](bug).
