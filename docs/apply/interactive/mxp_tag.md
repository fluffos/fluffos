---
layout: doc
title: interactive / mxp_tag
---
# mxp_tag

### NAME

    mxp_tag - process MXP tags from client

### SYNOPSIS

    int mxp_tag(string tag) ;

### DESCRIPTION

    This apply is called when the driver receives an MXP tag from an
    MXP-capable client. The mudlib can process these tags and decide
    whether to allow or block them.

    **Arguments:**
    - `tag`: The MXP tag received from the client

    **Return Value:**
    - Return non-zero to block/reject the tag
    - Return 0 to allow the tag

    MXP tags allow clients to send structured data and interact with
    the MUD through clickable elements, forms, and other rich features.

    MXP support must be enabled in the driver configuration with:
    ```
    enable mxp : 1
    ```

### SEE ALSO

    has_mxp(3), mxp_enable(4)
