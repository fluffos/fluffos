---
layout: default
title: interactive / send_gmcp
---

### NAME

    send_gmcp() - sends a message to all living objects

### SYNOPSIS

    void send_gmcp( string payload );

### DESCRIPTION

    Sends a GMCP data 'payload' for this_object() to the user's client. 

    For payload formatting information see: http://www.gammon.com.au/gmcp, 
    https://github.com/keneanung/GMCPAdditions

    Note: send_gmcp() should only be called from within a user object.
    
### SEE ALSO

    has_gmcp(3)

