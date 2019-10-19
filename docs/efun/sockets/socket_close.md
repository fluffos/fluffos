---
layout: default
title: sockets / socket_close
---

### NAME

    socket_close() - close a socket

### SYNOPSIS

    #include <socket_err.h>

    int socket_close( int s );

### DESCRIPTION

    socket_close() closes socket s. This frees a socket efun slot for use.

### RETURN VALUES

    socket_close() returns:

        EESUCCESS on success.

        a negative value indicated below on error.

### ERRORS

    EEFDRANGE      Descriptor out of range.

    EEBADF         Descriptor is invalid.

    EESECURITY     Security violation attempted.

### SEE ALSO

    socket_accept(3), socket_create(3)

