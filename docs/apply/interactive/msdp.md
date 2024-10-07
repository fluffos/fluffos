---
layout: doc
title: interactive / gmcp
---
# gmcp

### NAME

    msdp - provides an interface to MSDP data received from the client

### SYNOPSIS

    void msdp( string message ) ;

### DESCRIPTION

    This apply is called when the driver receives MSDP data from the client. 
    This function should be placed in an interactive object and may be used 
    to process the data in a similar manner to receive_message.

### SEE ALSO

    msdp_enable(4), has_msdp(3), send_msdp_variable(3)
