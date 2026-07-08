---
layout: doc
title: ffi / ffi_symbol
---
# ffi_symbol

### NAME

    ffi_symbol() - resolve a symbol to its raw address

### SYNOPSIS

    int ffi_symbol( int lib, string name );

### DESCRIPTION

    Resolves the symbol `name` in library `lib` and returns its raw
    native address as an integer, or 0 if not found.

    To *call* a C function you do not need this — use ffi_prepare(), which
    resolves the symbol itself. ffi_symbol() is for taking the address of
    a data symbol (read it with ffi_peek()) or of a function to pass to C
    as an FFI_POINTER argument.

    Gated by the master apply valid_ffi("symbol", name, caller).

### SEE ALSO

    ffi_prepare(3), ffi_peek(3), ffi_address(3), valid_ffi(4)
