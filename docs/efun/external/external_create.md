---
title: external / external_create
---
# external_create

### NAME

    external_create() - allocate a handle for an external command

### SYNOPSIS

    int external_create(int external_index, string | string * args);

### DESCRIPTION

    Allocate a handle for a command configured as `external_cmd_N`. The
    process is not started. Pass the handle to `external_run()` to run
    it (awaitable), then read `external_stdout()`, `external_stderr()`
    and `external_exit_code()` from the same handle.

        int h = external_create(CURL_CMD, ({ "-s", url }));
        mixed *r = await external_run(h);
        string body = r[0];       /* also external_stdout(h) */
        string err = r[1];
        int code = r[2];
        external_close(h);

    Write to the child's stdin with `external_write()` (buffered until
    start, then flushed). `external_close_stdin()` closes the write end
    so the child sees EOF.

    `args` is an array of arguments, or a space-separated string.

    The handle is owned by the calling object: another object cannot
    start, read, write, or close it. Destructing the owner aborts a running
    process and frees the handle.

### SEE ALSO

    external_run(3), external_kill(3), external_start(3),
    external_write(3),
    external_close_stdin(3),
    external_stdout(3), external_stderr(3),
    external_exit_code(3), external_close(3)
