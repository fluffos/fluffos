---
layout: default
title: sockets / socket_accept
---

### NAME

    socket_accept() - accept a connection on a socket

### SYNOPSIS

    #include <socket_err.h>

    int socket_accept( int s, string read_callback,
                       string write_callback );

### DESCRIPTION

    The argument s is a socket that has been created with socket_create(3),
    bound to an address with socket_bind(3), and is listening  for  connec‐
    tions after a socket_listen(3). socket_accept() extracts the first con‐
    nection on the queue of pending connections, creates a new socket  with
    the  same  properties  of s and allocates a new file descriptor for the
    socket.  If  no  pending  connections  are  present   on   the   queue,
    socket_accept()  returns  an  error  as  described  below. The accepted
    socket is used to read and write data to and from the socket which con‐
    nected  to  this  one;  it  is not used to accept more connections. The
    original socket s remains open for accepting further connections.

    The argument read_callback is the name of a function for the driver  to
    call when the new socket (not the accepting socket) receives data.  The
    write callback should follow this format:

        void read_callback(int fd)

    Where fd is the socket which is ready to accept data.

    The argument write_callback is the name of a function for the driver to
    call  when  the  new  socket  (not the accepting socket) is ready to be
    written to. The write callback should follow this format:

        void write_callback(int fd)

    Where fd is the socket which is ready to be written to.

    Note: The close_callback of the accepting socket (not the  new  socket)
    is called if the new socket closes unexpectedly, i.e. not as the result
    of a socket_close(3) call. The close callback should follow  this  for‐
    mat:

        void close_callback(int fd)

    Where fd is the socket which has closed.

### RETURN VALUES

    socket_accept()  returns  a  non-negative  descriptor  for the accepted
    socket  on  success.  On  failure,  it  returns   a   negative   value.
    socket_error(3)  can be used on the return value to get a text descrip‐
    tion of the error.

### ERRORS

    EEFDRANGE      Descriptor out of range.

    EEBADF         Descriptor is invalid.

    EESECURITY     Security violation attempted.

    EEMODENOTSUPP  Socket mode not supported.

    EENOTLISTN     Socket not listening.

    EEWOULDBLOCK   Operation would block.

    EEINTR         Interrupted system call.

    EEACCEPT       Problem with accept.

    EENOSOCKS      No more available efun sockets.

### SEE ALSO

    socket_bind(3), socket_connect(3), socket_create(3), socket_listen(3)

