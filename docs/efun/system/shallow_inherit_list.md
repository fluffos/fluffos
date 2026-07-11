---
title: system / shallow_inherit_list
---
# shallow_inherit_list

### NAME

    shallow_inherit_list() - list the objects directly inherited by an object

### SYNOPSIS

    string *shallow_inherit_list(object ob);

### DESCRIPTION

    Returns an array of the filenames of the objects that `ob` inherits
    directly -- its immediate inherits only, one level deep. Each filename
    carries a leading slash. If `ob` is omitted it defaults to
    this_object().

    Only the inherits named in `ob`'s own program are reported; the programs
    those parents inherit in turn are not followed. To recurse through the
    entire inheritance graph and list every inherited file at every depth,
    use deep_inherit_list() instead.

    The efun inherit_list() is an alias for shallow_inherit_list().

### EXAMPLE

    // Given an object whose source begins:
    //   inherit "/std/room";
    //   inherit "/std/container";
    shallow_inherit_list(ob);
    // ({ "/std/room", "/std/container" })

### SEE ALSO

    deep_inherit_list(3), inherit_list(3)
