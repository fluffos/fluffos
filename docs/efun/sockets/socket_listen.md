---
layout: doc
title: sockets / socket_listen
---
# socket_listen

### NAME

    socket_listen() - listen for connections on a socket

### SYNOPSIS

    #include <socket_err.h>

    int socket_listen( int s, string listen_callback );

### DESCRIPTION

    To accept connections, a socket is first created with socket_create(3),
    the socket is them put into listening mode with  socket_listen(3),  and
    the connections are accepted with socket_accept(3). The socket_listen()
    call applies only to sockets of type STREAM, MUD, STREAM_TLS, or
    STREAM_TLS_BINARY.

    The argument listen_callback is the name of a function for  the  driver
    to  call  when  a  connection is requested on the listening socket. The
    listen callback should follow this format:

        void listen_callback(int fd)

    Where fd is the listening socket.

    For TLS server sockets (STREAM_TLS or STREAM_TLS_BINARY), you must set
    the SO_TLS_CERT and SO_TLS_KEY options using socket_set_option() before
    calling socket_listen(). The certificate and key files must be in PEM
    format. When a client connects, the TLS handshake will be performed
    automatically when socket_accept() is called.

### RETURN VALUES

    socket_listen() returns:

        EESUCCESS on success.

        a negative value indicated below on error.

### ERRORS

    EEFDRANGE      Descriptor out of range.

    EEBADF         Descriptor is invalid.

    EESECURITY     Security violation attempted.

    EEMODENOTSUPP  Socket mode not supported.

    EENOADDR       Socket not bound to an address.

    EEISCONN       Socket is already connected.

    EELISTEN       Problem with listen.

### SEE ALSO

    socket_accept(3), socket_connect(3), socket_create(3)

