---
layout: doc
title: ffi / ffi_free
---
# ffi_free

### NAME

    ffi_free() - free a native memory block

### SYNOPSIS

    void ffi_free( buffer mem );

### DESCRIPTION

    Releases a buffer allocated by ffi_alloc(). Optional — the block is
    also reclaimed by the garbage collector when no LPC value references
    it. Using `mem` after freeing it is an error.

### SEE ALSO

    ffi_alloc(3)
