---
title: ffi / ffi_write
---
# ffi_write

### NAME

    ffi_write() - write a typed scalar into a buffer

### SYNOPSIS

    void ffi_write( buffer mem, int offset, int type_code, mixed value );

### DESCRIPTION

    Writes `value` as the scalar type `type_code` (a code from <ffi.h>)
    at byte `offset` within `mem`. `value` is an int for integer/pointer
    types or a float for FFI_FLOAT/FFI_DOUBLE. Used to fill struct fields
    and out-parameters before an ffi_call().

### SEE ALSO

    ffi_read(3), ffi_struct_layout(3), ffi_alloc(3)
