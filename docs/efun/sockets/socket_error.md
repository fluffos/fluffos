---
layout: default
title: sockets / socket_error
---

### NAME

    socket_error() - return a text description of a socket error

### SYNOPSIS

    #include <socket_err.h>

    string socket_error( int error );

### DESCRIPTION

    socket_error()  returns  a  string  describing  the  error signified by
    error.

### RETURN VALUES

    socket_error() returns:

        a string describing the error on success.

        "socket_error: invalid error number" on bad input.

### SEE ALSO

    socket_create(3), socket_connect(3)

