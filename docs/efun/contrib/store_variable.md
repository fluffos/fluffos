---
layout: doc
title: contrib / store_variable.pre
---
# store_variable

### NAME

    store_variable - store a value in an object's global variable

### SYNOPSIS

    void store_variable(string variable_name, mixed value, object ob | void);

### DESCRIPTION

    This efun stores the value in the global variable variable_name in `ob`.

    `variable_name` is name of the global variable.
    `value` is the data to be stored in the global variable.
    `ob` defaults to this_object() if not specified.

    If `ob` is not specified, then `variable_name` can be any global variable
    in the inheritance hierarchy, regardless of scope.  If `ob` is specified,
    then `variable_name` must be public scope. 

    This is a potential security hazard and, therefore, you may wish to overload
    this function to perform security checks.

### EXAMPLE

    store_variable( "weight", 150, this_player() ) ;

### SEE ALSO

    fetch_variable
