---
title: internals / dump_jemalloc
---
# dump_jemalloc

### NAME

    dump_jemalloc() - dump the jemalloc heap profile and statistics

### SYNOPSIS

    void dump_jemalloc();

### DESCRIPTION

    Triggers a jemalloc heap-profile dump (via the "prof.dump" mallctl)
    and prints jemalloc allocator statistics to the driver log.

    This is effective only when the driver was built against jemalloc.
    When the driver was not built with jemalloc, the efun does nothing
    except log a message noting that jemalloc is disabled. It returns
    nothing.

### SEE ALSO

    check_memory(3)
