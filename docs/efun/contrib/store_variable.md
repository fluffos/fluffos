---
layout: default
title: contrib / store_variable.pre
---

### NAME

    store_variable - store a value in an object's global variable

### SYNOPSIS

    void store_variable(string variable_name, mixed value, object ob | void);

### DESCRIPTION

    This efun stores the value in the global variable variable_name in ob.
    The variable must not be private.

    variable_name is name of the global variable
    value is the data to be stored in the global variable
    ob defaults to this_object() if not specified

    This is a potential security hazard and, therefore, you may wish to overload
    this function to perform security checks.

### EXAMPLE

    store_variable( "weight", 150, this_player() ) ;

### SEE ALSO

    fetch_variable
