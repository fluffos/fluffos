#ifndef MAIN_H
#define MAIN_H

#include "lpc_incl.h"
#include <time.h>
/*
 * main.c
 */

#define PORT_UNDEFINED 0
#define PORT_TELNET 1
#define PORT_BINARY 2
#define PORT_ASCII 3
#define PORT_MUD 4
#define PORT_WEBSOCKET 5

struct port_def_t {
  int kind;
  int port;
  int fd;
#ifdef F_NETWORK_STATS
  int in_packets;
  int in_volume;
  int out_packets;
  int out_volume;
#endif
  struct evconnlistener *ev_conn;
};

extern port_def_t external_port[5];
#ifdef PACKAGE_EXTERNAL
extern char *external_cmd[NUM_EXTERNAL_CMDS];
#endif

// TODO: These should all be deleted soon.
extern time_t boot_time;

void debug_message(const char *, ...);

#endif
