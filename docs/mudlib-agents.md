---
title: Mudlib AGENTS.md
slug: /mudlib-agents
description: Starter AGENTS.md for a FluffOS mudlib. Copy this into the mudlib root and fill in the blanks.
---

# Mudlib `AGENTS.md`

This page is a **template**. After the user picks a mudlib, copy it to
that tree’s root as `AGENTS.md` (or merge it if one already exists). Fill
every `TODO` from that lib’s README and config. Future assistants should
read the **mudlib** `AGENTS.md` first, then
[https://www.fluffos.info/llm](https://www.fluffos.info/llm).

Do **not** paste the driver-repo `AGENTS.md` from `fluffos/fluffos` into a
mudlib. That file is for C++ / VM work.

---

```markdown
# AGENTS.md — TODO_MUDLIB_NAME

This tree is a **mudlib** (the game): LPC rooms, objects, commands, login.
The **driver** is a separate program ([FluffOS](https://github.com/fluffos/fluffos)).
Do not clone the driver into this directory. Do not invent efuns.

Onboarding contract: https://www.fluffos.info/llm
Efuns: https://www.fluffos.info/efun/
Applies: https://www.fluffos.info/apply/
LPC: https://www.fluffos.info/lpc/
Config keys: https://www.fluffos.info/driver/config

## Identity

- **Name:** TODO
- **Clone:** TODO (git URL or local path)
- **Language / encoding of game text:** TODO (UTF-8 / GBK / mixed)
- **Last known driver:** TODO (FluffOS master / v2019 / older)
- **Config file:** TODO (e.g. `config.fluffos`, `config.ini`, `config.deadsouls`)
- **Master object:** TODO (e.g. `/secure/daemon/master` — from the config `master file` line)
- **Include directories:** TODO (from the config)
- **Simul-efun file:** TODO or none
- **Ports:** TODO (telnet / websocket / TLS)

## How to boot

From the mudlib root (or the directory the config’s `mudlib directory` expects):

    /path/to/fluffos/build/bin/driver CONFIG_FILE

`mudlib directory` of `./` means the process **cwd** is the mudlib root.
Use an absolute `mudlib directory` if you start the driver from elsewhere.

Connect: `telnet 127.0.0.1 PORT` and/or the websocket URL the README names.

Do not reuse `fluffos/fluffos/testsuite/etc/config.test` on this tree.

## Admin / first login

TODO — how to create a wizard (AUTO_WIZ, groups.cfg, hardcoded admin id).
If this lib came from fluffos/mudlibs with the seeded account, that password
is a published default: change it before binding a public address.

## Layout (fill from the real tree)

- `TODO/` — master / secure / daemons
- `TODO/` — commands (player vs wizard)
- `TODO/` — standard objects (room, living, item)
- `TODO/` — domains / areas
- `TODO/` — include headers
- `TODO/` — save data (do not commit player saves unless the lib expects it)

LPC extensions in this tree: `.c` / `.lpc` / both (check before creating files).

## Hard rules for assistants

1. **Stop and ask** before rewriting combat, currency, or login. Match the
   surrounding style (indent, naming, Chinese vs English identifiers).
2. **Do not invent efuns.** If it is not on https://www.fluffos.info/efun/
   or in the driver `src/packages/*/*.spec`, it does not exist. Prefer
   existing simul_efuns and lib helpers.
3. **Applies** (`create`, `init`, `reset`, `logon`, `connect`, `valid_*`)
   are driver callbacks — signatures are at https://www.fluffos.info/apply/.
   Do not make applies `async`.
4. **Do not** run the driver’s `testsuite/` unless the user is changing
   FluffOS itself.
5. Prefer a **minimal LPC repro** in this mudlib over editing driver C++.
6. Encoding: write UTF-8 source unless this lib is still GBK. For a GBK
   tree, convert with [fluffos/gbk2utf8](https://github.com/fluffos/gbk2utf8)
   only if the user asked to convert.
7. After adding LPC, compile by booting or by loading the object in-game
   (`update` / the lib’s equivalent). A file that never loads is not done.

## Driver binary

- Incremental (no `make install`): `fluffos/build/src/driver`
- After `make install`: `fluffos/build/bin/driver`
- Docker: `ghcr.io/fluffos/fluffos:master` — still needs this mudlib + config
```

After filling the TODOs, delete this wrapper heading if you are editing
the copied file in the mudlib. Keep the filled `AGENTS.md` committed so
the next session does not rediscover ports and the master path.
