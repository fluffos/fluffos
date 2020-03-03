---
layout: default
title: apply / interactive / receive_message
---

### NAME

    receive_message - provides the interface used by the message efun.

### SYNOPSIS

    void receive_message( string class, string message );

### DESCRIPTION

    The  message()  efun calls this method in the player object.  The class
    parameter is typically used to indicate the class  (say,  tell,  emote,
    combat,  room  description, etc) of the message.  The receive_message()
    apply together with the message() efun can provide a good mechanism for
    interfacing with a "smart" client.

### SEE ALSO

    catch_tell(4), message(3), receive(3), receive_snoop(4)

