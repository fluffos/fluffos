---
layout: doc
title: interactive / has_gmcp
---
# has_gmcp

### NAME

    has_gmcp() - returns whether or not a given interactive user's client 
    has GMCP protocol enabled.

### SYNOPSIS

    int has_gmcp( object user );

### DESCRIPTION

    Return non-zero if interactive user has the GMCP protocol enabled in 
    their client. 0 will be returned if the user does not.

    Note:
    Fluffos requires the following option to be set in the startup config
    in order to support the GMCP protocol.

    ```
    enable gmcp : 1
    ```

### SEE ALSO

    gmcp_enable(4), gmcp(4), send_gmcp(3), has_zmp(), has_mxp()
