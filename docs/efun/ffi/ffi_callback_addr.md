---
layout: doc
title: ffi / ffi_callback_addr
---
# ffi_callback_addr

### NAME

    ffi_callback_addr() - raw address of a callback closure

### SYNOPSIS

    int ffi_callback_addr( int cb );

### DESCRIPTION

    Returns the raw native code address of the closure for callback
    handle `cb` (from ffi_callback()), as an integer — the value to hand
    to C as an FFI_POINTER argument.

### SEE ALSO

    ffi_callback(3), ffi_callback_free(3), ffi_call(3)
