---
title: types / classes
---
# classes

Classes, or structs, are a way to group a collection of values together,
similiar to a mapping. A class holds a pre-defined number of member values 
which are accessed by their given name.

To use a class across multiple code files, it is ideal to have a shared
inheritable for the class definition. Do not use include statements.

Define what the class will look like:
```c
class Person {
    string name;
    int level;
}
```

Create an empty instance of the Person class:
```c
    class Person npc = new(class Person);
```

Create an instance of the Person class with passed values:
```c
    class Person npc = new(class Person, name: "Name", level: 15);
```

Accessing member values using dot operator:
```c
    npc.name = "Name";
    npc.level = 15;
```

Accessing member values using arrow operator:
```c
    npc->name = "Name";
    npc->level = 15;
```

Casting a class:
```c
function (mixed p, string name) {
    ((class Person)p)->name = name;
}
```

`classp(value)` is true for class instances. A class is a fixed layout
(named members), not a mapping: you cannot add keys at runtime, and two
instances of the same class do not share identity just because their
fields match.

To share a class definition across files, put the `class` declaration in
an inheritable and `inherit` it. Do not `#include` the declaration —
each include creates a distinct class type even if the members look the
same.

`new(class Person, name: "Ada", level: 15)` is the usual constructor.
`new("/obj/sword")` is unrelated: that form clones an **object**. See
[objects](../../concepts/general/objects).
