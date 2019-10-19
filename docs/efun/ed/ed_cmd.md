---
layout: default
title: ed / ed_cmd
---

### NAME

    ed_cmd() - send a command to an ed session

### SYNOPSIS

    string ed_cmd(string cmd)

### DESCRIPTION

    This efun is available only if __OLD_ED__ is not defined.

    The  command  'cmd' is sent to the active ed session, and the resulting
    output is returned.

