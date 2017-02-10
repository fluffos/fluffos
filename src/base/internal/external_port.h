/*
 * external_port.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef BASE_INTERNAL_EXTERNAL_PORT_H_
#define BASE_INTERNAL_EXTERNAL_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

static const int PORT_UNDEFINED = 0;
static const int PORT_TELNET = 1;
static const int PORT_BINARY = 2;
static const int PORT_ASCII = 3;
static const int PORT_MUD = 4;
static const int PORT_WEBSOCKET = 5;

struct port_def_t {
  int kind;
  int port;
  int fd;
  // used by F_NETWORK_STATS
  int in_packets;
  int in_volume;
  int out_packets;
  int out_volume;
};

extern struct port_def_t external_port[5];

#ifdef __cplusplus
};
#endif

#endif /* BASE_INTERNAL_EXTERNAL_PORT_H_ */
