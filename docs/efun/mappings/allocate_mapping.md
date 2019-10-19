---
layout: default
title: mappings / allocate_mapping
---

### NAME

    allocate_mapping() - pre-allocate space for a mapping

### SYNOPSIS

    mapping allocate_mapping( int size );

### DESCRIPTION

    Returns a mapping with space for 'size' elements preallocated.

    For example:

        mapping x;
        int y = 200;

        x = allocate_mapping(y);

    where  y is the initial size of the mapping.  Using allocate_mapping is
    the preferred way to initalize the mapping if you have some idea of how
    many  elements  the map will contain (200 in this case).  The reason is
    that allocating storage all at once is slightly more memory  efficient.
    Thus  if  you  are using mappings to store a soul with 200 entries, the
    above initialization would be quite appropriate.  Note, that the  above
    initialization  does not restrict you to 200 entries.  It just that the
    first 200 entries will be stored more efficiently.  Note: if  you  will
    be  deleting many elements from the mapping, you should use x = ([]) to
    initialize the mapping rather than using allocate_mapping().

    Note: at this point in time, 'size' is meaningless, x  =  allocate_map‐
    ping(200); is equivalent to x = ([ ]);

### SEE ALSO

    map_delete(3)

