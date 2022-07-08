---
layout: default
title: parsing / parse_refresh
---

### NAME

    parse_refresh()

### SYNOPSIS

    void parse_refresh();

### DESCRIPTION

    The parsing package caches information about objects in order to improve
    performance. This means that if any information that gets cached is
    changed, you need to tell MudOS to clear the cache. That's what this efun
    does. If the information returned by any of the applies below changes, you
    need to call parse_refresh() so that the parser knows it has changed. For
    example if the name of an object changes, or perhaps an adjective changes
    as a spell is cast to change it from blue to red - call parse_refresh()
    afterwards. The efun clears the cache for the object that called it.

### SEE ALSO

    parse_init(3)

