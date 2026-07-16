---
title: contrib / find_cycles
---
# find_cycles

### NAME

    find_cycles() - locate every reference loop in a value

### SYNOPSIS

    string *find_cycles(mixed value);

### DESCRIPTION

    Returns one index path per back-edge -- per slot that closes a
    reference loop in `value`. An empty array means the value is acyclic.
    Breaking (or deleting) exactly the returned slots would make the value
    loop-free; break_cycles() does that in one call.

    Path syntax, concatenated from the outermost container inward:

        [3]          array item 3
        .2           class field 2 (declaration order)
        ["name"]     mapping value under key "name"
        [key <map>]  a mapping KEY (the key itself continues the path)
        (args)       a function pointer's captured argument list

    Which slot of a loop is reported depends on traversal order (array
    index order, mapping table order), so treat the paths as diagnostics,
    not as a stable contract. Long string keys are truncated in the
    rendering.

    The traversal is iterative and has no nesting-depth limit.

### EXAMPLE

    mixed *a = ({ "x", 0 });
    a[1] = a;
    find_cycles(a);                   // ({ "[1]" })

    mapping m = ([]);
    m["self"] = m;
    find_cycles(m);                   // ({ "[\"self\"]" })

    find_cycles(({ 1, ({ 2 }) }));    // ({ }) -- acyclic

### SEE ALSO

    has_cycle(3), break_cycles(3), refs(3)
