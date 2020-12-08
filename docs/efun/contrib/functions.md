---
layout: default
title: contrib / functions.pre
---

### NAME

    functions

### SYNOPSIS

    mixed *functions(object ob, int flag: 0);

### DESCRIPTION
    Returns an array of strings containing the function names found in
    <ob>. The functions returned will include all functions, whether public,
    protected, or private.

    The <flag> option is by default zero and shows only as described above,
    with just the string names of the functions. All available flag values
    are below with their behaviour.

    Flag: 0
    Returns: all function names in the deep_inherit_list in a string array 
    ({ "func1", "func2", "func3", "func4", ... })

    Flag: 1
    Returns: A two-dimensional array of mixed arrays containing additional
    information about each function. All functions are from the
    deep_inherit_list.

    ({
        ({
            "func1",    // function name
            2,          // number of parameters in function
            "int",      // return type of function
            "object",   // the first parameter
            "string *", // the second parameter
        }),
        ...
    })

    Flag: 2
    Like Flag 0, but only contains functions local to that object's file,
    and not from the deep_inherit_list.

    Flag: 3
    Like Flag 1, but only contains functions local to that object's file,
    and not from the deep_inherit_list.

### SEE ALSO

    variables(3)
