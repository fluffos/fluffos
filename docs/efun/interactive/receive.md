---
layout: default
title: interactive / receive
---

### NAME

    receive() - displays a message to the current object

### SYNOPSIS

    int receive( string message );

### DESCRIPTION

    This  efun is an interface to the add_message() function in the driver.
    Its purpose is to display a message to the current object.  It  returns
    1  if the current object is interactive, 0 otherwise.  Often, receive()
    is called from within catch_tell(4) or receive_message(4).

### SEE ALSO

    catch_tell(4), receive_message(4), message(3)

