---
layout: default
title: sockets / socket_address
---

### NAME

    socket_address() - return the remote address for an efun socket

### SYNOPSIS

    #include <socket_err.h>

    string socket_address( int s );

### DESCRIPTION

    socket_address()  returns the remote address for an efun socket s.  The
    returned address is of the form:

        "127.0.0.1 23".

### RETURN VALUES

    socket_address() returns:

        a string format address on success.

        an empty string on failure.

### SEE ALSO

    socket_connect(3),  socket_create(3),  resolve(3),  query_host_name(3),
    query_ip_name(3), query_ip_number(3)

