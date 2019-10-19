---
layout: default
title: interactive / message
---

### NAME

    message() - deliver messages to "living" objects

### SYNOPSIS

    void message( mixed class, string message, mixed target,
                  mixed exclude );

### DESCRIPTION

    message()  calls  receive_message(mixed  class,  string message) in all
    objects in the target list excluding those in the  exclude  list.  This
    basically tells the object the message.

    Class  is  the  type of message (used for clients and such). An example
    would be 'combat', 'shout', 'emergency' etc.

    Message is a string containing the text to send.

    Target is a list of objects to be sent the message. This can be  either
    a single object string or object pointer, or may be an array of either.
    If a target is non-living all objects in its environment  will  receive
    the message.

    Exclude  is a list of objects that should not receive the message. This
    can either be one object or an array of objects.

### SEE ALSO

    say(3), write(3), shout(3), tell_object(3), tell_room(3)

