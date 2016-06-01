/*
 * port_config.h
 *
 * Copy this file to your mudlib include dir for use with the
 * get_config() efun, option __PORT_CONFIG__.
 */

#ifndef __PORT_CONFIG_H
#define __PORT_CONFIG_H

#define PORT_UNDEFINED  0
#define PORT_TELNET     1
#define PORT_BINARY     2
#define PORT_ASCII      3
#define PORT_MUD        4
#define PORT_WEBSOCKET  5

#endif // __PORT_CONFIG_H
