---
title: ffi / ffi_callback_free
---
# ffi_callback_free

### NAME

    ffi_callback_free() - release a callback closure

### SYNOPSIS

    void ffi_callback_free( int cb );

### DESCRIPTION

    Releases the libffi closure for callback handle `cb`. Optional — the
    closure is also reclaimed by the GC. Do not free a callback while C
    still holds its address.

### SEE ALSO

    ffi_callback(3), ffi_callback_addr(3)
