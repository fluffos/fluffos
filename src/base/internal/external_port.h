/*
 * external_port.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef BASE_INTERNAL_EXTERNAL_PORT_H_
#define BASE_INTERNAL_EXTERNAL_PORT_H_

#include <event2/util.h>

#define PORT_UNDEFINED 0
#define PORT_TELNET 1
#define PORT_BINARY 2
#define PORT_ASCII 3
#define PORT_MUD 4
#define PORT_WEBSOCKET 5

struct port_def_t {
  int kind;
  int port;
  evutil_socket_t fd;
  // used by F_NETWORK_STATS
  int in_packets;
  int in_volume;
  int out_packets;
  int out_volume;
  struct evconnlistener* ev_conn;
  // websocket context
  struct lws_context* lws_context;
};

static inline const char* port_kind_name(int kind) {
  switch (kind) {
    case PORT_TELNET:
      return "telnet";
    case PORT_BINARY:
      return "binary";
    case PORT_ASCII:
      return "ascii";
    case PORT_MUD:
      return "mud";
    case PORT_WEBSOCKET:
      return "websocket";

    default:
      return "unknown";
  }
}

extern port_def_t external_port[5];

#endif /* BASE_INTERNAL_EXTERNAL_PORT_H_ */
