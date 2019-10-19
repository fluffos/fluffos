---
layout: default
title: interactive / catch_tell
---

### NAME

    catch_tell - provides an interface to interact with users

### SYNOPSIS

    void catch_tell( string message );

### DESCRIPTION

    If  INTERACTIVE_CATCH_TELL is defined in options.h, whenever the driver
    has something to tell an object due to say(),  shout(),  tell_object(),
    etc.   catch_tell  in the player object will be called with the message
    to be printed.  The message can be displayed, discarded, or modified in
    any way desired.  This is a useful way to have flexible earmuffs, or to
    support intelligent clients.

### SEE ALSO

    message(3), receive(3), receive_message(4)

