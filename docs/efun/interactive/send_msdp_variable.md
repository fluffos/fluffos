---
layout: doc
title: interactive / send_gmcp
---
# send_gmcp

### NAME

    send_msdp_variable() - send a MSDP variable

### SYNOPSIS

    void send_gmcp( string name, string value );
    void send_gmcp( string name, int value );
    void send_gmcp( string name, float value );
    void send_gmcp( string name, buffer value );

### DESCRIPTION

    Sends a MSDP variable 'name' with 'value' for this_object() to the user's client. 

    For payload formatting information see: https://tintin.mudhalla.net/protocols/msdp

    Note: send_msdp_variable() should only be called from within a user object.
    
### SEE ALSO

    msdp_enable(4), msdp(4), has_msdp(3)

