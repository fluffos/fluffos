---
title: objects / recompile_object
---
# recompile_object

### NAME

    recompile_object() - recompile a program and swap it into the live
    master copy and all its clones, keeping variable state

### SYNOPSIS

    int recompile_object( object master_copy );

### DESCRIPTION

    Recompiles the master copy's program from its source file and swaps
    the fresh program into the master copy AND every clone sharing it.
    Nothing is destructed: object identity is preserved everywhere, so
    pointers held by other objects, the object's name, inventory,
    shadows, interactive state, heart_beat and pending call_outs all
    stay intact. This is an in-place "hot update", as opposed to the classic
    destruct + load_object() cycle (which resets state and cannot touch
    clones).

    Each updated object's global variables carry over BY NAME: the new
    program's variable initializers run first, then every variable
    whose name also existed in the old program gets its old value back
    (private variables included - the transfer happens inside the
    driver). Variables new in this version keep their initializers;
    vanished names are dropped. create() is NOT called again.

    The recompile behaves like a normal load: unloaded inherited
    programs are loaded on demand, and the master applies
    inherit_program(4), include_file(4), get_include_path(4) and
    valid_read(4) are all consulted.

    Returns the number of objects updated (the master copy plus its
    clones).

### ERRORS

    The call fails with an error if:

    - a clone is passed (pass the master copy; its clones are updated
      with it);
    - any object sharing the program is currently executing, anywhere
      on the call stack (bytecode positions and variable indices in
      live frames are relative to the old program) - in particular an
      object cannot recompile itself;
    - the source fails to compile (the objects are left untouched on
      the old program);
    - the target is the simul_efun object, a replace_program() is
      pending on the program, or recompile_object is already in progress.

### CAVEATS

    Function pointers made before the update whose behavior depends on
    the owner's program layout (pointers to local functions, and
    functionals / anonymous functions) become STALE: calling them after
    a recompile_object of their owner raises a clean "Stale function pointer"
    error instead of running mis-indexed code. Recreate them after the
    update. Efun and simul_efun pointers are unaffected.

    Programs that INHERIT the updated program are not recompiled - like
    with the destruct/reload cycle, inheritance copies code at compile
    time. Update inheritors separately (parents first). The testsuite's
    /single/hot_reload.lpc daemon automates exactly that ordering from
    the compile-time dependency graph.

### EXAMPLE

    ```c
    object ob = find_object("/obj/sword");

    // ... edit /obj/sword.c on disk ...

    int n = recompile_object(ob);
    // every live sword (master copy + clones) now runs the new code,
    // each keeping its own enchantment, wielder, condition, ...
    write(sprintf("updated %d objects\n", n));
    ```

### SEE ALSO

    reload_object(3), children(3), clonep(3), destruct(3),
    inherit_program(4), include_file(4)
