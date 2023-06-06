---
layout: default
title: async / async_write
---

### NAME

    async_write() - appends a string to a file then executes a callback

### SYNOPSIS

    void async_write( string file, string str, int flag, function callback );

### DESCRIPTION

    Append  the string 'str' into the file 'file'. If flag is 1, write_file
    overwrites instead of appending.

    Unlike write_file, which returns 0 for failure or 1 for success, this efun
    will return -1 for failure and 0 for success to the callback.

    The callback should follow this format:

        function (int res) {
            // -1 for failure
            //  0 for success
        }

### SEE ALSO

    file_size(3), read_file(3), write_buffer(3), write_file(3), async_read(3),
    valid_read(4), valid_write(4)

