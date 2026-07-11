---
title: general / compress
---
# compress

### NAME

    compress() - zlib-compress a string or buffer in memory

### SYNOPSIS

    buffer compress(string | buffer data)

### DESCRIPTION

    Compresses `data` using zlib (a raw zlib/deflate stream) and returns
    the compressed bytes as a buffer. `data` may be either a string or a
    buffer; if it is neither, compress() returns undefined (0).

    This is in-memory compression, the counterpart to uncompress(). To
    recover the original bytes, pass the returned buffer to uncompress().
    For compressing files on disk (gzip format), use compress_file().

### EXAMPLE

    buffer packed = compress("the quick brown fox");
    buffer original = uncompress(packed);
    // original now holds the bytes "the quick brown fox"

### SEE ALSO

    uncompress(3), compress_file(3)
