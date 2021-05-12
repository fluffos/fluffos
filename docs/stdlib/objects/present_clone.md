---
layout: default
title: stdlib / present_clone
---

### NAME

    present_clone() - finds an object in an inventory based on a file name or
    a blueprint

### SYNOPSIS

    object present_clone(string str)
    object present_clone(string str, int n)
    object present_clone(string str, object env)
    object present_clone(string str, object env, int n)

    object present_clone(object obj)
    object present_clone(object obj, int n)
    object present_clone(object obj, object env)
    object present_clone(object obj, object env, int n)

### DESCRIPTION

    This efun searches the inventory of object <env> (default is
    previous_object()) for an object with a specific blueprint.
    The blueprint can be specified either by name <str>, or as the same
    blueprint as of object <obj>. The matching criteria in both cases is
    the load_object(base_name(obj)).

    If <n> is given, the <n>th object in <env> is returned (if present),
    otherwise the first object matching <str> or <obj>.
    If no object in <env> matches the criteria, 0 is returned.

### EXAMPLE

    Assume that object 'env' contains the objects /obj/money#8,
    /std/weapon#9, /std/weapon#12 and /obj/key in the given order.

    +--------------------------------------------------+---------------+
    | Function call                                    | returns       |
    +--------------------------------------------------+---------------+
    | present_clone("/obj/money", env)                 | /obj/money#8  |
    | present_clone("/std/weapon#12", env)             | /std/weapon#9 |
    | present_clone(find_object("/obj/money#14"), env) | /obj/money#8  |
    | present_clone("/obj/key#18", env)                | /obj/key      |
    | present_clone("/std/weapon#12", env, 2)          | /std/weapon#12|
    | present_clone("/std/weapon#12", env, 3)          | 0             |
    +--------------------------------------------------+---------------+
