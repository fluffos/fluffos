---
layout: doc
title: interactive / message
---
# message

### NAME

    message - deliver messages to "living" objects

### SYNOPSIS

    void message( mixed type, string message, mixed target, mixed exclude );

### DESCRIPTION

    message() calls the apply receive_message(mixed type, string message) in 
    all objects in the target list excluding those in the exclude list.  This
    provides the objects with the message, allowing them to take appropriate
    actions based upon the type of message received.

    The type of message can be used for filtering, formatting, adjusting, or 
    more. An example would be 'combat', 'shout', 'emergency', 'system', etc.

    Message is a string containing the text to send.

    Target is a list of objects to receive the message. This can be either
    a single object string or object pointer, or may be an array of either.
    If a target is non-living, all objects in its environment will receive
    the message.

    Exclude is a list of objects that should not receive the message. This
    can either be one object or an array of objects.

### EXAMPLE

```c
    message("say", "You say: Hello!", this_player());
    message("say", this_player()->query_cap_name() + " says: Hello!", environment(this_player()), ({ this_player() }));
```

### SEE ALSO

    say(3), write(3), shout(3), tell_object(3), tell_room(3), receive_message(4)

