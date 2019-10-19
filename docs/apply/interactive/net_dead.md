---
layout: default
title: interactive / net_dead
---

### NAME

    net_dead  - called by the MudOS driver when an interactive object drops
    its connection

### SYNOPSIS

    void net_dead( void );

### DESCRIPTION

    If an interactive object (i.e. the player object)  suddenly  loses  its
    connection  (i.e. it goes "net dead"), then the driver calls this func‐
    tion on that object giving it a chance to clean up, notify its environ‐
    ment  etc.   Be  aware  that  functions that depend on the object being
    interactive will not work as expected.

### AUTHOR

    Wayfarer@Portals

