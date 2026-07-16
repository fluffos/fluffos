---
title: contrib / break_cycles
---
# break_cycles

### NAME

    break_cycles() - safely clear every reference loop in a value

### SYNOPSIS

    int break_cycles(mixed value);

### DESCRIPTION

    Clears every reference loop in `value` IN PLACE and returns the number
    of edges that were broken. Because compound values are passed by
    reference, the caller's value (and every other holder of it) sees the
    change.

    Exactly the loop-closing back-edges are touched, nothing else:

    - an array/class item or mapping VALUE that closes a loop is
      overwritten with 0;
    - a mapping KEY that closes a loop cannot be overwritten (the node is
      hashed by that key), so the whole key/value node is deleted, exactly
      as if map_delete() had been called;
    - a slot inside a function pointer's captured argument list that
      closes a loop is overwritten with 0 (the function pointer itself
      survives, its captured argument becomes 0); when the loop closes on
      the argument-list edge itself -- possible because bind() shares the
      argument list between the old and the new function pointer -- the
      bound function pointer's whole argument list is detached and
      replaced with a zero-filled one of the same size;
    - everything that is not part of a loop -- including deliberate
      sharing of one structure from several places -- is left untouched.

    One broken edge un-loops an entire ring, so a mutual pair or a ring of
    N containers counts as 1, not N.

    Afterwards has_cycle(value) is 0 and the value can be saved with
    save_object()/save_variable(), deep-copied with copy(), and printed
    with sprintf("%O") without hitting the nesting-depth errors that a
    loop otherwise causes -- and dropping the last reference actually
    frees the memory instead of leaking it.

    Call it before discarding any structure that might have become
    cyclic, e.g. from a generic cache daemon's clean-up path:

        void flush() {
            break_cycles(cache);
            cache = ([]);
        }

    The traversal is iterative and has no nesting-depth limit.

    Caveat (shared with map_delete()): if a loop is closed in mapping-KEY
    position, the node deletion carries the same restriction as deleting
    a mapping entry from inside a `foreach (key, ref value in m)` loop
    over that same mapping -- do not call break_cycles(m) from inside
    such a loop while the ref variable is aimed at the entry being
    removed.

### EXAMPLE

    mixed *a = ({ "keep", 0 });
    a[1] = a;                  // loop
    break_cycles(a);           // 1
    a[0];                      // "keep" -- untouched
    a[1];                      // 0      -- back-edge cleared
    save_variable(a);          // works again

### SEE ALSO

    has_cycle(3), find_cycles(3), refs(3), map_delete(3)
