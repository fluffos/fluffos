#ifndef TELNET_H_
#define TELNET_H_

// Because libtelnet already uses telnet_init.
struct telnet_t *net_telnet_init(struct interactive_t *user);

// Send initial negotiations.
void send_initial_telnet_negotiations(struct interactive_t *user);

// Change terminal mode
void set_linemode(struct interactive_t *ip, bool flush = true);
void set_charmode(struct interactive_t *ip, bool flush = true);
void set_localecho(struct interactive_t *ip, bool enable, bool flush = true);

void telnet_do_naws(struct telnet_t *);
void telnet_dont_naws(struct telnet_t *);
void telnet_start_request_ttype(struct telnet_t *);
void telnet_request_ttype(struct telnet_t *);
void telnet_send_nop(struct telnet_t *);
void telnet_send_ga(struct telnet_t *);

// Various mud extension protocol using telnet.
#define TELNET_TELOPT_MXP 91  // mud extension protocol
#define TELNET_TELOPT_GMCP 201

bool on_receive_mxp_tag(struct interactive_t *, const char *);
void on_telnet_will_mxp(struct interactive_t *);
void on_telnet_do_zmp(const char **, unsigned long, interactive_t *);
void on_telnet_do_gmcp(struct interactive_t *);

#endif /* TELNET_H_ */
