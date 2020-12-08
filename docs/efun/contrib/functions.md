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
    protected, or private. The <flag> parameter is by default zero.

    Flag: 0
    Returns: a string array containing function names, including all inherited
    functions.
    ({ "func1", "func2", "func3", "func4", ... })

    Flag: 1
    Returns: A two-dimensional array of mixed arrays containing additional
    information about each function, including all inherited functions.

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
    Like Flag 0, but excludes inherited functions.

    Flag: 3
    Like Flag 1, but excludes inherited functions.

### SEE ALSO

    variables(3)
