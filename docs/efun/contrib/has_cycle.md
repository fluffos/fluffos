---
title: contrib / has_cycle
---
# has_cycle

### NAME

    has_cycle() - test whether a value contains a reference loop

### SYNOPSIS

    int has_cycle(mixed value);

### DESCRIPTION

    Returns 1 if the reference graph of `value` contains a loop (a
    structure that can reach itself), 0 otherwise.

    The walk follows array and class items, mapping keys and values, and a
    function pointer's captured argument list. Objects are leaves: a loop
    routed through an object's global variables is reclaimed by destruct()
    and is not reported here.

    Sharing without a loop (the same array referenced from two places) is
    NOT a cycle and returns 0.

    The traversal is iterative, so unlike save_variable() or copy() it has
    no nesting-depth limit: arbitrarily deep acyclic values scan cleanly.

    Values containing a loop cannot be saved, deep-copied, or fully
    printed, and once the last outside reference is dropped they leak
    permanently (the driver reclaims values by reference counting and has
    no cycle collector). See the "Reference Loops" concepts page for the
    full story.

### EXAMPLE

    mixed *a = ({ 0 });
    a[0] = a;
    has_cycle(a);      // 1
    a[0] = 0;
    has_cycle(a);      // 0

### SEE ALSO

    find_cycles(3), break_cycles(3), refs(3), copy(3)
