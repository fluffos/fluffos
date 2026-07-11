---
title: ffi / ffi_status
---
# ffi_status

### NAME

    ffi_status() - counts of open FFI handles

### SYNOPSIS

    mapping ffi_status();

### DESCRIPTION

    Returns a mapping of live-handle counts for introspection and leak
    checks, with integer values under the keys "libraries" (open
    ffi_load() handles), "functions" (prepared ffi_prepare() handles),
    and "callbacks" (live ffi_callback() closures).

### SEE ALSO

    ffi_load(3), ffi_prepare(3), ffi_callback(3)
