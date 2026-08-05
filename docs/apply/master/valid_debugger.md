---
title: master / valid_debugger
---
# valid_debugger

### NAME

    valid_debugger - optional veto over WebSocket LPC debugger attachment

### SYNOPSIS

    int valid_debugger( string remote_ip );

### DESCRIPTION

    Consulted once, when a debugger client's `attach` request has already
    passed the driver's own gates (a correct `debugger password` token, if
    one is configured) but before the session is marked attached. Returning
    a falsy value (0, or simply not defining this apply's approval) denies
    the attach; the driver sends a DAP failure response and closes the
    connection, exactly like a bad password. Returning a truthy value lets
    the attach proceed.

    The single argument is the client's peer IP address (a dotted-quad or
    IPv6 literal, e.g. "127.0.0.1"), with no reverse DNS lookup performed.

    This apply is entirely **optional** -- it is a policy layer a mudlib
    can add on top of the driver's existing gates (the port is disabled
    unless `debugger port` is configured; a non-loopback `debugger address`
    refuses to start without `debugger password` set). If the master
    object does not define `valid_debugger`, attachment proceeds exactly
    as if this apply did not exist. Use it to restrict attachment to a
    known set of addresses, log attach attempts, or add rate-limiting on
    top of the password check.

    The debugger operates at driver level, bypassing normal mudlib
    `valid_read`/`valid_write` policy once attached (see
    [Configuration](../../driver/config) and the
    [WebSocket LPC Debugger](../../concepts/general/debugger) guide) --
    `valid_debugger` is the only mudlib-side checkpoint before that access
    is granted.

### SEE ALSO

    [WebSocket LPC Debugger](../../concepts/general/debugger), valid_socket(4)
