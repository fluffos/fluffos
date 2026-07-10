---
title: master / inherit_program
---
# inherit_program

### NAME

    inherit_program - control, redirect, or synthesize inheritance at
    compile time

### SYNOPSIS

    mixed inherit_program( string from, string path, int priv );

### DESCRIPTION

    The driver calls this apply on the master object each time the
    compiler processes an `inherit "path";` statement while compiling
    the program `from`. The flag `priv` is nonzero when the inheritance
    is private (`private inherit "path";`).

    **Arguments:**
    - `from`: the file being compiled (with its source extension, e.g.
      `"/obj/sword.lpc"`)
    - `path`: the inherit statement's path, exactly as written
    - `priv`: 1 for `private inherit`, 0 otherwise

    **Return Value:**
    - a *string*: an alternate path for the inherited file; the
      inheritance proceeds as if that path had been written. Returning
      `path` unchanged keeps the default behavior.
    - an *array of strings*: the source code of the inherited file
      itself, one element per line. The program is compiled from that
      text and its master copy materializes under `path`'s name, with no
      backing file. Note that if an object named `path` is already
      loaded, it is inherited directly and the supplied source is not
      consulted, and that such inline programs may themselves only
      inherit programs that are already loaded.
    - *any other value* prevents inheritance of the file `path`: the
      compile fails with an error.

    If the master object does not define this apply (or no master is
    loaded yet), inheritance behaves as usual.

    The apply runs in the middle of a compile. Like valid_override(4)
    and get_include_path(4) it must not trigger another compile (do not
    load or clone unloaded objects from it).

### EXAMPLE

    ```c
    mixed inherit_program(string from, string path, int priv) {
        // record the dependency graph for a hot-reload daemon
        // (find_object never loads -- a path call_other could trigger
        // a compile, which is forbidden here)
        object d = find_object(HOT_RELOAD_D);
        if (d) d->note_inherit(from, path, priv);

        // map a versioned library namespace
        if (sscanf(path, "/lib/v1/%*s"))
            return "/lib/v2" + path[7..];

        // forbid privately inheriting the security library
        if (priv && path == "/secure/acl")
            return 0;

        return path;   // default behavior
    }
    ```

### SEE ALSO

    include_file(4), compile_object(4), valid_override(4),
    get_include_path(4)

The [hot reload guide](../../concepts/general/hot_reload.md) shows how
to build mudlib auto-reloading on top of this apply.
