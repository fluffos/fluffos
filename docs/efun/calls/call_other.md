---
layout: default
title: calls / call_other
---

### NAME

    call_other() - call a function in another object

### SYNOPSIS

    unknown call_other( object ob | object *obs,
                        string func | mixed *args, ... );

### DESCRIPTION

    Calls  a  function in another object, with [optional] argument(s).  The
    return value is returned from the other object, so it cannot  be  known
    at  compile  time  and may need to be cast if using type checking.  The
    function named 'func' will be called in 'ob', with arguments 3, 4,  etc
    given as arguments 1, 2, etc to 'func' in 'ob'.  call_other will return
    the return value of 'func'.  If the first argument is an array  instead
    of  an object, then the call will be done in all elements of that array
    (all elements should be of type object), and an array of  returns  will
    be  returned.   If argument 2 is an array instead of a string, then the
    first element in the array should be a string, the  function  to  call,
    and all other elements will be passed to the function in the order they
    appear in the array.

    There is a much more attractive way to do call_others:

        object ob | object *obs -> func ( ... );

    ie,

        call_other(ob, "query", "name");

    could be written as:

        ob->query("name");

    Using an array as second argument, the same call could be written:

        call_other(ob, ({ "query", "name" }));

    An example of using an array as the first argument:

        users()->quit();

