---
title: external / external_kill
---
# external_kill

### NAME

    external_kill() - stop an external command, keep the handle

### SYNOPSIS

    int external_kill(int handle);

### DESCRIPTION

    Send `SIGTERM` to a running child (`TerminateProcess` with status
    `143` on Win32, the same as `128 + SIGTERM`). Returns `1` if a
    signal was sent, `0` if the handle is not running (Created or
    already exited). Errors only if the handle is invalid or not owned
    by this object.

    The run promise is not rejected. It fulfills when the child
    actually exits, with `({ stdout, stderr, exit_code })` — typically
    `r[2] == 143`. Partial output stays on the handle.

        int h = external_create(SLEEP_CMD, ({ "20" }));
        mixed p = external_run(h);
        call_out((: external_kill, h :), 10);
        mixed *r = await p;
        external_close(h);

    `external_close(h)` also kills a running child, but frees the
    handle and rejects the run promise with `"*external process
    aborted"`. Use `external_kill` when you still want the result.

    The omit-callback form has no handle: `promise_reject(p, reason)`
    of that start promise kills the child. `with_timeout()` rejects a
    gate and does **not** kill the child.

### SEE ALSO

    external_run(3), external_close(3), external_start(3),
    promise_reject(3)
