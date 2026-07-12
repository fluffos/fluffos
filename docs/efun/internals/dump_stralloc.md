---
title: internals / dump_stralloc
---
# dump_stralloc

### NAME

    dump_stralloc() - report shared-string table statistics

### SYNOPSIS

    string dump_stralloc(string file);

### DESCRIPTION

    Writes statistics about the driver's shared-string table -- the
    string allocator -- to `file`, and also returns that same report as
    a string.

    `file` is validated for write access before use. The efun errors if
    the path is invalid or if the file cannot be opened for writing.

### SEE ALSO

    check_memory(3)
