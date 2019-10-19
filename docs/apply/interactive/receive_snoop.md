---
layout: default
title: interactive / receive_snoop
---

### NAME

    receive_snoop - catch incoming snoop text

### SYNOPSIS

    void receive_snoop( string message) ;

### DESCRIPTION

    If  RECEIVE_SNOOP  is defined in options.h, whenever a user is snooping
    another user, all snoop text is sent to  receive_snoop()  in  his  user
    object.  Inside of this function, you can do as you wish with the text.
    A common activity would be to receive() it.

### SEE ALSO

    catch_tell(4), receive(3), receive_message(4)

