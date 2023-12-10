---
layout: doc
title: general / classp
---
# classp

### NAME

    classp

### SYNOPSIS

    int classp(mixed arg)

### DESCRIPTION

    Will return if the item `arg` is a class or not. If `arg` is a class,
    then the function will return 1, otherwise it will return 0.

### EXAMPLE
```c
class Person {
    string name;
    int age;
}

void runit() {
    class Person p = new(class Person, name: "Bob", age: 42);

    printf("classp(p) = %d", classp(p)); // 1
    printf("classp(0) = %d", classp(0)); // 0
    printf("classp(this_object()) = %d", classp(this_object())); // 0
}
```
