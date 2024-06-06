---
layout: doc
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
