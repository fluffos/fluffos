---
layout: default
title: general / typeof
---

### NAME

    typeof() - return the type of an expression

### SYNOPSIS

    string typeof( mixed var );

### DESCRIPTION

    Return  the  type  of  an  expression.   The return values are given in
    <type.h>.  They are:

    T_INT           "int"
    T_STRING        "string"
    T_ARRAY         "array"
    T_OBJECT        "object"
    T_MAPPING       "mapping"
    T_FUNCTION      "function"
    T_FLOAT         "float"
    T_BUFFER        "buffer"
    T_CLASS         "class"

    T_INVALID          "*invalid*"
    T_LVALUE           "*lvalue*"
    T_LVALUE_BYTE      "*lvalue_byte*"
    T_LVALUE_RANGE     "*lvalue_range*"
    T_LVALUE_CODEPOINT "*lvalue_codepoint*"
    T_ERROR_HANDLER    "*error_handler*"
    T_FREED            "*freed*"
    T_UNKNOWN          "*unknown*"

### SEE ALSO

    allocate(3), allocate_mapping(3), strlen(3)

