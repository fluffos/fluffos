---
layout: doc
title: objects / new
---
# new

### NAME

    new() - load a new copy of an object/class

### SYNOPSIS

    object new( string filename, ... );
    class ClassName new(class ClassName, ...) ;

### DESCRIPTION

    Clones a new object from `filename`, and give it a new unique name. Returns
    the new object. Optionally, additional arguments may be passed to new()
    which will be passed to the 'create()' apply in the new object.

    If the first argument is a class, then a new copy of the class is created
    and returned. Optionally, additional parameters may be passed to new()
    which will be used to initialize values in the returned class. If you are
    passing arguments to the class, you must specify the member name and the
    value of the members you would like to initialize.

### EXAMPLE

    Object example:

    object ob = new("/obj/torch") ; // clone a torch object
    object money = new("/obj/money", 10, "dollars" ) ; // clone a money object and set initial values

    Class example:

    class ClassPerson {
        string name ;
        int age ;
    }

    class ClassPerson person = new(class ClassPerson) ;
    person.name = "Bob" ;
    person.age = 42 ;

    // or

    class ClassPerson person = new(class ClassPerson, name: "Bob", age: 42) ;

### SEE ALSO

    clone_object(3), destruct(3), move_object(3)
