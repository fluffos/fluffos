---
title: ffi / ffi_error
---
# ffi_error

### NAME

    ffi_error() - most recent FFI error message

### SYNOPSIS

    string ffi_error();

### DESCRIPTION

    Returns the most recent FFI error message for the current thread —
    for example the reason a ffi_load() returned 0 — or the empty string
    if there has been no error. Efuns on the FFI surface raise an LPC
    error on misuse; ffi_error() reports the failures that are signalled
    by a 0 return instead.

### SEE ALSO

    ffi_load(3), ffi_symbol(3), ffi_status(3)
