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

    This efun is available only in DEBUGMALLOC builds (compiled with
    DEBUGMALLOC_EXTENSIONS); it does not exist in ordinary builds.

### SEE ALSO

    dump_stralloc(3), reclaim_objects(3)
