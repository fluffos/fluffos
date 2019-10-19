---
layout: default
title: master / valid_write
---

### NAME

    valid_write - checks if a certain person has write permission on a file

### SYNOPSIS

    int valid_write( string file, mixed user, string func );

### DESCRIPTION

    Every  time a user tries to write a file, thie driver calls valid_write
    in the master object to check if the  write  should  be  allowed.   The
    arguments  are  the  filename, the name of the person making the write,
    and the calling function name.  If valid_write  returns  non-zero,  the
    write is allowed.

### SEE ALSO

    valid_read(4)

