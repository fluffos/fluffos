---
layout: doc
title: ffi / ffi_sizeof
---
# ffi_sizeof

### NAME

    ffi_sizeof() - size of a scalar type code

### SYNOPSIS

    int ffi_sizeof( int type_code );

### DESCRIPTION

    Returns the size in bytes, on this platform, of the scalar C type
    named by `type_code` (a code from <ffi.h>). Useful for sizing
    ffi_alloc() blocks and computing offsets by hand.

### SEE ALSO

    ffi_alloc(3), ffi_struct_layout(3), ffi_read(3)
