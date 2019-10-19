---
layout: default
title: objects / set_heart_beat
---

### NAME

    set_heart_beat() - enable or disable an object's heartbeat

### SYNOPSIS

    int set_heart_beat( int flag );

### DESCRIPTION

    Passing 'flag' as 0 disables the object's heart beat.  Passing a 'flag'
    of 1 will cause heart_beat() to be called in the object once each heart
    beat  (a variable number defined by your local administrator, usually 2
    seconds).  Passing a 'flag' of greater than 1 will usually set the num‐
    ber  of heart beats inbetween calls to heart_beat(), however your local
    administrator may have the system configured to treat any 'flag'  above
    1 as 1.

### SEE ALSO

    heart_beat(4), query_heart_beat(3)

