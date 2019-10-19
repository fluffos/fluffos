---
layout: default
title: internals / dump_socket_status
---

### NAME

    dump_socket_status() - display the status of each LPC socket

### SYNOPSIS

    void dump_socket_status( void );

### DESCRIPTION

    dump_socket_status()  is  a diagnostic facility which displays the cur‐
    rent status of all LPC sockets configured into the MudOS driver.  It is
    useful  for debugging LPC sockets applications.  Each row in the output
    corresponds to a single LPC socket.  The first row corresponds  to  LPC
    socket descriptor 0, the second row, 1, etc.  The total number of sock‐
    ets is configured when the driver is built.

    The first column "Fd" is the operating system file  descriptor  associ‐
    ated  with the LPC socket.  "State" is the current operational state of
    the LPC socket.  "Mode" is the socket mode, which is passed as an argu‐
    ment to socket_create().  The local and remote addresses are the Inter‐
    net address and port numbers in Internet dot notations.  '*'  indicates
    an  address  or  which  is  0.  N.B. LPC sockets that are in the CLOSED
    state are not currently in use; therefore the data displayed  for  that
    socket may be idiosyncratic.

    The  following  output  was generated on Portals, where the only socket
    application running at the time was MWHOD.  It indicates that two sock‐
    ets  are  current in use, one is listening for connection requests on a
    STREAM mode socket.  The other is waiting for incoming data on a  DATA‐
    GRAM mode socket.

    Fd    State      Mode      Local Address      Remote Address
    --  ---------  --------  -----------------  ------------------
    13   LISTEN     STREAM   *.6889             *.*
    14    BOUND    DATAGRAM  *.6888             *.*
    -1    CLOSED      MUD    *.*                *.*

### SEE ALSO

    debug_info(3), dump_file_descriptors(3)

