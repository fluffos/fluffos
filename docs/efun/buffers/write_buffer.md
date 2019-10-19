---
layout: default
title: buffers / write_buffer
---

### NAME

    write_buffer() - write a buffer to a file, or read into a buffer from a
    source

### SYNOPSIS

    int write_buffer( string | buffer dest, int start,
                      mixed source );

### DESCRIPTION

    If 'dest' is a file, then 'source' must be an int (and will be  written
    to the file in network-byte-order), a buffer, or a string, and 'source'
    will be written to the file 'dest' starting at byte # 'start'.

    If 'dest' is a buffer, then 'source' will be written  into  the  buffer
    starting  at  byte  # 'start' in the buffer.  If 'source' is an int, it
    will be written in network-byte-order.

### SEE ALSO

    read_buffer(3), allocate_buffer(3)

### AUTHOR

    Truilkan

