---
layout: default
title: objects / query_heart_beat
---

### NAME

    query_heart_beat() - query the status of an object's heartbeat

### SYNOPSIS

    int query_heart_beat( object );

### DESCRIPTION

    Returns  the  value with which set_heart_beat() has been called with on
    'object'.  If object is not given, it defaults to the  current  object.
    If the object has no heart beat, 0 will be returned.

### SEE ALSO

    heart_beat(4), set_heart_beat(3)

