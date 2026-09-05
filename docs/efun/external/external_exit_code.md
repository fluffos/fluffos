---
title: external / external_exit_code
---
# external_exit_code

### NAME

    external_exit_code() - wait status of an external_create() handle

### SYNOPSIS

    int external_exit_code(int handle);

### DESCRIPTION

    `-1` if the process has not exited yet. After `await external_start(h)`
    fulfills, the child's wait status: `0` on success, the exit code on a
    normal exit, and `128 + signo` on POSIX if the child died from a
    signal. A non-zero code does not reject the start promise.

### SEE ALSO

    external_create(3), external_start(3)
