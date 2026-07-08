---
layout: doc
title: objects / save_object
---
# save_object

### NAME

    save_object() - save the values of variables in an object into a file

### SYNOPSIS

    int save_object( string name, int flag );
    string save_object( int flag );

### DESCRIPTION

    Save  all  values  of  non-static  variables in this object in the file
    'name'.  valid_write() in the master object determines whether this  is
    allowed.   The  optional  second argument is a bitfield: If bit 0 is 1,
    then variables that  are  zero  (0)  are  also  saved  (normally,  they
    aren't).   Object  variables always save as 0.  If bit 1 is 1, then the
    save file will be compressed.

    If no file name is given (the argument is omitted or is the integer
    flag bitfield), the serialized data is not written to disk but is
    returned as a string instead, suitable for use with the string form of
    restore_object().

### RETURN VALUE

    With a file name, save_object() returns 1 for success, 0 for failure.
    In the no-file-name form it returns the save data as a string.

### SEE ALSO

    restore_object(3)

