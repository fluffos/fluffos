[![Actions Status](https://github.com/seanmiddleditch/libtelnet/workflows/CI/badge.svg)](https://github.com/seanmiddleditch/libtelnet/actions)

libtelnet - TELNET protocol handling library
============================================

 http://github.com/seanmiddleditch/libtelnet

 Sean Middleditch and contributors


The author or authors of this code dedicate any and all copyright
interest in this code to the public domain. We make this dedication
for the benefit of the public at large and to the detriment of our
heirs and successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
code under copyright law.


I. Introduction
---------------

libtelnet provides safe and correct handling of the core TELNET
protocol.  In addition to the base TELNET protocol, libtelnet also
implements the Q method of TELNET option negotiation.  libtelnet can
be used for writing servers, clients, or proxies.

For more information on the TELNET standards supported by libtelnet,
visit the following websites:

* http://www.faqs.org/rfcs/rfc854.html
* http://www.faqs.org/rfcs/rfc855.html
* http://www.faqs.org/rfcs/rfc1091.html
* http://www.faqs.org/rfcs/rfc1143.html
* http://www.faqs.org/rfcs/rfc1408.html
* http://www.faqs.org/rfcs/rfc1572.html

II. API
-------

The libtelnet API contains several distinct parts.  The first part is
the basic initialization and deinitialization routines.  The second
part is a single function for pushing received data into the telnet
processor.  The third part is the libtelnet output functions, which
generate TELNET commands and ensure data is properly formatted before
sending over the wire.  The final part is the event handler
interface.

This document covers only the most basic functions.  See the
libtelnet manual pages or HTML documentation for a complete
reference.

#### IIa. Initialization

 Using libtelnet requires the initialization of a telnet_t structure
 which stores all current state for a single TELNET connection.

 Initializing a telnet_t structure requires several pieces of data.
 One of these is the telopt support table, which specifies which
 TELNET options your application supports both locally and remotely.
 This table is comprised of telnet_telopt_t structures, one for each
 supported option.  Each entry specifies the option supported,
 whether the option is supported locally or remotely.

```
struct telnet_telopt_t {
    short telopt;
    unsigned char us;
    unsigned char him;
};
```

 The us field denotes whether your application supports the telopt
 locally.  It should be set to TELNET_WILL if you support it and to
 TELNET_WONT if you don't.  The him field denotes whether the telopt
 is supported on the remote end, and should be TELNET_DO if yes and
 TELNET_DONT if not.

 When definition the telopt table you must include an end marker
 entry, which is simply an entry with telopt set to -1.  For
 example:

```
static const telnet_telopt_t my_telopts[] = {
    { TELNET_TELOPT_ECHO,      TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_TTYPE,     TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
    { TELNET_TELOPT_NAWS,      TELNET_WILL, TELNET_DONT },
    { -1, 0, 0 }
};
```

 If you need to dynamically alter supported options on a
 per-connection basis then you may use a different table
 (dynamically allocated if necessary) per call to telnet_init() or
 you share a single constant table like the above example between
 all connections if you support a fixed set of options.  Most
 applications will support only a fixed set of options.

* `telnet_t *telnet_init(const telnet_telopts_t *telopts,
     telnet_event_handler_t handler, unsigned char flags,
     void *user_data);`

   The telnet_init() function is responsible for allocating memory
   and initializing the data in a telnet_t structure.  It must be
   called immediately after establishing a connection and before any
   other libtelnet API calls are made.

   The telopts field is the telopt support table as described above.

   The handler parameter must be a function matching the
   telnet_event_handler_t definition.  More information about events
   can be found in section IId.

   The user_data parameter is passed to the event handler whenver it
   is invoked.  This will usually be a structure container
   information about the connection, including a socket descriptor
   for implementing TELNET_EV_SEND event handling.

   The flags parameter can be any of the following flag constants
   bit-or'd together, or 0 to leave all options disabled.

    TELNET_FLAG_PROXY
      Operate in proxy mode.  This disables the RFC1143 support and
      enables automatic detection of COMPRESS2 streams.

    TELNET_FLAG_NVT_EOL
      Receive data with translation of the TELNET NVT CR NUL and CR LF
      sequences specified in RFC854 to C carriage return (\r) and C
      newline (\n), respectively.

   If telnet_init() fails to allocate the required memory, the
   returned pointer will be zero.

* `void telnet_free(telnet_t *telnet);`

   Releases any internal memory allocated by libtelnet for the given
   telnet pointer.  This must be called whenever a connection is
   closed, or you will incur memory leaks.  The pointer passed in may
   no longer be used afterwards.

#### IIb. Receiving Data

* `void telnet_recv(telnet_t *telnet,
     const char *buffer, unsigned int size, void *user_data);`

   When your application receives data over the socket from the
   remote end, it must pass the received bytes into this function.

   As the TELNET stream is parsed, events will be generated and
   passed to the event handler given to telnet_init().  Of particular
   interest for data receiving is the TELNET_EV_DATA event, which is
   triggered for any regular data such as user input or server
   process output.

#### IIc. Sending Data

 All of the output functions will invoke the TELNET_EV_SEND event.

 Note: it is very important that ALL data sent to the remote end of
 the connection be passed through libtelnet.  All user input or
 process output that you wish to send over the wire should be given
 to one of the following functions.  Do NOT send or buffer
 unprocessed output data directly!

* `void telnet_iac(telnet_t *telnet, unsigned char cmd);`

   Sends a single "simple" TELNET command, such as the GO-AHEAD
   commands (255 249).

* `void telnet_negotiate(telnet_t *telnet, unsigned char cmd,
     unsigned char opt);`

   Sends a TELNET negotiation command.  The cmd parameter must be one
   of TELNET_WILL, TELNET_WONT, TELNET_DO, or TELNET_DONT.  The opt
   parameter is the option to negotiate.

   Unless in PROXY mode, the RFC1143 support may delay or ellide the
   request entirely, as appropriate.  It will ignore duplicate
   invocations, such as asking for WILL NAWS when NAWS is already on
   or is currently awaiting response from the remote end.

* `void telnet_send(telnet_t *telnet, const char *buffer, size_t size);`

   Sends raw data, which would be either the process output from a
   server or the user input from a client.

   For sending regular text it may be more convenient to use
   telnet_printf().

* `void telnet_send_text(telnet_t *telnet, const char *buffer,
     size_t size);`

   Sends text characters with translation of C newlines (\n) into
   CR LF and C carriage returns (\r) into CR NUL, as required by
   RFC854, unless transmission in BINARY mode has been negotiated.

   For sending regular text it may be more convenient to use
   telnet_printf().

* `void telnet_begin_sb(telnet_t *telnet, unsigned char telopt);`

   Sends the header for a TELNET sub-negotiation command for the
   specified option.  All send data following this command will be
   part of the sub-negotiation data until a call is made to
   telnet_finish_subnegotiation().

   You should not use telnet_printf() for sending subnegotiation
   data as it will perform newline translations that usually do not
   need to be done for subnegotiation data, and may cause problems.

* `void telnet_finish_sb(telnet_t *telnet);`

   Sends the end marker for a TELNET sub-negotiation command.  This
   must be called after (and only after) a call has been made to
   telnet_begin_subnegotiation() and any negotiation data has been
   sent.

* `void telnet_subnegotiation(telnet_t *telnet, unsigned char telopt,
     const char *buffer, unsigned int size);`

   Sends a TELNET sub-negotiation command.  The telopt parameter is
   the sub-negotiation option.

   Note that this function is just a shorthand for:
   ```
    telnet_begin_sb(telnet, telopt);
    telnet_send(telnet, buffer, size);
    telnet_end_sb(telnet);
   ```

   For some subnegotiations that involve a lot of complex formatted
   data to be sent, it may be easier to make calls to both
   telnet_begin_sb() and telnet_finish_sb() and using telnet_send()
   or telnet_printf2() to format the data.

   NOTE: telnet_subnegotiation() does have special behavior in
   PROXY mode, as in that mode this function will automatically
   detect the COMPRESS2 marker and enable zlib compression.

* `int telnet_printf(telnet_t *telnet, const char *fmt, ...);`

  This functions very similarly to fprintf, except that output is
  sent through libtelnet for processing.  IAC bytes are properly
  escaped, C newlines (\n) are translated into CR LF, and C carriage
  returns (\r) are translated into CR NUL, all as required by
  RFC854.  The return code is the length of the formatted text.

  NOTE: due to an internal implementation detail, the maximum
  lenth of the formatted text is 4096 characters.


#### IId. Event Handling

 libtelnet relies on an event-handling mechanism for processing the
 parsed TELNET protocol stream as well as for buffering and sending
 output data.

 When you initialize a telnet_t structure with telnet_init() you had
 to pass in an event handler function.  This function must meet the
 following prototype:

  `void (telnet_t *telnet, telnet_event_t *event, void *user_data);`

 The event structure is detailed below.  The user_data value is the
 pointer passed to telnet_init().

 The following is a summary of the most important parts of the
 telnet_event_t data type.  Please see the libtelnet manual pages or
 HTML document for a complete reference.

```
union telnet_event_t {
  enum telnet_event_type_t type;

  struct data_t {
    enum telnet_event_type_t _type;
    const char *buffer;
    size_t size;
  } data;

  struct error_t {
    enum telnet_event_type_t _type;
    const char *file;
    const char *func;
    const char *msg;
    int line;
    telnet_error_t errcode;
  } error;

  struct iac_t {
    enum telnet_event_type_t _type;
    unsigned char cmd;
  } iac;

  struct negotiate_t {
    enum telnet_event_type_t _type;
    unsigned char telopt;
  } neg;

  struct subnegotiate_t {
    enum telnet_event_type_t _type;
    const char *buffer;
    size_t size;
    unsigned char telopt;
  } sub;
};
```

 The enumeration values of telnet_event_type_t are described in
 detail below.  Whenever the the event handler is invoked, the
 application must look at the event->type value and do any necessary
 processing.

 The only event that MUST be implemented is TELNET_EV_SEND.  Most
 applications will also always want to implement the event
 TELNET_EV_DATA.

 Here is an example event handler implementation which includes
 handlers for several important events.

```
void my_event_handler(telnet_t *telnet, telnet_event_t *ev,
    void *user_data) {
  struct user_info *user = (struct user_info *)user_data;

  switch (ev->type) {
  case TELNET_EV_DATA:
    process_user_input(user, event->data.buffer, event->data.size);
    break;
  case TELNET_EV_SEND:
    write_to_descriptor(user, event->data.buffer, event->data.size);
    break;
  case TELNET_EV_ERROR:
    fatal_error("TELNET error: %s", event->error.msg);
    break;
  }
}
```

* TELNET_EV_DATA

   The DATA event is triggered whenever regular data (not part of any
   special TELNET command) is received.  For a client, this will be
   process output from the server.  For a server, this will be input
   typed by the user.

   The event->data.buffer value will contain the bytes received and the
   event->data.size value will contain the number of bytes received.
   Note that event->data.buffer is not NUL terminated!

   NOTE: there is no guarantee that user input or server output
   will be received in whole lines.  If you wish to process data
   a line at a time, you are responsible for buffering the data and
   checking for line terminators yourself!

* TELNET_EV_SEND

   This event is sent whenever libtelnet has generated data that must
   be sent over the wire to the remove end.  Generally that means
   calling send() or adding the data to your application's output
   buffer.

   The event->data.buffer value will contain the bytes to send and the
   event->data.size value will contain the number of bytes to send.
   Note that event->data.buffer is not NUL terminated, and may include
   NUL characters in its data, so always use event->data.size!

   NOTE: Your SEND event handler must send or buffer the data in
   its raw form as provided by libtelnet.  If you wish to perform
   any kind of preprocessing on data you want to send to the other

* TELNET_EV_IAC

   The IAC event is triggered whenever a simple IAC command is
   received, such as the IAC EOR (end of record, also called go ahead
   or GA) command.

   The command received is in the event->iac.cmd value.

   The necessary processing depends on the specific commands; see
   the TELNET RFC for more information.

* TELNET_EV_WILL / TELNET_EV_DO

   The WILL and DO events are sent when a TELNET negotiation command
   of the same name is received.

   WILL events are sent by the remote end when they wish to be
   allowed to turn an option on on their end, or in confirmation
   after you have sent a DO command to them.

   DO events are sent by the remote end when they wish for you to
   turn on an option on your end, or in confirmation after you have
   sent a WILL command to them.

   In either case, the TELNET option under negotiation will be in
   event->neg.telopt field.

   libtelnet manages most of the pecularities of negotiation for you.
   For information on libtelnet's negotiation method, see:

    http://www.faqs.org/rfcs/rfc1143.html

   Note that in PROXY mode libtelnet will do no processing of its
   own for you.

* TELNET_EV_WONT / TELNET_EV_DONT

   The WONT and DONT events are sent when the remote end of the
   connection wishes to disable an option, when they are refusing to
   a support an option that you have asked for, or in confirmation of
   an option you have asked to be disabled.

   Most commonly WONT and DONT events are sent as rejections of
   features you requested by sending DO or WILL events.  Receiving
   these events means the TELNET option is not or will not be
   supported by the remote end, so give up.

   Sometimes WONT or DONT will be sent for TELNET options that are
   already enabled, but the remote end wishes to stop using.  You
   cannot decline.  These events are demands that must be complied
   with.  libtelnet will always send the appropriate response back
   without consulting your application.  These events are sent to
   allow your application to disable its own use of the features.

   In either case, the TELNET option under negotiation will be in
   event->neg.telopt field.

   Note that in PROXY mode libtelnet will do no processing of its
   own for you.

* TELNET_EV_SUBNEGOTIATION

   Triggered whenever a TELNET sub-negotiation has been received.
   Sub-negotiations include the NAWS option for communicating
   terminal size to a server, the NEW-ENVIRON and TTYPE options for
   negotiating terminal features, and MUD-centric protocols such as
   ZMP, MSSP, and MCCP2.

   The event->sub->telopt value is the option under sub-negotiation.
   The remaining data (if any) is passed in event->sub.buffer and
   event->sub.size.  Note that most subnegotiation commands can include
   embedded NUL bytes in the subnegotiation data, and the data
   event->sub.buffer is not NUL terminated, so always use the
   event->sub.size value!

   The meaning and necessary processing for subnegotiations are
   defined in various TELNET RFCs and other informal specifications.
   A subnegotiation should never be sent unless the specific option
   has been enabled through the use of the telnet negotiation
   feature.

   TTYPE/ENVIRON/NEW-ENVIRON/MSSP/ZMP SUPPORT:
   libtelnet parses these subnegotiation commands.  A special
   event will be sent for each, after the SUBNEGOTIATION event is
   sent.  Except in special circumstances, the SUBNEGOTIATION event
   should be ignored for these options and the special events should
   be handled explicitly.

* TELNET_EV_COMPRESS

   The COMPRESS event notifies the app that COMPRESS2/MCCP2
   compression has begun or ended.  Only servers can send compressed
   data, and hence only clients will receive compressed data.

   The event->command value will be 1 if compression has started and
   will be 0 if compression has ended.

* TELNET_EV_ZMP

   The event->zmp.argc field is the number of ZMP parameters, including
   the command name, that have been received.  The event->zmp.argv
   field is an array of strings, one for each ZMP parameter.  The
   command name will be in event->zmp.argv[0].

* TELNET_EV_TTYPE

   The event->ttype.cmd field will be either TELNET_TTYPE_SEND,
   TELNET_TTYPE_IS, TELNET_TTYPE_INFO.

   The actual terminal type will be in event->ttype.name.

* TELNET_EV_ENVIRON

   The event->environ.cmd field will be either TELNET_ENVIRON_IS,
   TELNET_ENVIRON_SEND, or TELNET_ENVIRON_INFO.

   The actual environment variable sent or requested will be sent
   in the event->environ.values field.  This is an array of
   structures with the following format:

   ```
     struct telnet_environ_t {
       unsigned char type;
       const char *var;
       const char *value;
     };
   ```

   The number of entries in the event->environ.values array is
   stored in event->environ.count.

   Note that libtelnet does not support the ESC byte for ENVIRON/
   NEW-ENVIRON.  Data using escaped bytes will not be parsed
   correctly.

* TELNET_EV_MSSP

   The event->mssp.values field is an array of telnet_environ_t
   structures.  The cmd field in each entry will have an
   unspecified value, while the var and value fields will always
   be set to the MSSP variable and value being set.  For multi-value
   MSSP variables, there will be multiple entries in the values
   array for each value, each with the same variable name set.

   The number of entries in the event->mssp.values array is
   stored in event->mssp.count.

* TELNET_EV_WARNING

   The WARNING event is sent whenever something has gone wrong inside
   of libtelnet (possibly due to malformed data sent by the other
   end) but which recovery is (likely) possible.  It may be safe to
   continue using the connection, but some data may have been lost or
   incorrectly interpreted.

   The event->error.msg field will contain a NUL terminated string
   explaining the error.

* TELNET_EV_ERROR

   Similar to the WARNING event, the ERROR event is sent whenever
   something has gone wrong.  ERROR events are non-recoverable,
   however, and the application should immediately close the
   connection.  Whatever has happened is likely going only to result
   in garbage from libtelnet.  This is most likely to happen when a
   COMPRESS2 stream fails, but other problems can occur.

   The event->error.msg field will contain a NUL terminated string
   explaining the error.

III. Integrating libtelnet with common muds
-------------------------------------------

FIXME: fill in some notes about how to splice in libtelnet with
common Diku/Merc/Circle/etc. MUD codebases.

IV. Safety and correctness considerations
-----------------------------------------

Your existing application may make heavy use of its own output
buffering and transmission commands, including hand-made routines for
sending TELNET commands and sub-negotiation requests.  There are at
times subtle issues that need to be handled when communication over
the TELNET protocol, not least of which is the need to escape any
byte value 0xFF with a special TELNET command.

For these reasons, it is very important that applications making use
of libtelnet always make use of the libtelnet output functions for
all data being sent over the TELNET connection.

In particular, if you are writing a client, all user input must be
passed through to telnet_send().  This also includes any input
generated automatically by scripts, triggers, or macros.

For a server, any and all output -- including ANSI/VT100 escape
codes, regular text, newlines, and so on -- must be passed through to
telnet_send().

Any TELNET commands that are to be sent must be given to one of the
following: telnet_iac, telnet_negotiate, or telnet_subnegotiation().

If you are attempting to enable COMPRESS2/MCCP2, you must use the
telnet_begin_compress2() function.

V. MCCP2 compression
--------------------

The MCCP2 (COMPRESS2) TELNET extension allows for the compression of
all traffic sent from server to client.  For more information:

 http://www.mudbytes.net/index.php?a=articles&s=mccp

In order for libtelnet to support MCCP2, zlib must be installed and
enabled when compiling libtelnet.  Use -DHAVE_ZLIB to enable zlib
when compiling libtelnet.c and pass -lz to the linker to link in the
zlib shared library.

libtelnet transparently supports MCCP2.  For a server to support
MCCP2, the application must begin negotiation of the COMPRESS2 option
using telnet_negotiate(), for example:

 `telnet_negotiate(&telnet, TELNET_WILL,
     TELNET_OPTION_COMPRESS2, user_data);`

If a favorable DO COMPRESS2 is sent back from the client then the
server application can begin compression at any time by calling
telnet_begin_compress2().

If a connection is in PROXY mode and COMPRESS2 support is enabled
then libtelnet will automatically detect the start of a COMPRESS2
stream, in either the sending or receiving direction.

VI. Zenith MUD Protocol (ZMP) support
-------------------------------------

The Zenith MUD Protocol allows applications to send messages across
the TELNET connection outside of the normal user input/output data
stream.  libtelnet offers some limited support for receiving and
sending ZMP commands to make implementing a full ZMP stack easier.
For more information on ZMP:

 http://zmp.sourcemud.org/

For a server to enable ZMP, it must send the WILL ZMP negotitaion:

 `telnet_negotiate(&telnet, TELNET_WILL, TELNET_TELOPT_ZMP);`

For a client to support ZMP it must include ZMP in the telopt table
passed to telnet_init(), with the him field set to TELNET_DO:

 `{ TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },`

Note that while ZMP is a bi-directional protocol, it is only ever
enabled on the server end of the connection.  This automatically
enables the client to send ZMP commands.  The client must never
attempt to negotiate ZMP directly using telnet_negotiate().

Once ZMP is enabled, any ZMP commands received will automatically be
sent to the event handler function with the TELNET_EV_SUBNEGOTIATION
event code.  The command will automatically be parsed and the ZMP
parameters will be placed in the event->argv array and the number of
parameters will be placed in the event->argc field.

NOTE: if an error occured while parsing the ZMP command because it
was malformed, the event->argc field will be equal to 0 and the
event->argv field will be NULL.  You should always check for this
before attempting to access the parameter array.

To send ZMP commands to the remote end, use either telnet_send_zmp()
or telnet_send_zmpv().

* `int telnet_send_zmp(telnet_t *telnet, size_t argv,
    const char **argv);`

  Sends a ZMP command to the remote end.  The argc parameter is the
  number of ZMP parameters (including the command name!) to be sent.
  The argv parameter is an array of strings containing the
  parameters.  The element in argv[0] is the command name itself.
  The argv array must have at least as many elements as the value
  argc.

VII. MUD Server Status Protocol (MSSP) support
----------------------------------------------

MSSP allows for crawlers or other clients to query a MUD server's
supported feature list.  This allows MUD listing states to
automatically stay up to date with the MUD's features, and not
require MUD administrators to manually update listing sites for
their MUD.  For more information on MSSP:

 http://tintin.sourceforge.net/mssp/

VIII. Telnet proxy utility
--------------------------

The telnet-proxy utility is a small application that serves both as a
testbed for libtelnet and as a powerful debugging tool for TELNET
servers and clients.

To use telnet-proxy, you must first compile it using:

```
 $ make
```

If you do not have zlib installed and wish to disable MCCP2 support
then you must first edit the Makefile and remove the -DHAVE_ZLIB and
the -lz from the compile flags.

To run telnet-proxy, you simply give it the server's host name or IP
address, the server's port number, and the port number that
telnet-proxy should listen on.  For example, to connect to the server
on mud.example.com port 7800 and to listen on port 5000, run:

```
 $ ./telnet-proxy mud.example.com 7800 5000
```

You can then connect to the host telnet-proxy is running on (e.g.
127.0.0.1) on port 5000 and you will automatically be proxied into
mud.example.com.

telnet-proxy will display status information about the data passing
through both ends of the tunnel.  telnet-proxy can only support a
single tunnel at a time.  It will continue running until an error
occurs or a terminating signal is sent to the proxy process.
