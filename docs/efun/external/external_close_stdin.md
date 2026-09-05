---
title: external / external_close_stdin
---
# external_close_stdin

### NAME

    external_close_stdin() - close an external command's stdin

### SYNOPSIS

    void external_close_stdin(int handle);

### DESCRIPTION

    Close the write end of a handle's stdin after any buffered data is
    flushed. The child then sees EOF. Does not kill the process or
    release the handle; use `external_close()` for that.

    Safe to call before start: stdin is closed as soon as the process
    is spawned and the pending buffer has been written.

### SEE ALSO

    external_write(3), external_start(3), external_close(3)
