---
layout: default
title: master / valid_read
---

### NAME

    valid_read - checks if a certain person has read permission on a file

### SYNOPSIS

    int valid_read( string file, mixed user, string func );

### DESCRIPTION

    Every time a user tries to read a file, thie driver calls valid_read in
    the master object to check if the read should be  allowed.   The  argu‐
    ments are the filename, the name of the person making the read, and the
    calling function name.  If valid_read returns  non-zero,  the  read  is
    allowed.

### SEE ALSO

    valid_write(4)

