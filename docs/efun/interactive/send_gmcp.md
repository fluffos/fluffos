---
layout: default
title: interactive / send_gmcp
---

### NAME

    send_gmcp() - send a GMCP message

### SYNOPSIS

    void send_gmcp( string message );

### DESCRIPTION

    Sends a GMCP 'message' for this_object() to the user's client. 

    For payload formatting information see: http://www.gammon.com.au/gmcp, 
    https://github.com/keneanung/GMCPAdditions

    Note: send_gmcp() should only be called from within a user object.
    
### SEE ALSO

    gmcp_enable(4), gmcp(4), has_gmcp(3)

