---
title: ffi / ffi_struct_layout
---
# ffi_struct_layout

### NAME

    ffi_struct_layout() - compute a C struct's field layout

### SYNOPSIS

    mixed *ffi_struct_layout( int *field_types );

### DESCRIPTION

    Given an array of scalar type codes (from <ffi.h>), one per struct
    field, returns `({ total_size, ({ offset0, offset1, ... }) })`
    honoring the platform's alignment rules. Size an ffi_alloc() block to
    `total_size` and use the offsets with ffi_read()/ffi_write(). The
    tools/ffi generator emits these arrays from a C header.

### SEE ALSO

    ffi_alloc(3), ffi_read(3), ffi_write(3), ffi_sizeof(3)
