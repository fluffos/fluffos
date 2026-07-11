---
title: interactive / request_term_type
---
# request_term_type

### NAME

    request_term_type() - ask the client for its terminal type

### SYNOPSIS

    void request_term_type();

### DESCRIPTION

    Asks current_object's client for its terminal type by sending a telnet
    TTYPE (terminal-type) request, then flushes pending output. The client's
    response is handled by the driver's telnet layer.

    If current_object is not interactive, request_term_type() does nothing and a
    warning is emitted.

    Note: request_term_type() should only be called from within a user object.

### SEE ALSO

    start_request_term_type(3), request_term_size(3)
