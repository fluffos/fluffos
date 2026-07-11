---
title: general / uncompress_file
---
# uncompress_file

### NAME

    uncompress_file() - gunzip a gzip file on disk

### SYNOPSIS

    int uncompress_file(string source, string dest);
    int uncompress_file(string source)

### DESCRIPTION

    Decompresses the gzip file `source` into `dest`. If `dest` is omitted,
    `source` must end in ".gz" and `dest` defaults to `source` with the
    ".gz" suffix stripped; if `source` does not end in ".gz" in that case,
    the call fails and returns 0.

    On success, the original `source` file is DELETED (unlinked); only the
    decompressed `dest` file remains. Returns 1 on success and 0 on
    failure. Both `source` and `dest` are subject to the driver's
    read/write path validation (valid_read for `source`, valid_write for
    `dest`); a rejected path causes the call to return 0.

    For in-memory (non-file) decompression, use uncompress().

### SEE ALSO

    compress_file(3), uncompress(3)
