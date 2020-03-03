---
layout: default
title: apply / interactive / receive_environ
---

### NAME

    receive_environ

### SYNOPSIS

    void receive_environ( string var, string value);

### DESCRIPTION

    During telnet negotiations, if remote party sends over environment options defined according to
    https://tools.ietf.org/html/rfc1572, the user object will receive this callback to
    process it.

    The user object will be called once for each variable received.
