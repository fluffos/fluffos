---
title: promises / async_info
---
# async_info

### NAME
    async_info - list the currently suspended async function frames

### SYNOPSIS
    mapping *async_info();

### DESCRIPTION
    Returns one mapping per async function that is currently suspended at
    an `await`, oldest first. Each entry has:

        "id"            int      stable identity, increasing with park order
        "object"        object   the object whose function is suspended
        "function"      string   the suspended function's name
        "file"          string   source file of the await point
        "line"          int      line of the await point
        "promise"       promise  the promise the async call returned
        "awaiting"      promise  the promise being awaited
        "ready"         int      1 if `awaiting` has settled and the resume
                                 is already queued for a later gametick
        "acatch_depth"  int      number of acatch() regions around the await

    This is the async counterpart of call_out_info(3): a debugging and
    monitoring view of pending work. An empty array means nothing is
    suspended.

    The number of concurrently suspended frames is capped by the driver
    option "max suspended async functions"; async_info() is the way to see
    what is holding those slots.

### EXAMPLES
    ```c
    void show_pending() {
        foreach (mapping info in async_info()) {
            printf("%s->%s() at %s:%d%s\n",
                   file_name(info["object"]), info["function"],
                   info["file"], info["line"],
                   info["ready"] ? " (resuming)" : "");
        }
    }
    ```

### SEE ALSO
    call_out_info(3), promise_status(3), promise_then(3)

### NOTE
    Unrelated to the async_read(3)/async_write(3) family from the async
    package, which offloads file and database I/O to worker threads.
