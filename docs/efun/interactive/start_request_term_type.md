---
title: interactive / start_request_term_type
---
# start_request_term_type

### NAME

    start_request_term_type() - restart the terminal-type request cycle

### SYNOPSIS

    void start_request_term_type();

### DESCRIPTION

    (Re)starts the terminal-type request cycle for the command_giver's client by
    resetting the telnet TTYPE state, so that a subsequent request_term_type()
    begins again from the first entry in the client's list of terminal types.
    Pending output is flushed.

    If there is no interactive command_giver, start_request_term_type() does
    nothing and a warning is emitted.

    Note: start_request_term_type() should only be called from within a user
    object.

### SEE ALSO

    request_term_type(3), request_term_size(3)
