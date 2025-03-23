---
layout: doc
title: parsing / process_value
---

# process_value

### NAME

    process_value() - return values from described function calls

### SYNOPSIS

    mixed process_value(string str);

### DESCRIPTION

    Processes a string containing a function call description and returns the
    result of evaluating that function call. The function call description
    is in the format:
    @@function:object#id@@

    Where:

    * function is the name of the function to call
    * object is the object to call the function on
    * id is the object's ID

    The function call will be evaluated and its result returned.

    Note: This function is different from parse_command() which uses OBJ, OBJ2,
    PREP, and WORD tokens for pattern matching. This function is specifically
    for evaluating a single function call and returning its result.

    Example:

    ```c
    mixed value = process_value("@@query_weight:/obj/weapon#456@@");
    // Returns the weight of the weapon object
    ```

### SEE ALSO

    process_string(3), parse_command(3)

### CAVEAT

    This is usually used to support 'value by function call' in the mudlib.
    It is wise to set the effuserid of the object to 0 before using
    process_value as any function in any object can be called with almost
    any arguments.
