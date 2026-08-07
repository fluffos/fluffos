---
title: promises / await_callout
---
# await_callout

### NAME
    await_callout - a pending call_out's completion, as a promise

### SYNOPSIS
    promise await_callout(int handle);

### DESCRIPTION
    Returns a promise for the completion of the pending call_out named by
    `handle` (the value call_out(3) returned). The promise is:

    - **fulfilled** with the callback's return value when the call_out
      fires;
    - **rejected** if the call_out is removed with remove_call_out(3), or
      its object is destructed, before it runs;
    - **rejected** if the callback itself raises an error.

    Calling it twice on the same handle returns the same promise.

    Since `await` on a promise always suspends, this makes call_out the
    natural non-blocking pause inside an async function — the awaiting
    function resumes with the callback's result, and with a fresh
    evaluation-cost budget:

        int rows = await await_callout(call_out( (: load_chunk :), 2));

    It is an error to call this with a handle that is not pending (already
    fired, already removed, or never valid).

### EXAMPLES
    ```c
    // yield between chunks so a long job never hits "too long evaluation".
    // NOTE: an await cannot suspend inside a foreach (the loop keeps an
    // lvalue on the stack) -- index with a plain loop instead.
    async void reindex(string *files) {
        int i;

        for (i = 0; i < sizeof(files); i++) {
            index_one(files[i]);
            await await_callout(call_out( (: 0 :), 1));
        }
        write("done\n");
    }
    ```

### SEE ALSO
    call_out(3), remove_call_out(3), async_info(3), promise_then(3)
