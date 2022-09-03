---
layout: default
title: interactive / write
---

### NAME

    write() - send a message to current player

### SYNOPSIS

    void write( mixed str );

### DESCRIPTION

    Write  a  message  'str' to current player. 'str' can also be a number,
    which will be translated to a string.

    This efun does not invoke the receive_message() apply. so you may wish to
    override this efun if you wish it to be captured by this apply.

### SEE ALSO

    message(3), tell_object(3), tell_room(3), shout(3), say(3)
