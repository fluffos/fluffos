---
layout: doc
title: interactive / send_msdp_variable
---
# send_gmcp

### NAME

    send_msdp_variable() - send a MSDP variable

### SYNOPSIS

    void send_msdp_variable( string name, string value );
    void send_msdp_variable( string name, int value );
    void send_msdp_variable( string name, float value );
    void send_msdp_variable( string name, buffer value );

### DESCRIPTION

    Sends a MSDP variable 'name' with 'value' for this_object() to the user's client. 

    Note: send_msdp_variable() should only be called from within a user object.
    
### SEE ALSO

    msdp_enable(4), msdp(4), has_msdp(3)

