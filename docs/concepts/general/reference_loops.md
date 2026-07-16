---
title: general / reference_loops
---
# Reference loops (cyclic data structures)

LPC's compound values — arrays, mappings, classes, buffers, and function
pointers — are assigned **by reference**: `b = a` makes `b` point at the same
array as `a`, it does not copy it. Nothing stops one of those references from
pointing back at (or eventually reaching) its own container, and the result is
a **reference loop**:

```c
mixed *a = ({ 0 });
a[0] = a;                    // a self-referential array
a[0][0][0] == a;             // true -- you can walk the loop forever

mapping m1 = ([]), m2 = ([]);
m1["peer"] = m2;             // a two-value loop
m2["peer"] = m1;

class node { mixed next; }
class node n1 = new(class node), n2 = new(class node);
n1.next = n2;                // a circular linked list
n2.next = n1;
```

All of this is **legal**. The loop behaves like any other data structure as
long as you only take one step at a time: indexing, `foreach`, `==` (which is
pointer identity for compound types), `sizeof`, `member_array`, and every
other shallow operation work normally. What a loop changes is *memory
management* and the behavior of the handful of operations that walk a value
*recursively*.

## Why loops leak: reference counting has no cycle collector

The driver reclaims compound values by **pure reference counting**
(`src/vm/internal/base/svalue.cc`): every svalue holding a pointer to an
array/mapping/class counts as one reference, and the value is deallocated the
moment its count reaches zero. There is **no mark-and-sweep pass, no cycle
collector** — nothing ever asks "is this value still *reachable*?".

A reference loop keeps itself alive:

```c
mixed *a = ({ 0 });
a[0] = a;      // the array's ref count is now 2: the variable + itself
a = 0;         // ref count drops to 1 -- and can never reach 0 again
```

After `a = 0` the array is unreachable from any LPC variable, but its own
back-reference holds its count at 1 forever. The memory is leaked until the
driver shuts down. The same applies to a loop of any length: drop the last
outside reference and the whole ring stays allocated, each member kept alive
by the next.

Two properties make this leak unusually quiet:

* **The ref-count comparison can't see it.** A detached loop is perfectly
  self-consistent — every internal reference is accounted for by another
  member of the loop — so the classic "is X, should be Y" check in
  `check_memory()` passes. (On debug builds `check_memory()` now runs a
  *separate* trial-deletion scan for exactly this case — see
  [`find_orphaned_cycles()`](#after-the-fact-find_orphaned_cycles-debug-builds)
  below.)
* **Leak detectors may not see it either.** On Debug builds the driver's own
  allocation tracker keeps a pointer to every block, so tools like
  LeakSanitizer consider the memory "still reachable".

On release builds the only visible symptom is the slow growth of the
`Arrays` / `Mappings` counters in `mud_status(1)` (or `memory_summary()`)
across a long uptime.

## What the recursive consumers do with a loop

Any operation that walks a value's contents recursively would run forever on
a loop. The driver guards each of them with a depth cap instead of cycle
detection, so what you get is a *clean, catchable error* (or truncation), not
a crash:

| Operation | Behavior on a loop |
| --- | --- |
| `sprintf("%O", x)` | truncates with `...` at nesting depth 20 |
| `save_object()` / `save_variable()` | throws `Mappings and/or arrays nested too deep (100) for save_object -- possibly a reference loop; see has_cycle()` |
| `copy()` (deep copy) | throws `Mappings, arrays and/or classes nested too deep (100) for copy() -- possibly a reference loop; see has_cycle()` |
| `restore_object()` / `restore_variable()` | not applicable — the save format cannot express a loop, so restore can never create one |
| `==`, `foreach`, indexing, `sizeof`, ... | shallow; work normally |

These caps share `MAX_SAVE_SVALUE_DEPTH` (100, `options_internal.h`); the
practical consequence is that **a value containing a loop cannot be saved,
deep-copied, or fully printed**. If your object's `save_object()` suddenly
errors with "nested too deep", a reference loop that crept into a saved
variable is the usual culprit.

All of the above is pinned by the regression test
`testsuite/single/tests/operators/reference_loop.lpc`.

## The one loop the driver *does* break: through an object

Objects are the exception, because they have an explicit lifetime. When an
object is destructed, the driver **zeroes the object's global variables on
the spot** (`destruct2()` in `src/vm/internal/simulate.cc`) — precisely so
that "an object with a variable pointing to itself would never be freed"
cannot happen. Every reference the object's variables held is released at
destruct time, regardless of the object's own ref count.

So a loop is harmless **if an object is part of the ring**:

```c
object ob = clone_object("/obj/container");
mapping m = ([ "owner" : ob ]);
ob->set_data(m);             // ob.data -> m -> ob : a loop

destruct(ob);                // driver clears ob's variables:
                             // m's ref from ob is gone, m["owner"] reads 0
```

`destruct()` cuts the ring, and plain reference counting reclaims the rest.

## How to prevent reference loops (and their leaks)

1. **Prefer trees over graphs in plain data.** If a child needs to find its
   parent, consider storing a lookup key (an object name, an index into a
   central mapping) instead of a direct back-reference.

2. **Route unavoidable back-references through an object.** Store the cyclic
   state in a (possibly invisible) daemon or data object's variables. The
   structure stays cyclic and convenient, but `destruct()` — including the
   driver's normal `clean_up`/swap-driven destruction — reliably reclaims
   it. This is the idiomatic LPC ownership pattern: the *object* owns the
   graph; the graph does not own itself.

3. **Break the loop before dropping the last reference.** If you build
   transient cyclic structures, null the back-edge when you are done:

   ```c
   a[0] = 0;                 // self-referential array: cut the self-edge
   map_delete(m1, "peer");   // mutual mappings: one deleted edge un-loops both
   n2.next = 0;              // circular list: cut any one link
   ```

   One cut edge anywhere in the ring is enough — reference counting reclaims
   everything downstream of the cut.

4. **Never let a loop reach saved variables.** `save_object()` will error and
   your object's persistence silently stops working. Mark cache-like
   variables that might contain shared/cyclic data `nosave`.

5. **Audit with `refs()`.** The `refs(value)` efun (develop package) returns
   the value's reference count. A freshly built structure held by one
   variable reports 1; if it reports more and you can't account for the
   extras, something (possibly itself) is holding it:

   ```c
   mixed *a = ({ 0 });
   refs(a);      // 1 -- just the variable
   a[0] = a;
   refs(a);      // 2 -- the variable, plus the loop's own reference
   ```

## Runtime cycle tools: `has_cycle()`, `find_cycles()`, `break_cycles()`

The contrib package ships three efuns built for exactly this problem. All
three share one **iterative** graph walk (no C-stack recursion), so unlike
`save_variable()`/`copy()` they have no nesting-depth limit, and they follow
every kind of edge a loop can hide in: array/class items, mapping keys *and*
values, and a function pointer's captured argument list.

```c
mixed *a = ({ "x", 0 });
a[1] = a;

has_cycle(a);      // 1 -- there is a loop
find_cycles(a);    // ({ "[1]" }) -- one path per loop-closing slot
break_cycles(a);   // 1 -- cleared it, in place
a[1];              // 0; a[0] is still "x", and refs(a) is back to 1
```

* **`has_cycle(value)`** — cheap predicate. Ideal as a guard before
  `save_object()`, in a daemon's periodic self-check, or as an assertion in
  test code.
* **`find_cycles(value)`** — one index path per back-edge
  (`"[3][\"peer\"].1"` style), pointing at the exact slot that closes each
  loop. Diagnostics for *where* the loop crept in.
* **`break_cycles(value)`** — the safe-clearing primitive. It cuts exactly
  the loop-closing edges: item/value slots are zeroed in place, and a loop
  closed in mapping-*key* position has its node deleted (a hashed key cannot
  be overwritten). Deliberate sharing (DAG edges) is never touched, one cut
  un-loops a whole ring, and afterwards the value saves, copies, prints, and
  — crucially — *frees* normally. Call it before discarding any structure
  that might have become cyclic.

Because a broken loop is precisely the "break the back-edge" pattern from
rule 3, `break_cycles()` is what you reach for when the structure was built
by code you don't control (restored data run through mudlib mutators, a
generic cache, user-scripted content).

## After the fact: `find_orphaned_cycles()` (debug builds)

The three efuns above need a variable that still reaches the loop. Once the
last outside reference is dropped, the loop is unreachable — nothing in LPC
can name it anymore. On debugging builds (`DEBUGMALLOC_EXTENSIONS`), the
develop package adds the missing piece:

```c
mixed *a = ({ 0 });
a[0] = a;
a = 0;                       // leaked -- invisibly, until now

find_orphaned_cycles(0);     // 1 -- detected after the fact
find_orphaned_cycles(1);     // 1 -- detected AND reclaimed
find_orphaned_cycles(0);     // 0
```

Detection is by **trial deletion** (the same idea CPython's garbage
collector uses): a data block whose every reference comes from other data
blocks, with no path from any externally held block — an object's
variables, the VM stack, a `call_out`, a driver-internal holder — can only
be loop garbage. Because the verdict is computed from real reference
counts, reachable data is never misclassified. With the collect flag the
garbage is reclaimed safely: values still reachable elsewhere are
untouched, and everything the loop held (strings, buffers, object refs) is
released normally.

`check_memory()` runs the same scan and reports
`unreachable data block(s) kept alive only by reference loop(s)`, so on
debug builds — including the driver's own testsuite, which calls
`check_memory()` after every test file — **dropping a cycle is now a hard,
attributed failure** instead of a silent leak.

Since `save_object()`, `save_variable()` and `copy()` failing with "nested
too deep" is the classic *symptom* of a loop, those errors now say so:
`... nested too deep (100) for save_object -- possibly a reference loop;
see has_cycle()`.

For driver developers: `copy()`'s depth-cap error path must stay
unwind-safe — the deep-copy helpers hold their half-built containers in
RAII guards (`src/packages/contrib/contrib.cc`) because a cyclic argument
*always* takes that error path. Any new efun that walks svalues recursively
needs its own depth cap (audit checklist item — see `AGENTS.md` §13.4) and
the same unwind discipline.
