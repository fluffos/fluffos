---
layout: default
title: async / async_getdir
---

### NAME

    async_getdir() - returns information pertaining to a filesystem directory

### SYNOPSIS

    void async_getdir( string dir, function callback );

### DESCRIPTION

    If  'dir' is a filename ('*' and '?' wildcards are supported), an array
    of strings is returned to the callback containing all filenames that match
    the specification. If 'dir' is a directory name (ending with a slash--ie:
    "/u/", "/adm/", etc), all filenames in that directory are returned.

    Unlike the get_dir routine, this efun does not take an integer second
    argument to specify more information (filename, filesize, last touched).

    The callback should follow this format:

        function(mixed res) {
            // 0 when directory doesn't exist
            // empty array when no matching files exist
            // array of matching filenames
        }

### SEE ALSO

    file_size(3), get_dir(3), stat(3), time(3)

