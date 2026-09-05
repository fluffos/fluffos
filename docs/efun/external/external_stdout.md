---
title: external / external_stdout
---
# external_stdout

### NAME

    external_stdout() - collected stdout of an external_create() handle

### SYNOPSIS

    string external_stdout(int handle);

### DESCRIPTION

    Return the stdout collected so far for `handle`. Safe to call while
    the process is still running (partial output) or after
    `external_start(handle)` has fulfilled (complete output, UTF-8
    sanitized, capped at `__MAX_STRING_LENGTH__`).

### SEE ALSO

    external_create(3), external_start(3), external_stderr(3)
