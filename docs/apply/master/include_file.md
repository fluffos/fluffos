---
title: master / include_file
---
# include_file

### NAME

    include_file - control, translate, or synthesize #include files at
    compile time

### SYNOPSIS

    mixed include_file( string compiled, string from, string path );

### DESCRIPTION

    The driver calls this apply on the master object each time the
    compiler processes a #include directive: the file `path` (which
    might not exist) is about to be included by `from` during the
    compilation of `compiled`.

    **Arguments:**
    - `compiled`: the main file being compiled (with its source
      extension, e.g. `"/obj/sword.lpc"`)
    - `from`: the file containing the #include directive -- the same as
      `compiled` for a top-level include, or the including header for a
      nested one
    - `path`: the include name exactly as written between the `"..."`
      or `<...>` delimiters

    **Return Value:**
    - a *string*: the translated path of the include file. It resolves
      like a quoted include: absolute from the mudlib root, or relative
      to the including file, with the configured include directories as
      fallback. Returning `path` unchanged keeps the default behavior,
      including the `"..."`-versus-`<...>` search semantics.
    - an *array of strings*: the included file's contents itself, one
      element per line, used in place of any on-disk file.
    - *any other value* prevents inclusion of the file `path`: the
      compile fails with an error.

    If the master object does not define this apply (or no master is
    loaded yet), inclusion behaves as usual.

    The apply runs in the middle of a compile. Like valid_override(4)
    and get_include_path(4) it must not trigger another compile (do not
    load or clone unloaded objects from it).

### EXAMPLE

    ```c
    mixed include_file(string compiled, string from, string path) {
        // record the dependency graph for a hot-reload daemon
        HOT_RELOAD_D->note_include(compiled, from, path);

        // per-domain config header
        if (path == "domain.h")
            return "/d/" + domain_of(compiled) + "/include/domain.h";

        // generated constants, no file on disk
        if (path == "version.h")
            return ({ "#define LIB_VERSION " + LIB_VERSION });

        return path;   // default behavior
    }
    ```

### SEE ALSO

    inherit_program(4), get_include_path(4), compile_object(4)
