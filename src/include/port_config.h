/// @file port_config.h
/// @brief include file for predefkne __PORT_CONFIG__
///
/// macros from ```base/internal/external_port.h``` for possible port types
/// @author René Müller
/// @version 0.0.0
/// @date 2016-06-01

#ifndef __PORT_CONFIG_H
#define __PORT_CONFIG_H

#define PORT_UNDEFINED 0
#define PORT_TELNET 1
#define PORT_BINARY 2
#define PORT_ASCII 3
#define PORT_MUD 4
#define PORT_WEBSOCKET 5

#endif // __PORT_CONFIG_H
