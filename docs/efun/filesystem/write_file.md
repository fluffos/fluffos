---
layout: doc
title: filesystem / write_file
---
# write_file

### NAME

    write_file() - appends a string to a file

### SYNOPSIS

    int write_file( string file, string str, int flag );

### DESCRIPTION

    Append  the string 'str' into the file 'file'. Returns 0 or 1 for fail‐
    ure or success.  If flag is 1, write_file overwrites instead of append‐
    ing.

### SEE ALSO

    file_size(3), read_file(3), write_buffer(3), async_read(3), async_write(3),
    valid_read(4), valid_write(4)

