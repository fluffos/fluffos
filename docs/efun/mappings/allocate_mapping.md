---
layout: default
title: mappings / allocate_mapping
---

### NAME

    allocate_mapping() - pre-allocate space for a mapping

### SYNOPSIS

    mapping allocate_mapping( int size );
    mapping allocate_mapping(mixed *key, mixed value);

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

    allocate_mapping(mixed *keys, mixed v):
      - if value is an array, the returned array has keys 'keys' and values
       'value' (like 3.2.x's mkmapping)
      - if it is a function, the the mapping has keys 'keys' and values
        evaluate(value, key)
      - otherwise, each key has the value 'value'

### EXAMPLE

    allocate_mapping(0) ;
    // ([ ])

    allocate_mapping(25) ;
    // ([ ])

    allocate_mapping( ({ 1, 2, 3 }), ({ "one", "two", "three" }) )
    // ([ 1 : "one", 2 : "two", 3 : "three" ])

    allocate_mapping( users(), (: $1->query_name() :))
    // ([
    //     OBJ(karahd /std/user#9) : "Karahd",
    //     OBJ(gesslar /std/user#2) : "Gesslar"
    // ])

    allocate_mapping( ({ "apple", "banana", "pear" }), 25)
    // ([ "pear" : 25, "apple" : 25, "banana" : 25 ])

### SEE ALSO

    map_delete(3), allocate(3)

