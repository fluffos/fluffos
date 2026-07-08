---
layout: doc
title: ffi / ffi_read
---
# ffi_read

### NAME

    ffi_read() - read a typed scalar from a buffer

### SYNOPSIS

    mixed ffi_read( buffer mem, int offset, int type_code );

### DESCRIPTION

    Reads the scalar of type `type_code` (a code from <ffi.h>) stored at
    byte `offset` within `mem`, returning an int for integer/pointer
    types or a float for FFI_FLOAT/FFI_DOUBLE. Combine with
    ffi_struct_layout() to read C struct fields symbolically.

### SEE ALSO

    ffi_write(3), ffi_struct_layout(3), ffi_sizeof(3)
