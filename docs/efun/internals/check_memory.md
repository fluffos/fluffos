---
title: internals / check_memory
---
# check_memory

### NAME

    check_memory() - verify tracked memory allocations and report

### SYNOPSIS

    string check_memory(int flag);

### DESCRIPTION

    Walks every tracked memory allocation, verifying block integrity and
    reference counts, and returns the resulting report as a string.

    `flag` is a bitmask, defaulting to 0 (a full report with no totals
    table):

    Bit 0 (value 1)  Additionally appends a per-source allocation totals
                     table, listing block counts and byte totals broken
                     down by allocation source.
    Bit 1 (value 2)  Runs silently -- suppresses the warnings and report
                     and returns 0 instead of the report string.
    Bit 2 (value 4)  Skips the orphaned-reference-loop scan (see below).

    Unless bit 1 or bit 2 is set, the report also includes a scan for
    data blocks that are unreachable because only a reference loop keeps
    them alive (`unreachable data block(s) kept alive only by reference
    loop(s)`) -- garbage that pure reference counting can never reclaim
    and that the plain ref-count comparison cannot see. Reclaim such
    blocks with find_orphaned_cycles(1). The driver testsuite calls
    check_memory() after every test file, so a test that drops a cyclic
    structure without breaking it first now fails with that warning.

    This efun is available only in DEBUGMALLOC builds (compiled with
    DEBUGMALLOC_EXTENSIONS); it does not exist in ordinary builds.

### SEE ALSO

    find_orphaned_cycles(3), dump_stralloc(3), reclaim_objects(3)
