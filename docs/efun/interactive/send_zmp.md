---
title: interactive / send_zmp
---
# send_zmp

### NAME

    send_zmp() - send a ZMP message to the interactive client

### SYNOPSIS

    void send_zmp(string command, string *args);

### DESCRIPTION

    Sends a ZMP (Zenith MUD Protocol) message to the command_giver's client.
    'command' is the ZMP command name and 'args' is its argument list. Only the
    string elements of 'args' are transmitted; non-string elements are skipped.

    If there is no interactive command_giver, send_zmp() does nothing and a
    warning is emitted.

    Note: send_zmp() should only be called from within a user object.

### SEE ALSO

    has_zmp(3), send_gmcp(3), send_mxp(3)
