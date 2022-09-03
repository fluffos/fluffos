---
layout: default
title: calls / call_stack
---

### NAME

    call_stack - returns information about the functions involved in calling
    this function

### SYNOPSIS

    string *call_stack(int option);

### DESCRIPTION

    If the int `option` argument is 0, call_stack() returns an array of the
    names of the on the call stack, with the first one being the most recent
    (i.e. the currently running program).  If the int argument is 1, call_stack
    returns the objects in which that program is executing.  If it is 2, the
    name of the functions are returned.  If it is 3, the value of origin() in
    that frame is returned.

### SEE ALSO

    previous_object(3), origin(3)
