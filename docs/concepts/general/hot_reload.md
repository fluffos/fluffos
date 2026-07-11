---
title: general / hot_reload
---
# Hot reload

FluffOS can tell the mudlib, at compile time, exactly which files every
program is built from. That is the missing piece for **auto hot-reload**:
a daemon that watches source files on disk and automatically
hot-updates the affected programs — master copies and live clones
alike, variable state intact — the moment anything changes, so
developers see their edits take effect without manually running an
`update` command.

This page explains the driver mechanism and walks through a complete
mudlib implementation. A working reference lives in the testsuite:
`/single/hot_reload.lpc` (the daemon), the applies in
`/single/master.lpc`, and the end-to-end demonstration in
`/single/tests/applies/hot_reload.lpc`.

## Why "the file changed" is not enough

A compiled LPC program is built from more than its own source file:

* every `#include`d header is spliced into the compile, and
* every `inherit`ed program is bound into the child at compile time —
  the child's program links against the exact parent program it was
  compiled with.

So when `/std/base.c` or a header it includes changes, reloading only
the objects whose *own* file changed silently leaves every inheritor
running stale code. A correct hot-reloader needs the **dependency
graph**: for each program, the transitive set of files its bytecode was
built from.

## The driver mechanism: two compile-time master applies

The driver consults two master applies during every compile:

```c
mixed inherit_program( string from, string path, int priv );
mixed include_file( string compiled, string from, string path );
```

[`inherit_program`](../../apply/master/inherit_program.md) is called for
every `inherit "path";` statement while compiling `from` (`priv` is
nonzero for `private inherit`). [`include_file`](../../apply/master/include_file.md)
is called for every `#include` directive: `path` is about to be included
by `from` during the compilation of `compiled` (`from` differs from
`compiled` for an include nested inside a header).

Their return value controls the compile — a string redirects the
inherit/include to another path, an array of strings supplies the
source text itself, anything else denies it — but for hot reload you
only need their *arguments*: **return `path` unchanged** (which keeps
stock behavior exactly) **and record the edge**. Every compile then
feeds the daemon one `include_file` call per directive and one
`inherit_program` call per inherit, which is the complete dependency
graph.

Two rules for implementing them:

* They run in the **middle of a compile**. Never trigger another
  compile from them — no `load_object()`/`clone_object()` of unloaded
  files, and look the daemon up with `find_object()`, not a path
  call_other (which would load it).
* If the master does not define them (or no master is loaded yet, e.g.
  while the master itself compiles), the driver just proceeds normally,
  so they are safe to add to an existing mudlib.

## Step 1: delegate from the master

Keep the master thin; forward to a daemon when it is active:

```c
// in master.c
#define HOT_RELOAD_D "/daemon/hot_reload"

mixed inherit_program(string from, string path, int priv) {
    object d = find_object(HOT_RELOAD_D);   // find_object: never loads
    if (d) return d->inherit_program(from, path, priv);
    return path;                            // unchanged = stock behavior
}

mixed include_file(string compiled, string from, string path) {
    object d = find_object(HOT_RELOAD_D);
    if (d) return d->include_file(compiled, from, path);
    return path;
}
```

## Step 2: record the dependency graph

The daemon keys everything by *program name* (object name without the
`.c`/`.lpc` extension, which is how `from`/`compiled` arrive minus the
extension they carry). Includes are recorded as resolved file paths;
inherits as edges to another program:

```c
// program name -> ([ source path : 1 ]) - files spliced into the compile
private mapping file_deps = ([]);
// program name -> ([ inherited program name : 1 ])
private mapping inherit_deps = ([]);

private string program_of(string file) {
    if (strlen(file) > 4 && file[<4..] == ".lpc") return file[0..<5];
    if (strlen(file) > 2 && file[<2..] == ".c")   return file[0..<3];
    return file;
}

// Resolve an include spelling the way the compiler will: absolute from
// the mudlib root, else relative to the includer, else the include dirs.
private string resolve_include(string from, string path) {
    string *candidates;
    if (path[0] == '/') {
        candidates = ({ path });
    } else {
        int slash = strsrch(from, '/', -1);
        string dir = slash > 0 ? from[0..slash - 1] : "";
        candidates = ({ dir + "/" + path, "/include/" + path });
    }
    foreach (string c in candidates)
        if (file_size(c) >= 0) return c;
    return 0;
}

mixed include_file(string compiled, string from, string path) {
    string prog = program_of(compiled);
    string dep = resolve_include(from, path);
    if (dep) {
        if (!file_deps[prog]) file_deps[prog] = ([]);
        file_deps[prog][dep] = 1;
    }
    return path;      // always: hot reload only observes
}

mixed inherit_program(string from, string path, int priv) {
    string prog = program_of(from);
    if (!inherit_deps[prog]) inherit_deps[prog] = ([]);
    inherit_deps[prog][program_of(path)] = 1;
    return path;
}
```

The transitive file set of a program is then its own source, its
recorded includes, and — recursively — those of everything it inherits:

```c
private mapping closure_files(string prog, mapping seen) {
    mapping out = ([]);
    string src;

    if (seen[prog]) return out;
    seen[prog] = 1;

    src = source_of(prog);           // prog + ".lpc" if it exists, else ".c"
    if (src) out[src] = 1;
    if (file_deps[prog]) out += file_deps[prog];
    if (inherit_deps[prog])
        foreach (string parent in keys(inherit_deps[prog]))
            out += closure_files(parent, seen);
    return out;
}
```

## Step 3: detect changes and reload

Snapshot `({ size, mtime })` per file (`get_dir(file, -1)`) when a
program is registered or reloaded. Comparing the size as well as the
mtime matters: mtime has one-second granularity, so an edit made twice
within a second would otherwise be missed.

```c
private mixed *stat_file(string f) {
    mixed *info = get_dir(f, -1);
    if (!sizeof(info)) return 0;
    return ({ info[0][1], info[0][2] });   // size, mtime
}
```

The one subtlety is reload *order*. Recompiling a child finds
already-loaded parent programs by name, so a changed parent's master
copy must be destructed **first** or the fresh child compile would inherit
the stale in-memory parent:

```c
private void reload(string prog) {
    foreach (string a in ancestors(prog, ([]))) // inherit closure, prog excluded
        if (self_changed(a))                  // a's own source/includes changed
            forget(a);                        // destruct master copy, drop records
    forget(prog);
    load_object(prog);       // recompiles; the applies rebuild the records
    snapshot_program(prog);  // fresh size/mtime baseline
}

public int check_now() {
    int n = 0;
    // Collect the stale set BEFORE reloading anything: a reload
    // refreshes the snapshots of its whole closure, which would hide a
    // shared dependency's change from other watched programs in the
    // same pass.
    string *stale = filter(keys(watched), (: closure_changed($1) :));

    foreach (string prog in stale) {
        // A failed recompile (a syntax error mid-edit) must not abort
        // the pass: the snapshot stays stale, so the next pass retries
        // and self-heals once the file is fixed.
        if (catch(reload(prog))) continue;
        n++;
    }
    return n;
}
```

Destructing the parent's master copy is safe even while other programs
still use its code: compiled programs are reference counted, so
existing inheritors keep the old program until they are themselves
reloaded.

This destruct+load `reload()` is the simplest correct baseline — step
5 upgrades it to the driver's `recompile_object()` efun, which performs the
same recompile-and-swap on the *live* objects, keeping variable state
and reaching every clone.

## Step 4: make it automatic

Poll from a `call_out` loop; `watch()` is the registration point (it
loads the target itself so the compile happens while the daemon is
recording — `keep_state` is explained in step 5):

```c
public int watch(string prog, int keep_state: (: 1 :)) {
    prog = program_of(prog);
    if (!find_object(prog)) load_object(prog);
    watched[prog] = keep_state ? WATCH_KEEP_STATE : WATCH_PLAIN;
    snapshot_program(prog);
    return 1;
}

protected void poll() {
    // Re-arm first: a throw out of the pass must not kill the poller.
    if (poll_interval) call_out("poll", poll_interval);
    check_now();
}

public void enable(int interval) {
    poll_interval = interval ? interval : 2;
    "/secure/master"->set_compile_hooks(this_object());
    if (find_call_out("poll") == -1) call_out("poll", poll_interval);
}
```

## Step 5: keep variable state across reloads — recompile_object()

The destruct+load cycle resets global variables to their initializers
and cannot touch clones. The driver's
[`recompile_object()`](../../efun/objects/recompile_object.md) efun replaces that
cycle entirely:

```c
int n = recompile_object(find_object("/obj/widget"));
```

recompiles the program from its source file and swaps it into the
**live master copy and every clone sharing it** — no destruct, same
object identity everywhere (pointers held by other objects, inventory,
shadows, interactive state, call_outs and heart_beat all survive).
Each object's global variables carry over **by name**, private ones
included: the new program's initializers run first, then every
surviving name gets its old value back; new variables keep their
initializers, vanished names are dropped. It returns the number of
objects updated.

The reference daemon's default (state-keeping) reload path is exactly
this: `recompile_object()` on each changed ancestor, then on the watched
program, so the child's recompile picks up the freshly swapped parent.

Two things to know:

* An object **currently executing** (anywhere on the call stack)
  cannot be hot-updated — the daemon's poller naturally runs from a
  `call_out`, where watched objects are idle.
* Function pointers made against the old program layout (pointers to
  local functions, functionals) become stale and raise a clean error
  when called; recreate them after the update.

### Doing it by hand: value transfer

Objects that define `hot_reload_state()` (optionally paired with
`hot_reload_restore()`) opt out of `recompile_object()`
in the reference daemon: they reload through destruct+load and get
back exactly the state their pair chose to carry — the way to make
some state deliberately *not* survive.

The same destruct+load path can also carry state generically without
driver support, using three efuns from the contrib package —
[`variables()`](../../efun/contrib/variables.md),
[`fetch_variable()`](../../efun/contrib/fetch_variable.md) and
[`store_variable()`](../../efun/contrib/store_variable.md):

```c
// BEFORE destructing: capture every reachable global by name
mapping state = ([]);
foreach (string name in variables(ob))
    catch (state[name] = fetch_variable(name, ob));

// AFTER the fresh load: restore the names that still exist
foreach (string name, mixed value in state)
    catch (store_variable(name, value, fresh));
```

Unlike `recompile_object()`, this transfer runs outside the driver, so
`private` variables are not reachable (the `catch` skips them) and a
captured value that referenced the old master copy nulls out when it
is destructed.

To summarize the reference daemon's modes: `watch(prog)` reloads
through `recompile_object()` (state and clones carried in place),
`watch(prog, 0)` opts out (destruct+load, fresh initializers, clones
untouched), and the cooperative path takes over when the object
defines `hot_reload_state()`. All three shapes are pinned by
`/single/tests/applies/hot_reload.lpc`, and the `recompile_object()`
semantics themselves by `/single/tests/efuns/recompile_object.lpc`.

## Putting it together

```c
object d = load_object("/daemon/hot_reload");
d->enable(2);                 // register hooks, poll every 2 seconds
d->watch("/obj/widget");      // widget inherits /std/base, which includes colors.h

// ... edit /std/colors.h on disk ...

// within a poll interval the daemon notices: colors.h is in widget's
// dependency closure through /std/base -> hot-updates the stale base,
// then widget, in place. The next "/obj/widget"->describe() -- and
// every live widget clone -- runs the new code, all state intact.
```

This exact flow — including the deepest case, editing an include of an
*inherited* program — is exercised by the testsuite in
`/single/tests/applies/hot_reload.lpc`.

## Semantics and caveats

* **Clones follow the reload path.** The default `recompile_object()` path
  swaps the program into the master copy *and every live clone*, each
  keeping its own state. The destruct+load path (opt-out mode, or the
  cooperative pair) replaces only the master copy: existing clones
  keep the old program until re-created — enumerate them with
  [`children()`](../../efun/objects/children.md) (filter with
  `clonep()`) to destruct or migrate stragglers. Either way, anything
  fetched via `"/path/name"->func()` or fresh `find_object()`/`new()`
  after the reload gets the new code.
* **Virtual objects update through their backing program** — the
  efun recompiles the real file the virtual object carries. The
  daemon's dependency records are keyed by compiled program name, so
  watch virtual objects via their backing file.
* **Even the master and simul_efun objects can be recompiled** — the
  driver rebuilds their dispatch tables (with name-stable simul_efun
  indices) during the swap. Do it from a `call_out` or another idle
  context: like any object, they cannot be recompiled while executing.
* **Function pointers into a hot-updated object go stale** (pointers
  to local functions, functionals): calling one afterwards raises a
  clean error rather than running mis-indexed code. Recreate them
  after the update; efun/simul_efun pointers are unaffected.
* **Only programs compiled while the daemon was registered have
  complete records.** That is why `watch()` loads its target itself,
  and why the daemon rebuilds records during every reload.
* **Never compile from inside the applies.** They run mid-compile; the
  compiler is not reentrant. Record and return — do the loading from
  `check_now()`/`poll()`.
* Includes that resolve through `valid_read` denials or exotic
  [`get_include_path`](../../apply/master/get_include_path.md) setups
  may need a smarter `resolve_include()` than the sketch above.
* The applies also *can* rewrite compiles (redirect paths, inject
  source, deny). A hot-reload daemon should not: return `path`
  unchanged, always.

## See also

* [`inherit_program`](../../apply/master/inherit_program.md),
  [`include_file`](../../apply/master/include_file.md) — the apply
  reference pages, including the redirect / inline-source / deny
  return forms
* [`recompile_object`](../../efun/objects/recompile_object.md) — the efun
  reference: swap a recompiled program into the live master copy and
  its clones
* [`inherit`](../../lpc/constructs/inherit.md),
  [`#include`](../../lpc/preprocessor/include.md) — the language
  constructs and their resolution rules
