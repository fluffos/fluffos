---
title: external / external_write
---
# external_write

### NAME

    external_write() - write to an external command's stdin

### SYNOPSIS

    void external_write(int handle, string data);

### DESCRIPTION

    Append `data` to the stdin of a handle from `external_create()`.
    Writes before `external_start()` are buffered and flushed when the
    process starts. Writes after start go to the pipe (or the shared
    stdio socket on Win32).

    The write end stays open until `external_close_stdin()` or
    `external_close()`. Commands that read until EOF need the explicit
    close.

        int h = external_create(CAT_CMD, ({}));
        external_write(h, "one\n");
        mixed p = external_start(h);
        external_write(h, "two\n");
        external_close_stdin(h);
        mixed *r = await p;

    Errors if the handle is not owned by this object, stdin was already
    closed, or the process has exited.

### SEE ALSO

    external_create(3), external_start(3), external_close_stdin(3),
    external_close(3)
