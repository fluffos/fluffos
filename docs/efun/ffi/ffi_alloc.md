---
title: ffi / ffi_alloc
---
# ffi_alloc

### NAME

    ffi_alloc() - allocate a native memory block as a buffer

### SYNOPSIS

    buffer ffi_alloc( int nbytes );

### DESCRIPTION

    Returns a zeroed buffer of `nbytes` bytes whose storage is the native
    memory block itself, suitable for structs, out-parameters, and
    pointer arguments to ffi_call(). The block is reference-counted like
    any buffer and freed by the GC; ffi_free() releases it early.

### SEE ALSO

    ffi_free(3), ffi_read(3), ffi_write(3), ffi_address(3), ffi_struct_layout(3)
