---
layout: default
title: object / move_or_destruct
---

### NAME

    move_or_destruct - ask an object to move to the specified destination

### SYNOPSIS

    int move_or_destruct( object dest );

### DESCRIPTION

    If  an  object's is destructed, prior to it's destruction this apply is
    called on it's contents.  'dest' will be the to be  destructed  object.
    If  the  target  object  of  this apply does not move itself out of the
    object being destructed, it will be destructed as well.

### SEE ALSO

    destruct(3), move_object(3), init(4)

