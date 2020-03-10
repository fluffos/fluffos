---
layout: default
title: sockets / socket_status
---

### NAME

    socket_status() - display the status of each LPC socket

### SYNOPSIS

    mixed *socket_status(void | int);

### DESCRIPTION

    socket_status() returns an array of arrays; one for each socket.

### RETURN VALUES

    socket_status(int fd) returns an array with:
        . ret[0] = (int) fd
        . ret[1] = (string) state
        . ret[2] = (string) mode
        . ret[3] = (string) local address
        . ret[4] = (string) remote address
        . ret[5] = (object) owner


### SEE ALSO

    socket_accept(3), socket_create(3)
