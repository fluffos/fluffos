---
layout: default
title: interactive / gmcp
---

### NAME

    gmcp - provides an interface to GMCP data received from the client

### SYNOPSIS

    void gmcp( string message ) ;

### DESCRIPTION

    This apply is called when the driver receives GMCP data from the client. 
    This function should be placed in an interactive object and may be used 
    to process the data in a similar manner to receive_message.

### SEE ALSO

    gmcp_enable(4), has_gmcp(3), send_gmcp(3)
