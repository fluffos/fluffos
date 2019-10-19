---
layout: default
title: parsing / process_string
---

### NAME

    process_string() - give a string with replaced calldescriptions

### SYNOPSIS

    string process_string( string combinestring );

### DESCRIPTION

    Processes  a  string by replacing specific syntactic patterns with what
    is returned when the pattern is interpreted as a function call descrip‐
    tion.

    The syntactic patterns are on the form:

          "@@function[:filename][|arg1|arg2....|argN]@@"

    This is interpreted as a call:

           filename->function(arg1, arg2, ....., argN)

    Note  that  process_string  does  not recurse over returned replacement
    values. If a function returns another syntactic pattern, that  descrip‐
    tion will not be replaced.

    All  such  occurrences  in 'combinestring' is processed and replaced if
    the return value is a string. If the return value is not a  string  the
    pattern will remain unreplaced.

    Note that both object and arguments are marked optional with the brack‐
    ets and that the brackets are not included in the actual pattern.

### SEE ALSO

    process_value(3)

### CAVEAT

    This is usually used to support 'value by function call' in the mudlib.
    It  is  wise  to  set  the  effuserid  of  the object to 0 before using
    process_value as any function in any object can be called  with  almost
    any arguments.

### EXAMPLE

    A string:
        "You are chased by @@query_the_name:/obj/monster#123@@ eastward."

    is replaced by:
        "You are chased by the orc eastward."

    Assuming that query_the_name in monster#123 returns "the orc".

