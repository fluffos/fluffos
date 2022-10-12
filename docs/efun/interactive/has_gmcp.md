---
layout: default
title: interactive / has_gmcp
---

### NAME

    has_gmcp() - returns whether or not a given interactive user's client 
    has GMCP protocol enabled.

### SYNOPSIS

    int has_gmcp( object ob );

### DESCRIPTION

    Return non-zero if interactive user has the GMCP protocol enabled in 
    their client. 0 will be returned if the user does not.

### SEE ALSO

    gmcp_enable(4), gmcp(4), send_gmcp(3)
