---
layout: default
title: buffers / allocate_buffer
---

### NAME

    allocate_buffer() - allocate a buffer

### SYNOPSIS

    buffer allocate_buffer( int size );

### DESCRIPTION

    Allocate  a  buffer of <size> elements.  The number of elements must be
    >= 0 and not bigger than a system maximum (usually ~10000).   All  ele‐
    ments are initialized to 0.

### SEE ALSO

    bufferp(3), read_buffer(3), write_buffer(3), sizeof(3), to_int(3)

### AUTHOR

    Truilkan

