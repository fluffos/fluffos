---
layout: default
title: interactive / telnet_ga
---

### NAME

    telnet_ga() - send an TELNET GA message

### SYNOPSIS

    void telnet_ga();

### DESCRIPTION

    if user is under telnet, then send an TELNET_IAC TELNET_GA sequence,
    useful for prompts that don't terminate with a newline character.
