/*
 * external_port.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef BASE_INTERNAL_EXTERNAL_PORT_H_
#define BASE_INTERNAL_EXTERNAL_PORT_H_

#include "include/port_config.h"

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
