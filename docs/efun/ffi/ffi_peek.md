---
title: ffi / ffi_peek
---
# ffi_peek

### NAME

    ffi_peek() - copy bytes from a raw foreign address into a buffer

### SYNOPSIS

    buffer ffi_peek( int address, int nbytes );

### DESCRIPTION

    Copies `nbytes` bytes starting at the raw native `address` into a
    fresh owned buffer. This is the only way bytes behind a foreign
    pointer — for example a `char *` returned by a C function — become an
    LPC value. Pass -1 for `nbytes` to copy a NUL-terminated string up to
    an internal cap. Decode text with string_decode() once you have the
    bytes.

    Every call is gated by the master apply valid_ffi("peek", address,
    caller) before any memory is touched: reading an arbitrary native
    address is a process-memory disclosure primitive (and an unmapped
    address would crash the driver), so the master must approve it even
    though no foreign code runs.

### SEE ALSO

    ffi_call(3), ffi_symbol(3), ffi_address(3), string_decode(3),
    valid_ffi(4)
