---
layout: doc
title: filesystem / read_file
---
# read_file

### NAME

    read_file() - read a file into a string

### SYNOPSIS

    string read_file( string file, int start_line, int number_of_lines );

### DESCRIPTION

    Read  a  line  of text from a file into a string.  The second and third
    arguments are optional.  If only the first argument is  specified,  the
    entire file is returned (as a string).

    The  start_line  is the line number of the line you wish to read.  This
    routine will return 0 if you try to read past the end of the  file,  or
    if you try to read from a nonpositive line.

### SEE ALSO

    file_size(3), read_buffer(3), write_file(3), async_read(3), async_write(3),
    valid_read(4), valid_write(4)

