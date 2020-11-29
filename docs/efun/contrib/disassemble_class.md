---
layout: default
title: contrib / disassemble_class.pre
---

### NAME

    disassemble_class

### SYNOPSIS

    mixed *disassemble_class( mixed instantiated_class );

### DESCRIPTION

    Given an <instantiated_class>, return a mixed array containing the
    values of all of the members.

### EXAMPLE

    class person {
        string name ;
        int age ;
        string city ;
    }

    void fun()
    {
        mixed *result ;
        class person me = new(class person);

        me->name = "Foo" ;
        me->age = 42 ;
        me->city = "Fooville" ;

        result = disassemble_class( me ) ;

        write( sprintf("%O\n", result ) ) ;

        // ({ /* sizeof() == 3 */
        //   "Foo",
        //   42,
        //   "Fooville"
        // })        
    }    

### SEE ALSO

    assemble_class(3)
