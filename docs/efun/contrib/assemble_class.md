---
layout: default
title: contrib / assemble_class.pre
---

### NAME

    assemble_class

### SYNOPSIS

    mixed assemble_class( mixed *elements );

### DESCRIPTION

    Takes an array of <elements> and returns an instantiated class.

### EXAMPLE

    mixed *elements = ({ "Foo", 42, "Fooville" }) ;
    mixed cl = assemble_class( elements ) ;

    write( sprintf( "%O\n", cl ) ) ;

    // CLASS( 3 elements
    //   "Foo",
    //   42,
    //   "Fooville"
    //  )

### WARNING

    If you have a previously defined class and assign the result from
    assemble_class to a variable, the variable will be shaped from the
    result of assemble_class. No error is caught by the shapes being
    different, and the original specification is ignored.

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        class person me ;

        me = assemble_class( ({ "Would", "You", "Like", 42, "Bubblegums" }) ) ;
        write( sprintf( "%O\n", me ) ) ;    

        // CLASS( 5 elements
        //   "Would",
        //   "You",
        //   "Like",
        //   42,
        //   "Bubblegums"
        //  )
    }


### SEE ALSO

    disassemble_class(3)
