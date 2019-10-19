---
layout: default
title: interactive / telnet_suboption
---

### NAME

    telnet_suboption - process telnet suboptions

### SYNOPSIS

    void telnet_suboption( string buffer );

### DESCRIPTION

    This apply is called on the interactive object with the parameter given
    by the SE telnet suboption, for mudlib defined processing.   For  exam‐
    ple, this allows for future support of terminal types.

    The  first  byte  of  the  buffer  is  typically  a type descriptor, ie
    TELOPT_TTYPE. The next byte is a procession option, such as TELQUAL_IS.
    Following  this  is the type dependent data.  In the case of a terminal
    type call, the data will be routed through through the  terminal_type()
    apply instead.

    Possible suboptions:

        #define TELQUAL_IS    0 // option is...
        #define TELQUAL_SEND  1 // send options
        #define TELQUAL_INFO  2 // ENVIRON: informational version of IS
        #define TELQUAL_REPLY 3 // AUTHENTICATION: client version of IS
        #define TELQUAL_NAME  4 // AUTHENTICATION: client version of IS

