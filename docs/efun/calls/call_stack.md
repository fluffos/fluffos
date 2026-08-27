---
title: calls / call_stack
---
# call_stack

### NAME

    call_stack - returns information about the functions involved in calling
    this function

### SYNOPSIS

    mixed *call_stack(int option);

### DESCRIPTION

    If the int `option` argument is 0, call_stack() returns an array of the
    names of the on the call stack, with the first one being the most recent
    (i.e. the currently running program).  If the int argument is 1, call_stack
    returns the objects in which that program is executing.  If it is 2, the
    name of the functions are returned.  If it is 3, the value of origin() in
    that frame is returned. If it is 4, the value will be the full file path
    and the line number.

    A frame entered from DRIVER CONTEXT -- a promise reaction handler, the
    defer() teardown of an abandoned coroutine, a bare tick callback -- is
    running no program in no object.  Such a frame reports the file name
    "/<driver>" for option 0, 0 instead of an object for option 1, and an
    empty string for option 4; options 2 and 3 are unaffected (option 2
    reports "CATCH" or "<function>", as it does for any frame that is not a
    plain function call).

### SEE ALSO

    previous_object(3), origin(3)
