---
title: source files & object names
---
# Source files and object names

LPC source files use the **`.lpc`** extension; the legacy **`.c`**
extension is fully supported. This page describes how a name like
`"/std/room"` (or `"/std/room.c"`, or `"/std/room.lpc"`) is resolved to
a source file and to a loaded object.

## Resolution rules

When the driver needs to load a file — `load_object()`, an `inherit`
statement, `clone_object()` of an unloaded name, the master or
simul_efun file from the config — it resolves the source like this:

* **An explicit extension is exact.** `load_object("/foo.c")` probes
  only `foo.c`; `load_object("/foo.lpc")` probes only `foo.lpc`. The
  other spelling is never looked up.
* **Extension-less names prefer `.lpc`, then fall back to `.c`.**
  `load_object("/foo")` loads `foo.lpc` if it exists, otherwise
  `foo.c`.
* If no source file matches, the master's `compile_object()` apply is
  consulted with the extension-less name (the *virtual object* hook).
  If it declines, the load fails and `load_object()` returns `0`.

## Object names never carry an extension

A loaded object's name (what `file_name()` / `base_name()` return, and
what the object registry is keyed by) is always the extension-less
path: loading `/std/room.lpc` produces the object `/std/room`.

Consequently object identity is **extension-blind**:

* `find_object("/foo")`, `find_object("/foo.c")` and
  `find_object("/foo.lpc")` all find the same loaded object.
* `load_object()` consults the registry **before** the filesystem: any
  spelling of an already-loaded object's name returns that object; the
  exactness rule above only applies when the load actually reaches the
  disk.

The **program name** (`prog->filename`) is different: it records the
real file that was compiled, extension included. That is the name
reported by `inherit_list()`, `deep_inherit_list()`, error tracebacks,
and compiler diagnostics.

```c
object ob = load_object("/std/room");      // loads /std/room.lpc
file_name(ob);                             // "/std/room"
inherit_list(ob);                          // ({ "/std/container.lpc" })
```

## Writing portable mudlib code

* Refer to objects by their **extension-less** name in normal code
  (`"/std/room"`); use an explicit extension only when you really mean
  a specific file.
* Do not append `".c"` to names before calling `load_object()` /
  `call_other()` — pass the name through and let the driver resolve
  it.
* When comparing against program names (e.g. results of
  `function_exists()` on file paths, `inherit_list()` entries), expect
  either extension.

The full behavior is pinned by the driver test
`testsuite/single/tests/efuns/dual_extension.lpc`.
