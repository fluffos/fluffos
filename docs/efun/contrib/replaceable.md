---
layout: default
title: contrib / replaceable.pre
---

### NAME

    replaceable - check if object is replaceable

### SYNOPSIS

    int replaceable(object, void | string *func);

### DESCRIPTION

    string *func defaults to ({ "create", "__INIT" }) and contains a list of functions which
    may be ignored for checking

    checks if object defines any functions itself (beside create and __INIT)

    returns 1 if no and object is not simul_efun object and not ???
    0 otherwise

### SEE ALSO

    replace_program(3), query_replaced_program(3)
