---
layout: doc
title: ffi / ffi_call
---
# ffi_call

### NAME

    ffi_call() - call a prepared C function

### SYNOPSIS

    mixed ffi_call( int func, mixed *args );

### DESCRIPTION

    Calls the function handle `func` returned by ffi_prepare(). `args`
    must have exactly the prepared argument count; each element is an
    int, a float, or a buffer (never a string — LPC strings are
    UTF-8-native and never implicitly marshalled to char*), matching the
    prepared argument type codes.

    The return value follows `ret_type`: an int or float for scalar
    types, a buffer for an owned FFI_POINTER result or an int address for
    a raw foreign pointer, and 0 for FFI_VOID. Argument-count or -type
    mismatches raise an error rather than calling C with a bad frame.

### SEE ALSO

    ffi_prepare(3), ffi_peek(3), ffi_alloc(3), string_encode(3), string_decode(3)
