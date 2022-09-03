---
layout: default
title: interactive / telnet_msp_oob
---

### NAME

    telnet_msp_oob() - send an OOB MSP message

### SYNOPSIS

    void telnet_msp_oob(string);

### DESCRIPTION

    if user is under telnet, then send an IAC SB TELOPT_MSP msg IAC SE sequence,
    as documented in https://wiki.mudlet.org/w/Manual:Supported_Protocols#MSP
