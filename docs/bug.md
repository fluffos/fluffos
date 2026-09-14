---
title: Troubleshooting
---

# Troubleshooting

This page covers build and boot failures, then how to file a useful crash
report. To get a mud running, pick a mudlib first:
[From zero to a running mud](start) or the [LLM contract](llm).

## Build failures

**CMake cannot find ICU, OpenSSL, PCRE, jemalloc, MySQL, SQLite, or PostgreSQL.**
Install the matching `-dev` / Homebrew / MSYS2 packages from the
[build guide](build). Re-run CMake from a clean `build/` after installing.

**CMake is too old.** FluffOS needs CMake 3.22+. Ubuntu 22.04+ is fine.

**WSL build is extremely slow.** The tree must live on the Linux filesystem
(`~/fluffos`), not `/mnt/c/...`.

**Windows crypto / MySQL errors.** The documented MSYS2 flags disable those
packages: `-DPACKAGE_CRYPTO=OFF -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1`.

**`flex` missing.** You only need flex if you edit `src/compiler/internal/lexer.l`.
Otherwise the committed generated lexer is used.

## Boot and connection

These apply to **the mudlib you picked**. Do not switch to `testsuite/` to
“fix” a boot problem unless you are changing the driver.

**`Bad mudlib directory`.** The process cwd does not match `mudlib directory`.
If the config says `./`, start the driver from that lib’s root:

```bash
cd /path/to/mudlib
/path/to/fluffos/build/bin/driver CONFIG_FILE
```

**`Address already in use`.** Another process owns the port that lib’s config
binds. Stop it; do not start two drivers.

**Driver prints a version then exits.** Read the console and **that lib’s**
log directory. A missing master object, a compile error in the master, or a
bad `include directories` line will abort startup.

**Telnet connects and drops.** `master::connect()` failed while cloning the
login object. Same log.

**Browser page is empty.** Confirm you opened the websocket / HTTP port the
lib documents, not a telnet-only port.

### If you are running the driver testsuite

`testsuite/etc/config.test` uses `mudlib directory : ./` and ports 4000–4003:

```bash
cd testsuite
../build/bin/driver etc/config.test
```

Logs: `testsuite/log/debug.log`. Built-in web client: port 4001, with
`websocket http dir` pointing at `../src/www`.

## Driver vs sanitizers vs Valgrind

Most "random" crashes are earlier memory corruption. Prefer
**AddressSanitizer + UBSan** on Linux with Clang:

```bash
export CC=clang CXX=clang++
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON ..
make -j"$(nproc)" install
```

You also want `libdw-dev` and `libbz2-dev` on Ubuntu. The process is slower;
raise `maximum eval cost` in the config if eval-cost kills are getting in
the way.

**Valgrind** is optional and much slower. Use it when you cannot run ASan
(or to confirm a leak ASan already named). Do not treat Valgrind as the
default first tool — the sanitizer build is.

ASan/UBSan stop at the **first** invalid access and print a stack. That is
the report you want. Valgrind's `Invalid read of size 1` is the same class
of bug, found later.

## Catching a native crash (GDB)

The driver usually prints a backtrace itself. To catch it live:

```bash
# use the config for the mudlib you are running
gdb -ex "handle SIGPIPE nostop noprint pass" -ex "run" --args \
  /path/to/fluffos/build/bin/driver /path/to/mudlib/CONFIG_FILE
```

If you are debugging the driver testsuite, `cd testsuite` first and pass
`etc/config.test` instead.

When it stops:

```text
bt
info locals
```

Paste both, plus the version line from startup.

## Filing an issue

Open a [GitHub issue](https://github.com/fluffos/fluffos/issues). Include:

1. The **version string** the driver printed at start (`master` commit or tag).
2. Host OS and compiler (`gcc --version` / `clang --version`).
3. The **full** console output and `debug.log` excerpt, not a paraphrase.
4. A **minimal LPC file** that reproduces the problem if you have one.
5. For memory bugs: an ASan/UBSan log from a Debug sanitizer build.

Do not file "it crashed" with no log. Do not paste only the last line of a
thousand-line sanitizer report — include the first `ERROR` block and the
`#0` stack.

## Related

- [Build from Source](build)
- [CLI `driver`](cli/driver)
- [Runtime configuration](driver/config)
