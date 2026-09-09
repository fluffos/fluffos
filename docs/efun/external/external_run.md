---
title: external / external_run
---
# external_run

### NAME

    external_run() - start a handle from external_create()

### SYNOPSIS

    promise external_run(int handle);

### DESCRIPTION

    Start the process for a handle from `external_create()` and return a
    promise fulfilled with `({ stdout, stderr, exit_code })` when the
    process exits (a non-zero exit still fulfills -- read `r[2]`), or
    rejected with a socket error (`EESECURITY`, `EESOCKET`, ...) if
    spawn fails -- or with `"*external process aborted"` if the owner
    is destructed first. The child is started with `posix_spawn()`
    (vfork fast path on glibc; Win32 uses `CreateProcess`).

        int h = external_create(CURL_CMD, ({ "-s", url }));
        mixed *r = await external_run(h);
        string body = external_stdout(h);  /* same as r[0] */
        int code = external_exit_code(h);  /* same as r[2] */

    Drive stdin with `external_write()` (before or after start) and
    `external_close_stdin()` when the child should see EOF:

        int h = external_create(CAT_CMD, ({}));
        external_write(h, payload);
        external_close_stdin(h);
        mixed *r = await external_run(h);

    A handle can be run only once. Use `external_start(index, args)`
    when there is no handle.

### CANCELLING

    `external_kill(h)` stops the child and keeps the handle: the run
    promise still fulfills with `({ stdout, stderr, 143 })`.
    `external_close(h)` stops the child, frees the handle, and rejects
    the run promise with `"*external process aborted"`. Destructing
    the owner does the same as `external_close`.
    `promise_reject(p)` of the run promise also kills the child; the
    promise stays rejected with the given reason.

### SEE ALSO

    external_create(3), external_start(3), external_kill(3),
    external_write(3), external_close_stdin(3), external_stdout(3),
    external_stderr(3), external_exit_code(3), external_close(3)
