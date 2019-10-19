---
layout: default
title: objects / move_object
---

### NAME

    move_object() - move current object to another environment

### SYNOPSIS

    void move_object( object item, mixed dest );

### DESCRIPTION

    Move  the  object  'item'  into  the  object  'dest'.   'item'  must be
    this_object().  move_object may be optionally called with one  argument
    in  which  case 'item' is implicitly this_object() and the passed argu‐
    ment is 'dest'.

### SEE ALSO

    this_object(3), move_or_destruct(4)

