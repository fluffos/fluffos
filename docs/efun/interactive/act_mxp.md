---
title: interactive / act_mxp
---
# act_mxp

### NAME

    act_mxp() - begin an MXP telnet subnegotiation

### SYNOPSIS

    void act_mxp();

### DESCRIPTION

    Begins an MXP (MUD eXtension Protocol) telnet subnegotiation on
    current_object's client, signalling the client to enter MXP mode. This is
    only meaningful for a user object with an active telnet session.

    If current_object is not interactive, act_mxp() does nothing and a warning
    is emitted.

    Note: act_mxp() should only be called from within a user object.

### SEE ALSO

    has_mxp(3), send_gmcp(3)
