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

### SEE ALSO

    ffi_call(3), ffi_symbol(3), ffi_address(3), string_decode(3)
