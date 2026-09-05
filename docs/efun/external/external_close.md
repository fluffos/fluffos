---
title: external / external_close
---
# external_close

### NAME

    external_close() - release an external_create() handle

### SYNOPSIS

    void external_close(int handle);

### DESCRIPTION

    Free the handle. If the process is still running it is aborted (the
    start promise is rejected with `"*external process aborted"`) and
    the child is sent `SIGTERM` (or `TerminateProcess` on Windows).
    Destructing the owning object closes every handle it still holds.

### SEE ALSO

    external_create(3), external_run(3)
