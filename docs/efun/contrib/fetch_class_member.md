---
layout: default
title: contrib / fetch_class_member.pre
---

### NAME

    fetch_class_member

### SYNOPSIS

    mixed fetch_class_member( mixed instantiated_class, int index );

### DESCRIPTION

    Given an <instantiated_class>, return the value of the <index>th
    member. This is particularly useful when you have an anonymous 
    class with no labels for the data members.

### EXAMPLE

    mixed me = assemble_class( ({ "Foo", 42, "Fooville" }) ) ;
    int age ;
    string name, city ;

    name = fetch_class_member( me, 0 ) ;
    age = fetch_class_member( me, 1 ) ;
    city = fetch_class_member( me, 2 ) ;

### SEE ALSO

    store_class_member(3)
