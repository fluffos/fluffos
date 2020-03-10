---
layout: default
title: contrib / network_stats.pre
---

### NAME

    network_stats

### SYNOPSIS

    mapping network_stats();

### DESCRIPTION

    returns mapping:

    - incoming packets total : int
    - incoming volume total : int
    - outgoing packets total : int
    - outgoing volume total : int

    - incoming packets port X : int
    - incoming volume port X : int
    - outgoing packets port X : int
    - outgoing volume port X : int

    #ifdef PACKAGE_SOCKETS:

    - incoming packets sockets : int
    - incoming volume sockets : int
    - outgoing packets sockets : int
    - outgoing volume sockets : int
