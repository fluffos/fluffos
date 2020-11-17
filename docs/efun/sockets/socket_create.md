---
layout: default
title: sockets / socket_create
---

### NAME

    socket_create() - create an efun socket

### SYNOPSIS

    #include <socket_err.h>

    int socket_create( int mode, string read_callback,
                       void | string close_callback );

### DESCRIPTION

    socket_create()  creates  an efun socket. mode determines which type of
    socket is created. Currently supported socket modes are:

    0  MUD             for sending LPC data types using TCP protocol.

    1  STREAM          for sending raw data using TCP protocol.

    2  DATAGRAM        for using UDP protocol.

    3  STREAM_BINARY   for exchange binary message with TCP protocol.

    4  DATAGRAM_BINARY for exchange binary message with UDP protocol.

    The argument read_callback is the name of a function for the driver  to
    call  when the socket gets data from its peer. The read callback should
    follow this format:

        void read_callback(int fd, mixed message, string addr)

    Where fd is the socket which received the data, and message is the data
    which was received, addr is the client address.

    In non-binary mode, message will be sanitized with UTF8 encoding and returned as an string.

    In binary mode, raw messages will be returned as a buffer.

    The argument close_callback is the name of a function for the driver to
    call if the socket closes unexpectedly, i.e. not as  the  result  of  a
    socket_close(3) call. The close callback should follow this format:

        void close_callback(int fd)

    Where  fd  is the socket which has closed.  NOTE: close_callback is not
    used with DATAGRAM mode sockets.

### RETURN VALUES

    socket_create() returns:

        a non-negative descriptor on success.

        a negative value indicated below on error.

### ERRORS

    EEMODENOTSUPP  Socket mode not supported.

    EESOCKET       Problem creating socket.

    EESETSOCKOPT   Problem with setsockopt.

    EENONBLOCK     Problem setting non-blocking mode.

    EENOSOCKS      No more available efun sockets.

    EESECURITY     Security violation attempted.

### SEE ALSO

    socket_accept(3), socket_bind(3),  socket_close(3),  socket_connect(3),
    socket_listen(3), socket_write(3)
