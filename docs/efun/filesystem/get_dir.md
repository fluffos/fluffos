---
layout: default
title: filesystem / get_dir
---

### NAME

    get_dir() - returns information pertaining to a filesystem directory

### SYNOPSIS

    mixed *get_dir( string dir, int default: 0 );

### DESCRIPTION

    If  'dir' is a filename ('*' and '?' wildcards are supported), an array
    of strings is returned containing all filenames that match the specifi‐
    cation.   If 'dir' is a directory name (ending with a slash--ie: "/u/",
    "/adm/", etc), all filenames in that directory are returned.

    If called with a second argument equal to -1, get_dir  will  return  an
    array of subarrays, where the format of each subarray is:

          ({ filename, size_of_file, last_time_file_touched })

    Where  filename  is  a  string and last_time_file_touched is an integer
    being number of seconds since January 1, 1970 (same format as time(3)).
    The  size_of_file  element  is  the  same  value  that  is  returned by
    file_size(3); the size of the file in bytes, or -2 if it's a directory.

### SEE ALSO

    file_size(3), stat(3), time(3)

