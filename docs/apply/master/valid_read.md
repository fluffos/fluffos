---
title: master / valid_read
---
# valid_read

### NAME

    valid_read - checks if a certain person has read permission on a file

### SYNOPSIS

    int valid_read( string file, mixed user, string func );

### DESCRIPTION

    Every time a user tries to read a file, the driver calls valid_read in
    the master object to check if the read should be  allowed.   The  argu‐
    ments are the filename, the name of the person making the read, and the
    calling function name.  If valid_read returns  non-zero,  the  read  is
    allowed.

    This apply is also consulted in the middle of compiling an object: each
    #include directive checks the header path with valid_read (with <func>
    set to "include") before opening it. Returning 0 denies the include and
    fails that compile. Because the apply runs mid-compile, it must not
    trigger another compile itself (e.g. by loading an object). A runtime
    error thrown from valid_read (or anywhere else while a file is being
    compiled) aborts that compilation cleanly; the driver fully unwinds its
    compiler state and remains able to load other objects afterwards.

### SEE ALSO

    valid_write(4), include_file(4), inherit_program(4)

