---
layout: default
title: async / async_read
---

### NAME

    async_read() - read a file into a string then executes a callback

### SYNOPSIS

    void async_read( string file, function callback );

### DESCRIPTION

    Read  a  line  of text from a file into a string.  Normally, read_file
    takes a second and third arguments for start_line and number_of_lines to
    read, but async_read will return the entire file to the callback.

    The callback should follow this format:

        function(mixed res) {
            // -1 for file not read
            // string file contents otherwise
        }

### SEE ALSO

    file_size(3), read_buffer(3), read_file(3), write_file(3), async_write(3),
    valid_read(4), valid_write(4)

