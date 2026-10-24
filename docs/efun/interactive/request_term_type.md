---
title: interactive / request_term_type
---
# request_term_type

### NAME

    request_term_type() - ask the client for its terminal type

### SYNOPSIS

    void request_term_type();

### DESCRIPTION

    Asks current_object's client for its next terminal type by sending one
    telnet TERMINAL-TYPE SEND subnegotiation (IAC SB TTYPE SEND IAC SE,
    RFC 1091), then flushes pending output. The client's answer is delivered
    to the terminal_type() apply.

    The driver sends the first request itself, when the client agrees to
    TTYPE (IAC WILL TTYPE). Each later call to request_term_type() asks for
    the next entry in the client's list, so calling it from terminal_type()
    walks the whole list. An MTTS client answers successive requests with
    its client name, its terminal type, and then "MTTS <bitvector>",
    repeating the last entry once the list is exhausted.
    start_request_term_type() restarts the cycle from the first entry.

    If current_object is not interactive, request_term_type() does nothing and a
    warning is emitted. If the connection is not a telnet connection, it does
    nothing.

    Note: request_term_type() should only be called from within a user object.

### EXAMPLE

    // Collect every terminal type an MTTS client reports.
    string *ttypes = ({});

    void terminal_type(string term) {
      if (sizeof(ttypes) && ttypes[<1] == term) return;  // list exhausted
      ttypes += ({ term });
      request_term_type();
    }

### SEE ALSO

    terminal_type(4), start_request_term_type(3), request_term_size(3)
