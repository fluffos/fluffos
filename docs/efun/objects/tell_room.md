---
layout: default
title: objects / tell_room
---

### SYNOPSIS

    void tell_room( mixed ob, string str, object *exclude );

### DESCRIPTION

    Send  a message 'str' to object all objects in the room 'ob'.  'ob' can
    also be the filename of the room (string).  If 'exclude' is  specified,
    all objects in the exclude array will not receive the message.

### SEE ALSO

    message(3), write(3), shout(3), say(3), tell_object(3)

