---
layout: default
title: objects / save_object
---

### NAME

    save_object() - save the values of variables in an object into a file

### SYNOPSIS

    int save_object( string name, int flag );

### DESCRIPTION

    Save  all  values  of  non-static  variables in this object in the file
    'name'.  valid_write() in the master object determines whether this  is
    allowed.   The  optional  second argument is a bitfield: If bit 0 is 1,
    then variables that  are  zero  (0)  are  also  saved  (normally,  they
    aren't).   Object  variables always save as 0.  If bit 1 is 1, then the
    save file will be compressed.

### RETURN VALUE

    save_object() returns 1 for success, 0 for failure.

### SEE ALSO

    restore_object(3)

