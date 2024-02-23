---
layout: doc
title: interactive / has_gmcp
---
# has_gmcp

### NAME

    has_gmcp() - returns whether a given interactive user's client has
    GMCP protocol enabled

### SYNOPSIS

    int has_gmcp( object user );

### DESCRIPTION

    If the interactive user has the GMCP protocol enabled in their client,
    this function will return 1, otherwise 0 will be returned.

    Note:
    FluffOS requires the following option to be set in the runtime config
    in order to support the GMCP protocol.

```
enable gmcp : 1
```

### SEE ALSO

    gmcp_enable(4), gmcp(4), send_gmcp(3), has_zmp(3), has_mxp(3)
