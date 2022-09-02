---
layout: default
title: interactive / shout
---

### NAME

    shout() - sends a message to all living objects

### SYNOPSIS

    void shout( string str );

### DESCRIPTION

    Sends the string 'str' to all living objects except this_player().

    This efun does not invoke the receive_message() apply, so you may wish to
    override this efun if you wish it to be captured by this apply.

### SEE ALSO

    message(3), write(3), tell_object(3), tell_room(3), say(3)

