---
title: interactive / request_term_size
---
# request_term_size

### NAME

    request_term_size() - control telnet window-size (NAWS) reporting

### SYNOPSIS

    void request_term_size(void | int enable);

### DESCRIPTION

    Controls telnet NAWS (Negotiate About Window Size) for current_object's
    client. Called with no argument, or with a nonzero 'enable', it requests
    window-size reporting (telnet DO NAWS). Called with 'enable' equal to 0 it
    stops requesting window size (telnet DONT NAWS). Pending output is flushed.

    If current_object is not interactive, request_term_size() does nothing and a
    warning is emitted.

    Note: request_term_size() should only be called from within a user object.

### SEE ALSO

    request_term_type(3)
