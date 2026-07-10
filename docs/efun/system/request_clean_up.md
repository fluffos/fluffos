---
title: system / request_clean_up
---
# request_clean_up

### NAME

    request_clean_up - ask the driver to resume clean_up() queries on an object

### SYNOPSIS

    int request_clean_up( object ob );

### DESCRIPTION

    When an object's clean_up() apply returns 0, the driver stops asking
    that object about cleaning up forever. request_clean_up() flags 'ob'
    (this_object() if omitted) so the periodic sweep will start calling
    its clean_up() again.

    Returns 1 if the request took effect, 0 if the object does not define
    a clean_up() apply (or is destructed).

### SEE ALSO

    clean_up(4), set_reset(3)
