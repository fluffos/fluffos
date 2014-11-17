// Various mud extension protocol using telnet.

#ifndef NET_TELNET_EXT_H_
#define NET_TELNET_EXT_H_

struct interactive_t;

// Our own extensions
#define TELNET_TELOPT_MXP 91  // mud extension protocol
#define TELNET_TELOPT_GMCP 201

void on_telnet_do_zmp(const char**, unsigned long, interactive_t*);
void on_telnet_do_gmcp(interactive_t* ip);

#endif /* NET_TELNET_EXT_H_ */
