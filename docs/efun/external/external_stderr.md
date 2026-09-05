---
title: external / external_stderr
---
# external_stderr

### NAME

    external_stderr() - collected stderr of an external_create() handle

### SYNOPSIS

    string external_stderr(int handle);

### DESCRIPTION

    Return the stderr collected so far for `handle`. Same rules as
    `external_stdout()`.

### SEE ALSO

    external_create(3), external_run(3), external_stdout(3)
