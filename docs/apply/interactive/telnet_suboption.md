---
title: interactive / telnet_suboption
---
# telnet_suboption

### NAME

    telnet_suboption - process telnet suboptions

### SYNOPSIS

    void telnet_suboption( string buffer );

### DESCRIPTION

    This apply is called on the interactive object when the client sends a
    telnet subnegotiation (IAC SB <option> ... IAC SE) for an option the
    driver does not handle itself, for mudlib defined processing.

    The buffer is the subnegotiation payload only: the option byte has
    already been removed, so the first byte is the option's own subcommand.
    The option code is not passed, so the apply has to work out which option
    sent the payload from its contents. NUL bytes in the payload are replaced
    with 'I'. An empty subnegotiation does not call this apply.

    These options are handled by the driver and never reach this apply:

        TTYPE                  terminal_type(4)
        ENVIRON, NEW_ENVIRON   receive_environ(4)
        NAWS                   window_size(4)
        GMCP                   gmcp(4)
        MSDP                   msdp(4)
        ZMP                    zmp(4)
        LINEMODE, MSSP, COMPRESS2

### EXAMPLE

    The driver offers CHARSET and sends REQUEST ";UTF-8". A client that
    accepts replies with IAC SB CHARSET ACCEPTED "UTF-8" IAC SE, which
    reaches this apply as "\x02UTF-8" (2 is ACCEPTED from RFC 2066):

        void telnet_suboption(string buffer) {
            if (sizeof(buffer) > 1 && buffer[0] == 2 &&
                upper_case(buffer[1..]) == "UTF-8") {
                // client accepted UTF-8
            }
        }

### SEE ALSO

    terminal_type(4), receive_environ(4), window_size(4), gmcp(4), msdp(4), zmp(4)
