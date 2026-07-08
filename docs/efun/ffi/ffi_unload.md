---
layout: doc
title: ffi / ffi_unload
---
# ffi_unload

### NAME

    ffi_unload() - release a library handle

### SYNOPSIS

    void ffi_unload( int lib );

### DESCRIPTION

    Releases a library handle returned by ffi_load(). Function handles
    prepared from it become invalid. Libraries are also released when the
    driver shuts down.

### SEE ALSO

    ffi_load(3), ffi_status(3)
