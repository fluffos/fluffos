---
layout: default
title: buffers / read_buffer
---

### NAME

    read_buffer() - read from a file and return a buffer, or return part of
    a buffer as a string

### SYNOPSIS

    string | buffer read_buffer( string | buffer src,
                                 int start,  int len );

### DESCRIPTION

    If 'src' is a string (filename), then the filename will be read, start‐
    ing  at  byte # 'start', for 'len' bytes, and returned as a buffer.  If
    neither argument is given, the entire file is read.

    If 'src' is a buffer, then characters are read from the  buffer  begin‐
    ning  at  byte  #  'start' in the buffer, and for 'len' # of bytes, and
    returned as a string.

    Note that the maximum number of bytes you can read from a file and into
    a buffer is controlled via the 'maximum byte transfer' parameter in the
    runtime config file.

### SEE ALSO

    write_buffer(3),   allocate_buffer(3),    bufferp(3),    read_bytes(3),
    write_bytes(3)

### AUTHOR

    Truilkan

