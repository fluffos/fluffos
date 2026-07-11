---
title: general / compress_file
---
# compress_file

### NAME

    compress_file() - gzip-compress a file on disk

### SYNOPSIS

    int compress_file(string source, string dest);
    int compress_file(string source)

### DESCRIPTION

    Compresses the file `source` into `dest` using gzip, producing a
    standard gzip (.gz) file. If `dest` is omitted, it defaults to
    `source` with ".gz" appended.

    On success, the original `source` file is DELETED (unlinked); only the
    compressed `dest` file remains. Returns 1 on success and 0 on failure.

    When `dest` is omitted and `source` already ends in ".gz", the call
    fails and returns 0 (it will not double-compress). Both `source` and
    `dest` are subject to the driver's read/write path validation
    (valid_read for `source`, valid_write for `dest`); a rejected path
    causes the call to return 0.

    For in-memory (non-file) compression, use compress().

### SEE ALSO

    uncompress_file(3), compress(3)
