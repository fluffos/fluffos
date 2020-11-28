---
layout: default
title: contrib / store_class_member.pre
---

### NAME

    store_class_member

### SYNOPSIS

    mixed store_class_member(mixed instantiated_class, int class_element, mixed value);

### DESCRIPTION

    Store <value> in the <class_element>th member of an <instantiated_class>.

    Returns an updated version of the <instantiated_class>.

### EXAMPLE

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        class person me = new(class person) ;
        
        me->name = "Foo" ;
        me->age = 42 ;
        me->city = "Fooville" ;

        write( sprintf("%O\n", me) ) ;
        
        // Result:
        // CLASS( 3 elements
        //   "Foo",
        //   42,
        //   "Fooville"
        //  )
        
        me = store_class_member( me, 1, 43 ) ;

        write( sprintf("%O\n", me) ) ;

        // Result:
        // CLASS( 3 elements
        //   "Foo",
        //   43,
        //   "Fooville"
        //  )
    }

### SEE ALSO

    fetch_class_member(3)
