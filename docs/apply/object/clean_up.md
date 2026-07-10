---
title: object / clean_up
---
# clean_up

### NAME

    clean_up - periodically called in inactive objects

### SYNOPSIS

    int clean_up( int inherited );

### DESCRIPTION

    The  clean_up()  function is called by the driver on a regular basis in
    all objects  that  have  been  inactive  for  the  time  specified  for
    clean_up()  in  the  runtime configuration file.

    The argument is 0 for clones. For blueprints it is the program's
    current reference count: 1 for a plain loaded object nothing else
    uses, and greater when clones of it exist, other programs inherit it,
    or the driver's function-call cache still holds a reference to it.
    Treat values above 1 as a hint that the program is (or was recently)
    in use, not as an exact inheritance count, and avoid destructing in
    that case.

    If clean_up() returns 0, clean_up() will never be called again on that
    object unless the object asks for it again with request_clean_up().
    If it returns 1, it will be called again when the object remains
    inactive for the specified clean_up() delay.

    One  thing that might be commonly done by an object in this function is
    destructing itself to conserve memory.

### SEE ALSO

    request_clean_up(3)

