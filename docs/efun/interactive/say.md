---
layout: default
title: interactive / say
---

### NAME

    say() - send a message to all users in the same environment

### SYNOPSIS

    varargs void say( string str, object obj );

### DESCRIPTION

    Sends  a message to the environment of the originator, all items in the
    same environment, and all items inside of the originator.  The origina‐
    tor  is  this_player(),  unless  this_player() == 0, in which case, the
    originator is this_object().

    The second argument is optional.  If second argument  'obj'  is  speci‐
    fied,  the  message  is  sent  to all except 'obj'.  If 'obj' is not an
    object, but an array of objects, all those objects  are  excluded  from
    receiving the message.

### SEE ALSO

    message(3), write(3), shout(3), tell_object(3), tell_room(3)

