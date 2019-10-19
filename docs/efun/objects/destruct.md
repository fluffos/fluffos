---
layout: default
title: objects / destruct
---

### NAME

    destruct() - remove an object from the games

### SYNOPSIS

    void destruct( object ob );

### DESCRIPTION

    Completely   destroy   and  remove  object  'ob'.  After  the  call  to
    destruct(), no global variables will exist any longer, only locals, and
    arguments.   If  'ob' is this_object(), execution will continue, but it
    is best to return a value immediately.

### SEE ALSO

    clone_object(3), new(3), destruct_env_of(4), move(4)

