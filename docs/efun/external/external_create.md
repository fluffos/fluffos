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
    process is not started. Pass the handle to `external_start()` to run
    it (awaitable), then read `external_stdout()`, `external_stderr()`
    and `external_exit_code()` from the same handle.

        int h = external_create(CURL_CMD, ({ "-s", url }));
        await external_start(h);
        string body = external_stdout(h);
        int code = external_exit_code(h);
        external_close(h);

    `args` is an array of arguments, or a space-separated string.

    The handle is owned by the calling object: another object cannot
    start, read, or close it. Destructing the owner aborts a running
    process and frees the handle.

### SEE ALSO

    external_start(3), external_stdout(3), external_stderr(3),
    external_exit_code(3), external_close(3)
