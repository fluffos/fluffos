---
layout: doc
title: ffi / ffi_address
---
# ffi_address

### NAME

    ffi_address() - raw native address of a buffer

### SYNOPSIS

    int ffi_address( buffer mem );

### DESCRIPTION

    Returns the raw native address of the first byte of `mem` as an
    integer — for passing to C as a pointer value or for pointer
    comparisons. The address is valid only while `mem` is alive.

### SEE ALSO

    ffi_alloc(3), ffi_peek(3), ffi_call(3)
