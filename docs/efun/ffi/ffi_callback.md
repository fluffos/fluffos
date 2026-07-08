---
layout: doc
title: ffi / ffi_callback
---
# ffi_callback

### NAME

    ffi_callback() - expose an LPC function to C as a callback

### SYNOPSIS

    int ffi_callback( function fn, int ret_type, int *arg_types );

### DESCRIPTION

    Wraps the LPC function pointer `fn` in a libffi closure with the
    given return and argument type codes (from <ffi.h>), and returns a
    callback handle. Pass ffi_callback_addr() of that handle to C as an
    FFI_POINTER argument so a C library (a comparator for qsort(), an
    event handler, ...) can call back into LPC.

    Gated by the master apply valid_ffi("callback", 0, caller).

### SEE ALSO

    ffi_callback_addr(3), ffi_callback_free(3), ffi_call(3), valid_ffi(4)
