---
layout: default
title: contrib / fetch_variable.pre
---

### NAME

    fetch_variable - fetch a value stored in an object's global variable

### SYNOPSIS

    mixed fetch_variable(string variable_name, object ob | void);

### DESCRIPTION

    This efun returns the value stored in the global variable variable_name in ob.
    The variable must not be private.

    variable_name is name of the global variable
    ob defaults to this_object() if not specified

    This is a potential security hazard and, therefore, you may wish to overload
    this function to perform security checks.

### EXAMPLE

    int weight = fetch_variable( "weight", this_player() ) ;
    printf("%d\n", weight") ;
    
    // result: 150

### SEE ALSO

    store_variable
