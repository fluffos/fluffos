---
layout: default
title: interactive / window_size
---

### NAME

    window_size - report the users window size

### SYNOPSIS

    void window_size(int width, int height);

### DESCRIPTION

    window_size() is called with the user's window size, as reported by telnet
    negotiation.  If the user's client never responds to the query, this is
    never called.
