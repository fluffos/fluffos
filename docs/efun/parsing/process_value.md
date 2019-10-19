---
layout: default
title: parsing / process_value
---

### NAME

    process_value() - give a value from a described function call

### SYNOPSIS

    mixed process_value( string calldescription );

### DESCRIPTION

    Get  the  replacement  of  one syntactic pattern. The pattern is on the
    form:

           "function[:filename][|arg1|arg2....|argN]"

    The returned value can be of any type.

    Note that both object and arguments are marked optional with the brack‐
    ets and that the brackets are not included in the actual pattern.

### SEE ALSO

    process_string(3)

### CAVEAT

    This is usually used to support 'value by function call' in the mudlib.
    It is wise to set the  effuserid  of  the  object  to  0  before  using
    process_value  as  any function in any object can be called with almost
    any arguments.

