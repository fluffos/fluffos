---
title: general / uncompress
---
# uncompress

### NAME

    uncompress() - zlib-decompress a buffer in memory

### SYNOPSIS

    buffer uncompress(buffer data)

### DESCRIPTION

    Inflates `data`, a buffer produced by compress(), and returns the
    decompressed bytes as a buffer. The argument must be a buffer; if it
    is not, uncompress() returns undefined (0).

    `data` must be a valid, complete zlib stream. If inflation fails (the
    data is corrupt, truncated, or not a zlib stream), uncompress() raises
    a runtime error rather than returning.

    This is the counterpart to compress(). To decompress gzip files on
    disk, use uncompress_file().

### EXAMPLE

    buffer packed = compress("hello world");
    buffer original = uncompress(packed);
    // original now holds the bytes "hello world"

### SEE ALSO

    compress(3), uncompress_file(3)
