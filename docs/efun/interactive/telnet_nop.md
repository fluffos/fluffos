---
layout: doc
title: interactive / telnet_nop
---
# telnet_nop

### NAME

    telnet_nop() - send an TELNET NOP message

### SYNOPSIS

    void telnet_nop();

### DESCRIPTION

    if user is under telnet, then send an TELNET_IAC TELNET_NOP sequence, useful
    to keep the connection alive.
