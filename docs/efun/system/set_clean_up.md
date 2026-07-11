---
title: system / set_clean_up
---
# set_clean_up

### NAME

    set_clean_up - schedule when an object should next be queried for clean_up

### SYNOPSIS

    varargs void set_clean_up( object ob, int time );

### DESCRIPTION

    Schedules the driver's next clean_up() query on 'ob' for 'time'
    seconds from now, overriding the default idle-time rule (an object
    is normally queried once it has gone unreferenced for the
    'time to clean up' config setting). The deadline is one-shot: after
    it fires, the object reverts to the idle-time rule.

    If 'time' is omitted, any pending explicit deadline is cancelled and
    the object reverts to the idle-time rule immediately.

    In both forms the object is re-flagged for clean_up consideration
    (like request_clean_up()), provided it defines a clean_up() apply.
    Objects without a clean_up() apply are never queried.

    Note that the sweep that performs clean_up queries runs every few
    minutes, so 'time' is a lower bound, not an exact firing time. If
    the 'time to clean up' config setting is 0, clean_up is disabled
    globally and explicit deadlines are ignored as well.

### SEE ALSO

    clean_up(4), request_clean_up(3), set_reset(3)
