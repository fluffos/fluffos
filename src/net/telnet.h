#ifndef TELNET_H_
#define TELNET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "libtelnet/libtelnet.h"

// FIXME
#include "comm.h"  // interactive_t *

#include "net/mssp.h"
#include "net/telnet_ext.h"

static const telnet_telopt_t my_telopts[] = {
    {TELNET_TELOPT_TM, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_SGA, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_NAWS, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_LINEMODE, TELNET_WONT, TELNET_DO},
    {TELNET_TELOPT_ECHO, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_TTYPE, TELNET_WONT, TELNET_DO},
    {TELNET_TELOPT_NEW_ENVIRON, TELNET_WONT, TELNET_DO},
    {TELNET_TELOPT_COMPRESS2, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_ZMP, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_MSSP, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_GMCP, TELNET_WILL, TELNET_DO},
    {-1, 0, 0}};

// Telnet event handler
void telnet_event_handler(telnet_t *, telnet_event_t *, void *);

// Send initial negotiations.
void send_initial_telent_negotiantions(interactive_t *user);

// Change terminal mode
void set_linemode(interactive_t *ip);
void set_charmode(interactive_t *ip);
void set_echo(interactive_t *ip, bool enable);

#endif /* TELNET_H_ */
