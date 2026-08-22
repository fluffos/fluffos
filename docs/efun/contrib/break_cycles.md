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
    - a promise slot that closes a loop is cleared: its settled value is
      overwritten with 0, a pending reaction's handler function pointer
      or chained promise is dropped, and a saved slot of a parked async
      frame (or of one of its pending defer() handlers) is overwritten
      with 0 -- the parked function still resumes, with that slot now 0;
    - everything that is not part of a loop -- including deliberate
      sharing of one structure from several places -- is left untouched.

    One broken edge un-loops an entire ring, so a mutual pair or a ring of
    N containers counts as 1, not N.

    NOT EVERY LOOP IS BREAKABLE. Two references inside a suspended async
    frame are not LPC-visible values that can simply become 0, so a loop
    that closes through one of them is REPORTED by has_cycle() and
    find_cycles() but deliberately left intact:

    - a pending defer() handler's function -- it is called unconditionally
      when the frame unwinds, so clearing it would be a null dereference;
    - the coroutine's own result promise -- "breaking" it would mean
      discarding the whole parked frame, and settling that promise mutates
      the reaction list being rewritten.

    Such a loop simply is not counted in the return value. On a develop
    build find_orphaned_cycles() still reclaims it; otherwise it leaks, so
    prefer not to capture the promise an async function awaits inside a
    defer() registered by that same function.

    Afterwards has_cycle(value) is 0 -- unless the value reaches one of the
    unbreakable references above, in which case break_cycles() reports the
    edges it could not cut by simply not counting them, and has_cycle()
    still returns 1. In that case the caveats below do NOT apply: the value
    still cannot be saved or printed without hitting the nesting-depth
    error, and dropping the last reference still leaks (on a develop build
    find_orphaned_cycles() reclaims it). Otherwise has_cycle(value) is 0 and the value can be saved with
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
