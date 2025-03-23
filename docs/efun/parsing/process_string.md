---
layout: doc
title: parsing / process_string
---

# process_string

### NAME

    process_string() - process a string containing function call descriptions

### SYNOPSIS

    string process_string(string str);

### DESCRIPTION

    Processes a string by replacing function call descriptions with their results.
    The function call descriptions are in the format:
    @@function:object#id@@

    Where:

    * function is the name of the function to call
    * object is the object to call the function on
    * id is the object's ID

    The function call will be replaced with the string result of calling that
    function on that object.

    Note: This function is different from parse_command() which uses OBJ, OBJ2,
    PREP, and WORD tokens for pattern matching. This function is specifically
    for evaluating function calls within strings.

    Example:

    ```c
    string str = "You are chased by @@query_the_name:/obj/monster#123@@ eastward.";
    string result = process_string(str);
    // If query_the_name in monster#123 returns "the orc":
    // result = "You are chased by the orc eastward."
    ```

### SEE ALSO

    process_value(3), parse_command(3)

### CAVEAT

    This is usually used to support 'value by function call' in the mudlib.
    It is wise to set the effuserid of the object to 0 before using
    process_value as any function in any object can be called with almost
    any arguments.

### EXAMPLE

    A string:
        "You are chased by @@query_the_name:/obj/monster#123@@ eastward."

    is replaced by:
        "You are chased by the orc eastward."

    Assuming that query_the_name in monster#123 returns "the orc".
