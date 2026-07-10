---
title: ffi / ffi_prepare
---
# ffi_prepare

### NAME

    ffi_prepare() - describe a C function signature for calling

### SYNOPSIS

    int ffi_prepare( int lib, string name, int ret_type, int *arg_types );

### DESCRIPTION

    Resolves `name` in `lib`, builds the platform call frame for the
    described signature, and returns a callable function handle for
    ffi_call(). `ret_type` and each element of `arg_types` are type codes
    from <ffi.h> (FFI_INT32, FFI_DOUBLE, FFI_POINTER, ...).

    Gated by the master apply valid_ffi("prepare", name, caller). Errors
    if the symbol is missing or a type code is invalid.

### SEE ALSO

    ffi_call(3), ffi_load(3), ffi_sizeof(3), valid_ffi(4)
