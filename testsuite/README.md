# FluffOS Testsuite

This directory is a minimal mudlib (a descendant of the classic *Lil*
bootstrap mudlib) plus the driver's LPC regression suite. The driver
boots it directly; every efun/compiler/VM behavior change is expected
to come with a test here.

## Source file extension

All LPC sources in this tree use the **`.lpc`** extension. The driver
resolves source files by these rules (implemented in `load_object()`,
`src/vm/internal/simulate.cc`, and pinned by
`single/tests/efuns/dual_extension.lpc`):

* An **explicit extension is exact**: `load_object("/foo.c")` probes
  only `foo.c`, `load_object("/foo.lpc")` probes only `foo.lpc` — the
  other spelling is never looked up.
* **Extension-less names prefer `.lpc`** and fall back to `.c`
  (`load_object("/foo")` loads `foo.lpc` if present, else `foo.c`).
* **Object identity is extension-blind**: object names never carry an
  extension, and `find_object()` strips either spelling, so
  `"/foo"`, `"/foo.c"`, and `"/foo.lpc"` all find the same loaded
  object. The program name (`prog->filename`, what `inherit_list()`
  and diagnostics report) carries the real extension of the file that
  was compiled.
* **The registry wins over the filesystem**: `load_object()` returns an
  already-loaded object for *any* spelling of its name — the exactness
  rule only applies when the load actually hits the disk.
* When an exact probe misses, the load falls through to the master's
  `compile_object()` **virtual-object** hook, which receives the
  *stripped* name; if it declines, `load_object()` returns `0` (no
  error is thrown).
* `children()`, `save_object()`/`restore_object()`, `replace_program()`
  and `function_exists()` treat both spellings equivalently (name
  stripping / suffix handling covers `.lpc` and `.c` alike).

A few `.c` files exist on purpose: the `clone/dual_*.c` fixtures for
`dual_extension.lpc`, and `/tmp_eval_file.c` written at runtime by the
`eval`/`codefor` commands (live proof that genuinely `.c`-named
sources still compile).

## Running the suite

The suite is a first-class **ctest** test and a set of CMake targets:

| Invocation | What it does |
|---|---|
| `ctest -R testsuite` (or `ctest -L testsuite`) | Runs the whole LPC suite through ctest, alongside `ctest -LE testsuite` for the GTest binaries. This is what CI runs. |
| `driver-autotest` (CMake target) | Same run, invoked directly; exits nonzero on any failure. |
| `driver-testsuite` (CMake target) | Boots the driver against this mudlib for interactive poking (log in and type `tests`). |

The runner (`command/tests.lpc`) prints a gtest-style protocol:

```text
[ RUN      ] /single/tests/efuns/dual_extension.lpc
[       OK ] /single/tests/efuns/dual_extension.lpc (24 ms)
[==========] 3401 checks from 297 file(s) ran. (7470 ms total)
[  PASSED  ] 297 file(s).
Checks succeeded.
```

**Failures do not stop the run**: a failed check is printed with its
expected/actual diff and trace, recorded, and the run continues — one
run reports *every* failure (gtest semantics), then the recap lists each
`[  FAILED  ]` file and the driver exits nonzero. `Checks succeeded.`
plus exit 0 is the machine-readable pass signal.

Run one file, or a glob over test paths:

```bash
./build/bin/driver testsuite/etc/config.test '-ftest:single/tests/efuns/dual_extension.lpc'
./build/bin/driver testsuite/etc/config.test '-ftest:efuns/dual*'
```

Without an argument the runner walks `/single/tests/` recursively in
**randomized order** (run it 2–3× when touching the lexer/parser), so
tests must not depend on each other having run.

## How the runner treats each directory

`command/tests.lpc` walks `/single/tests/`:

* `single/tests/**/*.lpc` — regular tests: each file is loaded and its
  `do_tests()` is called; any uncaught error or failed assertion fails
  the suite.
* `.../fail/*.lpc` — files that must **fail to compile/load**; the
  runner asserts `catch(load_object(...))` throws.
* `.../crasher/*.lpc` — regression cases that only need to **not crash
  the driver**; errors are ignored.
* In DEBUGMALLOC builds the runner calls `check_memory()` after every
  file and fails on any leaked allocation, so tests must clean up
  (destruct clones, remove temp files).

## Writing a test

Create `single/tests/<area>/<name>.lpc` with a `do_tests()` entry
point. `etc/config.test` auto-includes `<globals.h>` into every object,
which provides the assertion macros from `include/tests.h`:

```c
void do_tests() {
    ASSERT(intp(1));                    // truthiness
    ASSERT2(sizeof(x) == 3, "reason");  // with message
    ASSERT_EQ("expected", actual);      // equality with diff output
    ASSERT_NE(a, b);
}
```

Failed assertions print `file:line, Check failed` (with expected/actual
and a trace for `ASSERT_EQ`/`ASSERT_NE`), are recorded, and the run
continues; any recorded failure makes the final recap fail the run with
a nonzero exit (that is how CI detects failure). Helper
fixtures that should not be executed as tests live outside
`/single/tests/` — conventionally in `/clone` (e.g. `inh0`–`inh2`,
`dual_*`) or as `#include`s under `/include`. A fixture whose test
depends on it being **unloaded** (e.g. a `fail/` test pinning that a
name does *not* resolve) must be private to that one test: the object
registry is extension-blind and survives across files, so any other
test loading the fixture first would change the outcome under the
randomized order.

Efun tests are named after the efun (`single/tests/efuns/<efun>.lpc`)
and should aim to cover every branch of the C++ implementation,
including error paths (`catch(...)`).

## Layout

| Path | What |
|---|---|
| `etc/config.test` | Driver config the suite boots with (mudlib dir, include dirs, global include, limits). |
| `single/master.lpc` | Master object: `flag()` test entry, `compile_object()` virtual-object hook (`/test/virtual`), `get_include_path()` cases, error handling. |
| `single/simul_efun.lpc` | Simul-efuns available everywhere in the suite. |
| `single/tests/` | The test tree (`efuns/`, `compiler/`, `operators/`, `applies/`, `std/`, plus `fail/` and `crasher/` subdirs). |
| `command/` | Interactive commands; `tests.lpc` is the suite runner, `speed.lpc` the benchmark entry (`-fspeed`). |
| `clone/`, `inherit/`, `std/`, `u/` | Fixture objects, inheritance helpers, minimal std lib, user dirs. |
| `include/` | Headers; `tests.h` (assertions), `globals.h` (auto-included). |
| `data/`, `log/` | Runtime state and logs written by the suite. |
