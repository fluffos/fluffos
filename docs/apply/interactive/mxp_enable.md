---
layout: doc
title: interactive / mxp_enable
---
# mxp_enable

### NAME

    mxp_enable - client has MXP support

### SYNOPSIS

    void mxp_enable() ;

### DESCRIPTION

    This apply is called when the driver has successfully negotiated MXP
    (MUD eXtension Protocol) support with the client. MXP allows enhanced
    formatting, clickable links, and other rich text features in MUD clients.

    The mudlib can use this callback to enable MXP-specific features or
    track which users have MXP-capable clients.

    MXP support must be enabled in the driver configuration with:
    ```
    enable mxp : 1
    ```

### SEE ALSO

    has_mxp(3), mxp_tag(4)
