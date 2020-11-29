---
layout: default
title: contrib / base_name.pre
---

### NAME

    base_name - return the base name without object id (OID)

### SYNOPSIS

    string base_name( string text ) ;
    string base_name( object ob ) ;
    string base_name() ;

### DESCRIPTION

    In the first version, return the segment of the <text> up to, but
    not including the first occurrence of #. 

    Like the first version, but performs file_name on <ob> first.

    Like the second version, but the object is this_object().

### EXAMPLE

    string path = base_name( "/std/object#123" ) ;  // "/std/object" 
    
    object ob = new("/std/object) ;
    string path = base_name( ob ) ;                 // "/std/object" 
    
    // in /std/object
    string path = base_name() ;                     // "/std/object"

### SEE ALSO

    file_name(3)
