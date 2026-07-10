---
title: ffi / ffi_load
---
# ffi_load

### NAME

    ffi_load() - load a native shared library

### SYNOPSIS

    int ffi_load( string path );

### DESCRIPTION

    Opens the shared library at `path` (via dlopen/LoadLibrary) and
    returns a positive library handle for use with ffi_symbol() and
    ffi_prepare(). Returns 0 on failure; call ffi_error() for the reason.

    An empty `path` opens the driver's own process image, giving access
    to already-linked C symbols such as libc/libm (`sqrt`, `abs`, ...).

    Every call is gated by the master apply valid_ffi("load", path,
    caller); the default master denies it. This efun is only present when
    the driver is built with package_ffi (`__PACKAGE_FFI__`).

### SEE ALSO

    ffi_symbol(3), ffi_prepare(3), ffi_unload(3), ffi_error(3), valid_ffi(4)
