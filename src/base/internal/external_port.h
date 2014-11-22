/*
 * external_port.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef BASE_INTERNAL_EXTERNAL_PORT_H_
#define BASE_INTERNAL_EXTERNAL_PORT_H_

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
  // used by F_NETWORK_STATS
  int in_packets;
  int in_volume;
  int out_packets;
  int out_volume;
  struct evconnlistener *ev_conn;
};

extern port_def_t external_port[5];

#endif /* BASE_INTERNAL_EXTERNAL_PORT_H_ */
