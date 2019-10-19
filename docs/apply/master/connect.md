---
layout: default
title: master / connect
---

### NAME

    connect - get an object for a new user

### SYNOPSIS

    object connect( void );

### DESCRIPTION

    The  driver  calls  connect()  in the master object whenever a new user
    logs into the driver.  The object returned by connect() is used as  the
    initial user object.  Note that it is possible to use exec(3) to switch
    the user connection from the initial user object to some other object.

### SEE ALSO

    exec(3), logon(4)

