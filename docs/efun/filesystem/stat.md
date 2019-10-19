---
layout: default
title: filesystem / stat
---

### NAME

    stat() - returns information pertaining to a file or a directory

### SYNOPSIS

    mixed stat( string str, int default: 0 );

### DESCRIPTION

    If  str  is  the  name of a regular file (not a directory), then stat()
    will return an array of information pertaining to that file.  The  form
    of the array is as follows:

           ({ file_size, last_time_file_touched, time_object_loaded })

    If stat is called on a directory (not a regular file), or with a second
    argument of -1, then stat() behaves identically to get_dir(3).

### SEE ALSO

    get_dir(3), file_size(3)

