---
title: external / external_write
---
# external_write

### NAME

    external_write() - write to an external command's stdin

### SYNOPSIS

    int external_write(int handle, string data);

### DESCRIPTION

    Append `data` to the stdin of a handle from `external_create()`.
    Writes before `external_run()` are buffered and flushed when the
    process starts. Writes after start go to the pipe (or the shared
    stdio socket on Win32).

    The write end stays open until `external_close_stdin()` or
    `external_close()`. Commands that read until EOF need the explicit
    close.

        int h = external_create(CAT_CMD, ({}));
        external_write(h, "one\n");
        mixed p = external_run(h);
        external_write(h, "two\n");
        external_close_stdin(h);
        mixed *r = await p;

    Returns `1` if the data was queued or written, `0` if stdin is
    already closed or the process has exited (the data is dropped).
    Errors only if the handle is invalid or not owned by this object.

### SEE ALSO

    external_create(3), external_run(3), external_close_stdin(3),
    external_close(3)
