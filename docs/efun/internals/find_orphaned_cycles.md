---
title: internals / find_orphaned_cycles
---
# find_orphaned_cycles

### NAME

    find_orphaned_cycles() - detect (and reclaim) memory leaked by dropped
    reference loops

### SYNOPSIS

    int find_orphaned_cycles(int collect);

### DESCRIPTION

    Scans every live array, class, mapping, and function pointer in the
    driver and returns the number of blocks that are UNREACHABLE -- kept
    alive only by a reference loop after the last outside reference was
    dropped. Pure reference counting can never reclaim such blocks, and no
    LPC-level tool (refs(), break_cycles()) can reach them anymore, because
    by definition no variable leads to them.

    With collect != 0 the orphaned blocks are also reclaimed, safely:
    strings, buffers, and objects they referenced are released normally,
    values still reachable elsewhere are untouched, and the loop members
    themselves are freed. This is a true (if manual) cycle collector.

    Detection is by trial deletion: a data block whose every reference is
    accounted for by other data blocks, with no path from any externally
    held block (an object's variables, the VM stack, a call_out, a driver-
    internal holder), is garbage. Reachable loops are NOT reported -- use
    has_cycle() / find_cycles() / break_cycles() while you still hold a
    reference.

    Only available on debugging builds (DEBUGMALLOC_EXTENSIONS), because it
    needs the debug allocator's registry of every live block; guard call
    sites with `#if efun_defined(find_orphaned_cycles)`. On those builds,
    check_memory() also reports orphaned loops (`unreachable data
    block(s) kept alive only by reference loop(s)`), which makes a dropped
    cycle a hard failure in the driver testsuite.

### EXAMPLE

    mixed *a = ({ 0 });
    a[0] = a;                    // build a loop...
    a = 0;                       // ...and drop it: leaked, invisibly

    find_orphaned_cycles(0);     // 1 -- found it
    find_orphaned_cycles(1);     // 1 -- found it and reclaimed it
    find_orphaned_cycles(0);     // 0

### SEE ALSO

    has_cycle(3), find_cycles(3), break_cycles(3), check_memory(3), refs(3)
