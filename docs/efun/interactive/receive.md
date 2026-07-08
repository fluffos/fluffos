---
layout: doc
title: interactive / receive
---
# receive

### NAME

    receive() - displays a message to the current object

### SYNOPSIS

    void receive( string | buffer message );

### DESCRIPTION

    This  efun is an interface to the add_message() function in the driver.
    Its  purpose  is  to  display  a message to the current object. The
    message may be a string or a buffer of raw bytes.  Often, receive() is
    called from within catch_tell(4) or receive_message(4).

### SEE ALSO

    catch_tell(4), receive_message(4), message(3)

