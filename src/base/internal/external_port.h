/*
 * external_port.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef BASE_INTERNAL_EXTERNAL_PORT_H_
#define BASE_INTERNAL_EXTERNAL_PORT_H_

#include <event2/util.h>
#include <openssl/ssl.h>
#include <string>

enum PORT_TYPE {
  PORT_TYPE_UNDEFINED = 0,
  PORT_TYPE_TELNET = 1,
  PORT_TYPE_BINARY = 2,
  PORT_TYPE_ASCII = 3,
  PORT_TYPE_MUD = 4,
  PORT_TYPE_WEBSOCKET = 5
};

struct port_def_t {
  PORT_TYPE kind;
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
  // ssl context
  SSL_CTX* ssl;
  std::string tls_cert;
  std::string tls_key;
};

static inline const char* port_kind_name(int kind) {
  switch (kind) {
    case PORT_TYPE_TELNET:
      return "telnet";
    case PORT_TYPE_BINARY:
      return "binary";
    case PORT_TYPE_ASCII:
      return "ascii";
    case PORT_TYPE_MUD:
      return "mud";
    case PORT_TYPE_WEBSOCKET:
      return "websocket";

    default:
      return "unknown";
  }
}

extern port_def_t external_port[5];

#endif /* BASE_INTERNAL_EXTERNAL_PORT_H_ */
